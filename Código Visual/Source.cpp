#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <conio.h>
#include "SerialClass/SerialClass.h"

#define MAX_BUFFER 200
#define PAUSA_MS 200
#define TAM 20
#define MAX_REGIS 20

typedef struct
{
	char nombre[TAM];
	int id;
}identificacion;

// Funciones prototipo
int leer_fichero(identificacion[]);
void guardar_fichero(identificacion*, int);
char menu1(void);
int menu2(void);
int menu3(void);

float leer_sensor_distancia(Serial*);
int Comprobar_huella(Serial*, identificacion[]);
int leer_huella(Serial*);
void AbrirPuerta(Serial*);
void CerrarPuerta(Serial*);
int Comprobar_contrasena(void);
void ActivarAlarma(Serial* Arduino);
int registrar_huella(Serial*, identificacion*, int);

int Enviar_y_Recibir(Serial*, const char*, char*);
int Recibir(Serial*, char*);
float float_from_cadena(char* cadena);

int main(void)
{
	Serial* Arduino;
	char puerto[] = "COM4"; // Puerto serie al que está conectado Arduino

	int opcion_menu2 = 0;
	char opcion_letra;
	int numero;
	int opc = 0;

	identificacion registros[MAX_REGIS];
	int i, numregistros = 0;
	float distancia;

	setlocale(LC_ALL, "es-ES");
	Arduino = new Serial((char*)puerto);
	numregistros = leer_fichero(registros);

	do
	{
		distancia = leer_sensor_distancia(Arduino);
		if (distancia == -1)
			printf("Atención: No se ha podido leer el sensor de distancia\n");
		else
			if (distancia <= 10) // Se detecta una persona
			{
				opcion_letra = menu1(); // 
				if (opcion_letra == 'S' || opcion_letra == 's')
				{
					
					
				    opcion_menu2 = menu2();
					
					switch (opcion_menu2)
					{
					case 1:
						numero = Comprobar_huella(Arduino, registros);
						if (numero < 0)
						{
							printf("Usted no está registrado.¿Quiere registrarse?\n");
							opc = menu3();
							if (opc == 1)
							{

								if (Comprobar_contrasena() == 0)
								{
									printf("Acceso denegado\n");
									ActivarAlarma(Arduino);
								}
								else
								{
									printf("Contraseña correcta\n");
									numregistros = registrar_huella(Arduino, registros, numregistros);
									guardar_fichero(registros, numregistros);
									AbrirPuerta(Arduino);
									CerrarPuerta(Arduino);
								}
							}
						}

						else
						{
							AbrirPuerta(Arduino);
							CerrarPuerta(Arduino);
						}
						break;
					case 2:
					{
						
						if (Comprobar_contrasena() == 0)
						{
							printf("Acceso denegado\n");
							ActivarAlarma(Arduino);
						}
						else
						{
							printf("Contraseña correcta\n");
							numregistros = registrar_huella(Arduino, registros, numregistros);
							guardar_fichero(registros, numregistros);
							AbrirPuerta(Arduino);
							CerrarPuerta(Arduino);
						}
						
					}
					break;
					case 3:
						printf("Los datos registrados actualmente son:\n");
						for (i = 0; i < numregistros; i++)
							printf("%s - %d\n", registros[i].nombre, registros[i].id);
						break;
					case 4:
						break;
					default: printf("\n Opción incorrecta\n");
					}
				}
			}
	} while (opcion_menu2 != 4);
	return 0;
}

int leer_fichero(identificacion registros[])
{
	FILE* fichero;
	errno_t e;
	int numregistros = 0;

	e = fopen_s(&fichero, "Personas_registradas.txt", "rt");
	if (fichero == NULL)
	{
		printf("El archivo de registros no existe\n");
	}
	else
	{
		fscanf_s(fichero, "%s", registros[numregistros].nombre, TAM);
		while (!feof(fichero))
		{
			fscanf_s(fichero, "%d\n", &registros[numregistros].id);
			numregistros++;
			fscanf_s(fichero, "%s", registros[numregistros].nombre, TAM);
		}
		fclose(fichero);
	}
	return numregistros;
}

