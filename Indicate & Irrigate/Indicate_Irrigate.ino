#include<dht.h>
dht DHT;
#define DHT11_PIN A0
const int soilpin=A1;
int ledsoil=12;
int ledtemp= 7;
void setup()
{
  Serial.begin(9600);
  for (int DigitalPin = 7; DigitalPin <= 9; DigitalPin++) 
 {
  pinMode(DigitalPin, OUTPUT);
  Serial.begin(9600);
  pinMode(ledsoil,OUTPUT);
 }
}
void loop()
{
    // READ DATA 
int chk = DHT.read11(DHT11_PIN); 
Serial.println(" Humidity " ); 
Serial.println(DHT.humidity, 1); 
Serial.println(" Temparature "); 
Serial.println(DHT.temperature, 1);
delay(2000); 
  if (DHT.temperature<=20)
  {
  digitalWrite(7, HIGH);
  }
  else
  {
  digitalWrite(7, LOW);
  }
 int output_value ;
     output_value= analogRead(soilpin);

   Serial.print("Mositure : ");

   Serial.print(output_value);

   Serial.println("%");

   delay(1000);

  int moisture=analogRead(soilpin);
  Serial.println(moisture);
  if(output_value>890)
  {
    digitalWrite(ledsoil,HIGH);
  }
else
{
  digitalWrite(ledsoil,LOW);
}
  
}
