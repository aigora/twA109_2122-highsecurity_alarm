#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <conio.h>
#include "SerialClass/SerialClass.h"

#define MAX_BUFFER 200
#define PAUSA_MS 200

// Funciones prototipo
int menu(void);
void verifica_sensores(Serial*,char*);
void monitorizar_sensor_distancia(Serial*);
void activar_alarma_distancia(Serial* Arduino);
void comprobar_mensajes(Serial*);
float leer_sensor_distancia(Serial*);
int Enviar_y_Recibir(Serial* ,const char* , char* ); 
float float_from_cadena(char* cadena);

int main(void)
{
	Serial* Arduino;
	char puerto[] = "COM4"; // Puerto serie al que est� conectado Arduino
	int opcion_menu;

	setlocale(LC_ALL, "es-ES");
	Arduino = new Serial((char*)puerto);
	do
	{
		comprobar_mensajes(Arduino);
		opcion_menu = menu();
		switch (opcion_menu)
		{
		case 0: break;
		case 1: 
			verifica_sensores(Arduino,puerto);
			break;
		case 2:
			monitorizar_sensor_distancia(Arduino);
			break;
		case 3:
			activar_alarma_distancia(Arduino);
			break;
		case 4:
			break;
		default: printf("\nOpci�n incorrecta\n");
		}
	} while (opcion_menu != 4);
	return 0;
}

// *********************************************************************************
// La funci�n men� se limita a mostrar en pantalla la oferta de opciones disponibles
// *********************************************************************************
int menu(void)
{
	static int opcion=-1;
	
	if (opcion != 0)
	{
		printf("\n");
		printf("Men� Principal\n");
		printf("==============\n");
		printf("1 - Verificar sensores.\n");
		printf("2 - Monitorizar sensores.\n");
		printf("3 - Activar/Desactivar alarma por distancia\n");
		printf("4 - Salir de la aplicaci�n\n");
		printf("Opci�n:");
	}
	if (_kbhit())
	{
		opcion = (int)_getch()-'0';
		printf("%d\n", opcion);
	}
	else
		opcion = 0;
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

void activar_alarma_distancia(Serial* Arduino)
{
	int bytesRecibidos;
	char mensaje_recibido[MAX_BUFFER];

	bytesRecibidos = Enviar_y_Recibir(Arduino, "SET_MODO_ALARMA\n", mensaje_recibido);
	if (bytesRecibidos <= 0)
		printf("\nNo se ha recibido confirmaci�n\n");
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

	printf("Pulse una tecla para finalizar la monitorizaci�n\n");
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
		printf("Revise la conexi�n, el puerto %s y desactive el monitor serie del IDE de Arduino.\n",port);
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
		case 0:// Antes del n�mero
			if (cadena[i] >= '0' && cadena[i] <= '9')
			{
				numero = cadena[i] - '0';
				estado = 1;
			}
			break;
		case 1:// Durante el n�mero
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