void guardar_fichero(identificacion* i, int n)
{
	FILE* fichero;
	errno_t e;
	int j;

	e = fopen_s(&fichero, "Personas_registradas.txt", "wt");
	if (fichero == NULL)
	{
		printf("Hay algún error a la hora de grabar el archivo\n");
	}
	else
	{
		for (j = 0;j < n;j++)
			fprintf(fichero, "%s %d\n", i[j].nombre, i[j].id);
		fclose(fichero);
	}
}

char menu1(void)
{
	char letra;
	char intro;
	printf("==============\n");
	printf("Si desea salir pulse S\n");
	printf("==============\n");
	scanf_s("%c", &letra, 1);
	scanf_s("%c", &intro, 1);
	return letra;
}

int menu2(void)
{
	int opcion;
	char intro;
	printf("\n");
	printf("==============\n");
	printf("1 - Guardia antiguo\n");
	printf("2 - Registrarse (Nuevo guardia)\n");
	printf("3 - Consultar lista de guardias registrados\n");
	printf("4 - Salir de la aplicación\n");
	printf("==============\n");
	scanf_s("%d", &opcion);
	scanf_s("%c", &intro, 1);
	return opcion;
}


int menu3(void)
{
	int opc;
	printf("\n");
	printf("==============\n");
	printf("1 - Sí\n");
	printf("2 - No\n");
	printf("==============\n");
	scanf_s("%d", &opc);
	return opc;
}

float leer_sensor_distancia(Serial* Arduino)
{
	float distancia;
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "GET_DISTANCIA\n", mensaje_recibido);

	if (bytesRecibidos <= 0)
		distancia = -1;
	else
		distancia = float_from_cadena(mensaje_recibido);
	return distancia;
}
int Comprobar_huella(Serial* Arduino, identificacion* i)
{
	int numero;
	numero = leer_huella(Arduino);
	if (numero >= 0)
	{
		printf("Usted se ha identificado como %s. Acceso otorgado\n", i[numero].nombre);
	}
	return numero;
}

int leer_huella(Serial* Arduino)
{
	int id;
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "HUELLA_EXISTENTE\n", mensaje_recibido);
	do
	{
		if (bytesRecibidos > 0)
			printf("%s\n", mensaje_recibido);
		bytesRecibidos = Recibir(Arduino, mensaje_recibido);
	} while (strstr(mensaje_recibido, "Match") == NULL);

	if (strstr(mensaje_recibido, "No Match") != NULL)
		id = -1;
	else
		id = (int)float_from_cadena(mensaje_recibido);
	return id;
}

void AbrirPuerta(Serial* Arduino)
{
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "ABRE_LA_PUERTA\n", mensaje_recibido);
	if (bytesRecibidos <= 0)
		printf("\nNo se ha recibido confirmación\n");
	else
		printf("\n%s\n", mensaje_recibido);
}

void CerrarPuerta(Serial* Arduino)
{
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "CIERRA_LA_PUERTA\n", mensaje_recibido);

	if (bytesRecibidos <= 0)
		printf("\nNo se ha recibido confirmación\n");
	else
		printf("\n%s\n", mensaje_recibido);
}

int Comprobar_contrasena(void)
{
	long int contrasena;
	long int clave;
	char intro;
	int n = 0;
	int intento = 3;
	clave = 906151612;
	printf("Introduzca la contraseña: ");
	scanf_s("%d", &contrasena);
	scanf_s("%c", &intro, 1);
	while (intento > 1)
	{
		if (contrasena == clave)
		{
			n = 1;
			intento = 0;
		}
		else
		{
			intento--;
			printf("\nContraseña incorrecta. Le quedan %d intentos\n", intento);
			printf("Introduzca la contraseña: ");
			scanf_s("%d", &contrasena);
			scanf_s("%c", &intro, 1);
		}
	}
	return n;
}

void ActivarAlarma(Serial* Arduino)
{
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "ALARMA\n", mensaje_recibido);
}

