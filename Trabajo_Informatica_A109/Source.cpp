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
int menu2(void);
char menu1(void);
int menu3(void);
void comprobar_mensajes(Serial*);
float leer_sensor_distancia(Serial*);
int leer_huella(Serial*);
int Enviar_y_Recibir(Serial* ,const char* , char* ); 
float float_from_cadena(char* cadena);
void BloquearPuerta(Serial* Arduino);
int registrar_huella(Serial* Arduino, identificacion* i);
int Comprobar_huella(Serial* Arduino, identificacion i[MAX_REGIS]);
void AbrirPuerta(Serial* Arduino);
void guardar_datos(identificacion* i, int n);
int leer_datos(void);



int main(void)
{
	Serial* Arduino;
	
	char puerto[] = "COM4"; // Puerto serie al que está conectado Arduino
	int opcion_menu2,opcion_menu3;
	int id,total;
	char opcion_letra;
	int numero;
	
	identificacion registros[MAX_REGIS];
	setlocale(LC_ALL, "es-ES");
	Arduino = new Serial((char*)puerto);

	FILE* fichero;
	errno_t e;
	e = fopen_s(&fichero, "Personas registradas.txt", "wt");
	if (fichero == NULL)
	{
		printf("No se ha podido abrir el archivo\n");
	}
	else
	{
		fprintf(fichero,"\n\n                                    Lista de registrados\n");
		fprintf(fichero, "\n-Los nombres se encuentran indicados junto a su número identificativo correspondiente\n\n");
	}
	fclose(fichero);
	

	if (leer_sensor_distancia(Arduino) <= 10)
	{
		do
		{
			opcion_letra = menu1();
			if (opcion_letra == 'S')
			{
				opcion_menu2 = menu2();
				switch (opcion_menu2)
				{
				case 1:
				{
					int numero;
					numero = Comprobar_huella(Arduino, registros);
					if (numero == 0)
					{
						printf("Usted no está registrado.¿Quiere registrarse?\n");
						opcion_menu3 = menu3();
						if (opcion_menu3 == 1)
						{
							opcion_menu2 = 2;
						}
					}
					else
						AbrirPuerta(Arduino);
				}
					break;
				case 2:
				{
					int n;
					n = Comprobar_contraseña();
					if (n == 0)
					{
						printf("Acceso denegado\n");
						BloquearPuerta(Arduino);

					}
					else
					{
						printf("Contraseña correcta\n");
						id = registrar_huella(Arduino, registros);
						guardar_datos(registros, id);
					}
					
				}
					
					break;
				case 3:
				{
					total = leer_datos(void);
					printf("Hay %d contactos registrados en total\n", total);

				}
					
					break;
				case 4:
					break;
				default: printf("\n¿? Elija una opción posible\n");
				}
			}
		} while (opcion_menu2 != 4);
	}
	return 0;
}

void guardar_datos(identificacion* i,int n)
{
	FILE* fichero;
	errno_t e;
	e = fopen_s(&fichero, "Personas registradas.txt", "at");
	if (fichero == NULL)
	{
		printf("Hay algún error con el archivo\n");
	}
	else
	{	fprintf(fichero, "%d %s\n",i[n].id, i[n].nombre);
	}
	fclose(fichero);
}

int leer_datos(void)
{
	FILE* fichero;
	errno_t e;
	identificacion a;
	int n, total = 0;
	char* p;
	e = fopen_s(&fichero, "Personas registradas.txt", "rt");
	if (fichero == NULL)
	{
		printf("Hay algún error con el archivo\n");
	}
	else
	{
	fscanf_s(fichero, "%d", &a.id, sizeof(int));
	fscanf_s(fichero, "%s", &a.nombre, TAM);
		while (!feof(fichero))
		{
			p = strchr(a.nombre, '\n');
			*p = '\0';
			printf("ID: %d Nombre: %s\n", a.id, a.nombre);
			total++;
			fscanf_s(fichero, "%d", &a.id, 1);
			fscanf_s(fichero, "%s", &a.nombre, TAM);
		}
	}
	fclose(fichero);
	return total;
}


int menu2(void)
{
	
	
	int opcion;
	printf("\n");
	printf("==============\n");
	printf("1 - Guardia antiguo\n");
	printf("2 - Registrarse (Nuevo guardia)\n");
	printf("3 - Consultar lista de guardias registrados\n");
	printf("4 - Salir de la aplicación\n");
	printf("==============\n");
	scanf_s("%d", &opcion);
	return opcion;
}

char menu1(void)
{
	char letra;
	printf("==============\n");
	printf("Si desea salir pulse S\n");
	printf("==============\n");
	scanf_s("%c", &letra);
	return letra;
		
}

int menu3(void)
{
	int opcion;
	printf("==============\n");
	printf("1 - Sí\n");
	printf("2 - No\n");
	printf("==============\n");
	scanf_s("%d", &opcion);
	return opcion;
}

