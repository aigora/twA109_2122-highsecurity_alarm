#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(2, 3); // Pines a los que va el sensor de huellas



const int EchoPin = 8; // Pines a los que va el sensor de movimiento
const int TriggerPin = 9;

void setup() {

  Serial.begin(9600);
  pinMode(TriggerPin,OUTPUT); 
  pinMode(EchoPin,INPUT);
}

void loop() {
  int cm = ping(TriggerPin,EchoPin);
 
 
 if( Serial.available()> 0) 
 {
  mensaje_entrada = Serial.readStringUntil('\n');
    if (mensaje_entrada.compareTo("GET_DISTANCIA")==0)
    {
      Serial.println(cm);
    }
  delay(900);


 if (mensaje_entrada.compareTo("HUELLA_EXISTENTE")==0)
 {
  finger.begin(57600);
  delay(5);
     if (finger.verifyPassword())
      {
   Serial.println("Sensor de huellas listo!");
      }
    else 
     {
    Serial.println("Algun problema con el sensor de huellas");
    while (1) { delay(1); }
     }
  Serial.println("Situe su dedo sobre el sensor...");
 }
 }
}


int ping(int TriggerPin, int EchoPin)
{
  long duracion, distanciaCm;

  digitalWrite(TriggerPin,LOW);
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin,LOW);

  duracion = pulseIn(EchoPin, HIGH);
  distanciaCm = duracion * 10/292/2;
  return distanciaCm;
}