int registrar_huella(Serial* Arduino, identificacion* i, int id)
{
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];
	char mensaje_enviado[MAX_BUFFER];
	char intro;

	bytesRecibidos = Enviar_y_Recibir(Arduino, "REGISTRAR\n", mensaje_recibido);
	printf("%s\n", mensaje_recibido);
	i[id].id = id;
	printf("Introduzca su nombre: ");
	scanf_s("%s", i[id].nombre, TAM);
	scanf_s("%c", &intro, 1);
	sprintf_s(mensaje_enviado, "%d\n", id);
	bytesRecibidos = Enviar_y_Recibir(Arduino, mensaje_enviado, mensaje_recibido);
	do
	{
		if (bytesRecibidos > 0)
			printf("%s\n", mensaje_recibido);
		bytesRecibidos = Recibir(Arduino, mensaje_recibido);
	} while (strstr(mensaje_recibido, "Match") == NULL);
	printf("%s\n", mensaje_recibido);
	if (strstr(mensaje_recibido, "No Match") == NULL)
		id++;
	return id;
}

int Enviar_y_Recibir(Serial* Arduino, const char* mensaje_enviar, char* mensaje_recibir)
{
	int bytes_recibidos = 0, total = 0;
	int intentos = 0, fin_linea = 0;

	Arduino->WriteData((char*)mensaje_enviar, (unsigned int)strlen(mensaje_enviar));
	Sleep(PAUSA_MS);

	bytes_recibidos = Arduino->ReadData(mensaje_recibir, sizeof(char) * MAX_BUFFER - 1);

	while ((bytes_recibidos > 0 || intentos < 5) && fin_linea == 0)
	{
		if (bytes_recibidos > 0)
		{
			total += bytes_recibidos;
			if (mensaje_recibir[total - 1] == 13 || mensaje_recibir[total - 1] == 10)
				fin_linea = 1;
		}
		else
			intentos++;
		Sleep(PAUSA_MS);
		bytes_recibidos = Arduino->ReadData(mensaje_recibir + total, sizeof(char) * MAX_BUFFER - 1);
	}
	if (total > 0)
		mensaje_recibir[total - 1] = '\0';
	return total;
}

int Recibir(Serial* Arduino, char* mensaje_recibir)
{
	int bytes_recibidos = 0, total = 0;
	int intentos = 0, fin_linea = 0;

	bytes_recibidos = Arduino->ReadData(mensaje_recibir, sizeof(char) * MAX_BUFFER - 1);

	while ((bytes_recibidos > 0 || intentos < 2) && fin_linea == 0)
	{
		if (bytes_recibidos > 0)
		{
			total += bytes_recibidos;
			if (mensaje_recibir[total - 1] == 13 || mensaje_recibir[total - 1] == 10)
				fin_linea = 1;
		}
		else
			intentos++;
		Sleep(PAUSA_MS);
		bytes_recibidos = Arduino->ReadData(mensaje_recibir + total, sizeof(char) * MAX_BUFFER - 1);
	}
	if (total > 0)
		mensaje_recibir[total - 1] = '\0';
	return total;
}

float float_from_cadena(char* cadena)
{
	float numero = 0;
	int i, divisor = 10, estado = 0;

	for (i = 0; cadena[i] != '\0' && estado != 3 && i < MAX_BUFFER; i++)
		switch (estado)
		{
		case 0:// Antes del número
			if (cadena[i] >= '0' && cadena[i] <= '9')
			{
				numero = (float)cadena[i] - '0';
				estado = 1;
			}
			break;
		case 1:// Durante el número
			if (cadena[i] >= '0' && cadena[i] <= '9')
				numero = numero * 10 + cadena[i] - '0';
			else
				if (cadena[i] == '.' || cadena[i] == ',')
					estado = 2;
				else
					estado = 3;
			break;
		case 2: // Parte decimal
			if (cadena[i] >= '0' && cadena[i] <= '9')
			{
				numero = numero + (float)(cadena[i] - '0') / divisor;
				divisor *= 10;
			}
			else
				estado = 3;
			break;
		}
	return numero;
}