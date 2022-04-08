#include <SoftwareSerial.h>

void enviar_mensajeBT(String );

const int RxBTPin=3; // Rx del módulo HC‐05/HC‐06 ‐> Tx Arduino
const int TxBTPin=4; // Tx del módulo HC‐05/HC‐06 ‐> Rx Arduino

String mensaje_entrada;
String mensaje_salida;
SoftwareSerial SerialBT(TxBTPin, RxBTPin); // Rx,Tx Arduino
void setup()  
{
   Serial.begin(9600);
   SerialBT.begin(9600);
}
void loop()  
{
  if (Serial.available()) // Si se recibe un mensaje desde Serial Monitor
  {
     mensaje_salida = Serial.readStringUntil('\n');
     SerialBT.println(mensaje_salida); // Se lo enviamos al PC
  }
  if (SerialBT.available()) // Si se recibe un mensaje desde Bluetooth
  {
     mensaje_entrada = SerialBT.readStringUntil('\n');
     Serial.println("LOG: Recibido‐> "+mensaje_entrada); // Hacemos eco por SM
     mensaje_salida=String("MI RESPUESTA"); // Enviamos el mensaje por Bluetooth
     enviar_mensajeBT(mensaje_salida);
     Serial.println("LOG: Enviado‐> "+mensaje_salida);     
  }
}


// Función que envía carácter a carácter un mensaje con breve pausa
void enviar_mensajeBT(String mensaje)
{
int i;
  for (int i=0;i<mensaje.length();i++)
     {
       SerialBT.write(mensaje.charAt(i));
       delay(50);
     }  
}