void comprobar_mensajes(Serial* Arduino)
{
	int bytesRecibidos, total = 0;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Arduino->ReadData(mensaje_recibido, sizeof(char) * MAX_BUFFER - 1);
	while (bytesRecibidos > 0)
	{
		Sleep(PAUSA_MS);
		total += bytesRecibidos;
		bytesRecibidos = Arduino->ReadData(mensaje_recibido + total, sizeof(char) * MAX_BUFFER - 1);
	}
	if (total > 0)
	{
		mensaje_recibido[total-1] = '\0';
		printf("\nMensaje recibido: %s\n", mensaje_recibido);
	}
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
int leer_huella(Serial* Arduino)
{
	int id;
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "HUELLA_EXISTENTE\n", mensaje_recibido);
    id = float_from_cadena(mensaje_recibido);
	return id;

}


int registrar_huella(Serial* Arduino, identificacion* i)
{
	int id;
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "REGISTRAR\n", mensaje_recibido);
	id = float_from_cadena(mensaje_recibido);
	i[id].id = id;
	printf("Introduzca su nombre: ");
	scanf_s("%s", &i[id].nombre);
	return id;
}

int Comprobar_contraseña(void)
{
	long int contraseña;
	long int clave;
	int n=0,i;
	int intento = 2;
	clave = 906151612;
	printf("Introduzca la contraseña: ");
	scanf_s("%d", &contraseña);
	while (intento != 0)
	{
		if (contraseña == clave)
		{
			n = 1;
			intento = 0;
		}
		else
		{
			intento--;
			printf("\nContraseña incorrecta. Le quedan %d intentos\n", intento);
			printf("Introduzca la contraseña: ");
			scanf_s("%d", &contraseña);
			
		}
	}
	
	return n;
}

int Comprobar_huella(Serial* Arduino, identificacion* i)
{
	int numero;
	numero = leer_huella(Arduino);
	if (numero != 0)
	{
		printf("Usted se ha identificado como %s. Acceso otorgado\n", i[numero].nombre);
	}
	return numero;
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

void BloquearPuerta(Serial* Arduino)
{
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "CIERRA_LA_PUERTA\n", mensaje_recibido);
	if (bytesRecibidos <= 0)
		printf("\nNo se ha recibido confirmación\n");
	else
		printf("\n%s\n", mensaje_recibido);
}

int Enviar_y_Recibir(Serial* Arduino, const char* mensaje_enviar, char* mensaje_recibir)
{
	int bytes_recibidos=0,total=0;
	int intentos = 0,fin_linea=0;


	Arduino->WriteData((char *)mensaje_enviar,strlen(mensaje_enviar));
	Sleep(PAUSA_MS);

	bytes_recibidos = Arduino->ReadData(mensaje_recibir,sizeof(char)*MAX_BUFFER-1);

	while ((bytes_recibidos > 0 || intentos < 5) && fin_linea==0)
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
		bytes_recibidos = Arduino->ReadData(mensaje_recibir+total, sizeof(char) * MAX_BUFFER - 1);
	}
	if (total > 0)
		mensaje_recibir[total - 1] = '\0';

	//printf("LOG: %d bytes -> %s\nIntentos=%d - EOLN=%d\n", total, mensaje_recibir,intentos,fin_linea);
	return total;
}





float float_from_cadena(char* cadena)
{
	float numero = 0;
	int i, divisor = 10, estado = 0;


	for (i=0;cadena[i]!='\0' && estado!=3 && i<MAX_BUFFER;i++)
		switch (estado)
		{
		case 0:// Antes del número
			if (cadena[i] >= '0' && cadena[i] <= '9')
			{
				numero = cadena[i] - '0';
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


/*
void activar_alarma_distancia(Serial* Arduino)
{
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "SET_MODO_ALARMA\n", mensaje_recibido);
	if (bytesRecibidos <= 0)
		printf("\nNo se ha recibido confirmación\n");
	else
		printf("\n%s\n", mensaje_recibido);
}

void monitorizar_sensor_distancia(Serial* Arduino)
{
	float frecuencia, distancia;
	char tecla;
	do
	{
		printf("Establezca frecuencia de muestreo (0,5 Hz - 2,0 Hz):");
		scanf_s("%f", &frecuencia);
	} while (frecuencia < 0.5 || frecuencia>2.0);

	printf("Pulse una tecla para finalizar la monitorización\n");
	do
	{
		if (Arduino->IsConnected())
		{
			distancia = leer_sensor_distancia(Arduino);
			if (distancia != -1)
				printf("%.2f ", distancia);
			else
				printf("XXX ");
		}
		else
			printf("\nNo se ha podido conectar con Arduino.\n");
		if ((1 / frecuencia) * 1000 > PAUSA_MS)
		  Sleep((1 / frecuencia) * 1000 - PAUSA_MS);
	} while (_kbhit() == 0);
	tecla = _getch();
	return;
}

void verifica_sensores(Serial* Arduino,char* port)
{
	float distancia;

	if (Arduino->IsConnected())
	{
		distancia = leer_sensor_distancia(Arduino);
		if (distancia!=-1)
		  printf("\nDistancia: %f\n", distancia);
	}
	else
	{
		printf("\nNo se ha podido conectar con Arduino.\n");
		printf("Revise la conexión, el puerto %s y desactive el monitor serie del IDE de Arduino.\n",port);
	}
}
void verifica_sensores(Serial*,char*);
void monitorizar_sensor_distancia(Serial*);
void activar_alarma_distancia(Serial* Arduino);


*/