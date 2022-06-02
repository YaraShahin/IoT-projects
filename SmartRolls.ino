#include <dht.h>
#include <ezButton.h>
#include <SoftwareSerial.h>       //Software Serial library

//define input sensors pins (dht11, limit switch, keypad, line follower channels)
#define temp A0
#define lsw A5
#define left_line A2
#define center_line A4
#define right_line A3
#define leave 11
#define toilet 4
#define living 9
#define bed 10

//define output actions pins
#define buzz A1
#define m1a 12
#define m1b 13
#define m1 5
#define m2a 7
#define m2b 8
#define m2 6

#define DEBUG true

dht DHT;
ezButton limitSwitch(lsw);
SoftwareSerial espSerial(2, 3);   //Pin 2 and 3 act as RX and TX. Connect them to TX and RX of ESP8266

String mySSID = "";       // WiFi SSID
String myPWD = ""; // WiFi Password
String myAPI = "8I60C6KP5PPADTL2";   // API Key
String myHOST = "api.thingspeak.com";
String myPORT = "80";

//dht11 temprature reading and status (0: normal, 1: abnormal)
float temp_val = -1;
bool temp_stat = 0;

//should the user want to be on chair?(0: decided to leave, 1: should be on)
bool sit = 1;
//Is user on chair (0: yes, 1: no)
int limit_stat = 0;

//the location of the robot (0: home, 1: toilet, 2: living, 3: bed)
int location = 0;
//the orientation of the robot at home position (0: west, 1: middle, 2: east)
int pos = 0;

//is_line returns true if there is a line
bool is_line(float val) {
  if (val > 400) return true;
  return false;
}

void forward() {
  Serial.println("forward");
  digitalWrite(m1a, HIGH);
  digitalWrite(m1b, LOW);
  digitalWrite(m2a, HIGH);
  digitalWrite(m2b, LOW);
}

void reverse() {
  Serial.println("reverse");
  digitalWrite(m1a, LOW);
  digitalWrite(m1b, HIGH);
  digitalWrite(m2a, LOW);
  digitalWrite(m2b, HIGH);
}

void s() {
  Serial.println("stop");
  digitalWrite(m1a, LOW);
  digitalWrite(m1b, LOW);
  digitalWrite(m2a, LOW);
  digitalWrite(m2b, LOW);
}

void send_data(int sendVal, String FIELD) {
  String sendData = "GET /update?api_key=" + myAPI + "&" + FIELD + "=" + String(sendVal);
  espData("AT+CIPMUX=1", 1000, DEBUG);       //Allow multiple connections
  espData("AT+CIPSTART=0,\"TCP\",\"" + myHOST + "\"," + myPORT, 1000, DEBUG);
  espData("AT+CIPSEND=0," + String(sendData.length() + 4), 1000, DEBUG);
  espSerial.find(">");
  espSerial.println(sendData);
  Serial.print("Value to be sent: ");
  Serial.println(sendVal);

  espData("AT+CIPCLOSE=0", 1000, DEBUG);
  delay(3000);
}

String espData(String command, const int timeout, boolean debug) {
  Serial.print("AT Command ==> ");
  Serial.print(command);
  Serial.println("     ");

  String response = "";
  espSerial.println(command);
  long int time = millis();
  while ( (time + timeout) > millis()) {
    while (espSerial.available()) {
      char c = espSerial.read();
      response += c;
    }
  }
  return response;
}

void go_living(){
  if (location==0) while (not is_line(analogRead(left_line)) and not is_line(analogRead(right_line)) and is_line(analogRead(center_line))) forward();
  if (location == 4) go_toilet();
  if (location ==2) while (not is_line(analogRead(left_line)) and is_line(analogRead(center_line))) reverse();
  if (location == 1) Serial.print("I'm already there");
  s();
  delay(500);
  location = 1;
  Serial.println("Went to living");
}

void go_toilet(){
  if (location==0)go_living();
  if (location == 1) while (not is_line(analogRead(left_line)) and is_line(analogRead(center_line))) forward();
  if (location == 2) Serial.println("Already toilet 2 duhh");
  if (location == 4) while (not is_line(analogRead(left_line)) and is_line(analogRead(center_line))) reverse();
  s();
  delay(500);
  location = 2;
  Serial.println("Went to toilet");
}



void go_bedroom(){
  if (location==0){go_living(); go_toilet();}
  if (location == 4) Serial.println("Already bedroom 4 duhh");
  if (location == 2) while (not is_line(analogRead(right_line)) and is_line(analogRead(center_line))) forward();
  if (location == 1) go_toilet();
  s();
  delay(500);
  location = 4;
  Serial.println("Went to bedroom");
}

void setup() {
  Serial.begin(9600);
  espSerial.begin(115200);
  delay(200);

  espData("AT+RST", 1000, DEBUG);                      //Reset the ESP8266 module
  espData("AT+CWMODE=1", 1000, DEBUG);                 //Set the ESP mode as station mode
  espData("AT+CWJAP=\"" + mySSID + "\",\"" + myPWD + "\"", 1000, DEBUG); //Connect to WiFi network

  //pinMode(temp, INPUT);
  pinMode(left_line, INPUT);
  pinMode(center_line, INPUT);
  pinMode(right_line, INPUT);

  pinMode(lsw, INPUT);

  //pinMode(leave, INPUT_PULLUP);
  pinMode(toilet, INPUT_PULLUP);
  pinMode(living, INPUT_PULLUP);
  pinMode(bed, INPUT_PULLUP);

  pinMode(m1a, OUTPUT);
  pinMode(m1b, OUTPUT);
  pinMode(m1, OUTPUT);
  pinMode(m2a, OUTPUT);
  pinMode(m2b, OUTPUT);
  pinMode(m2, OUTPUT);

  pinMode(buzz, OUTPUT);
  analogWrite(buzz, 0);

  limitSwitch.setDebounceTime(50); // set debounce time to 50 milliseconds

  analogWrite(m1, 100);
  analogWrite(m2, 100);

  delay(200);
  Serial.println("Done Setup");
}

void loop() {
  limitSwitch.loop();

  //reading keypad values (debounce needed)
  int leaveS = digitalRead(leave);
  int toiletS = digitalRead(toilet);   //key 2
  int livingS = digitalRead(living);   //key 1
  int bedS = digitalRead(bed);         //key 4

  //reading temprature value and sending to cloud
  DHT.read11(temp);
  temp_val = DHT.temperature;
  if (temp_val > 30 or temp_val < 20) temp_stat = 1;
  else temp_stat = 0;
  Serial.print("temperature = ");
  Serial.print(temp_val);
  Serial.print("C, ");
  Serial.println(temp_stat);
  send_data(temp_val, "field1");

  //reading limitswitch status and sending to cloud
  limit_stat = limitSwitch.getState();
  Serial.print(">>>>>>>>>");
  Serial.println(limit_stat);
  if (limit_stat) Serial.println("The limit switch: UNTOUCHED (OFF chair)");
  else Serial.println("The limit switch: TOUCHED (ON chair)");
  send_data(limit_stat, "field2");

  //setting the buzzer high or low according to normal or not temp_stat or 
  if ((limit_stat and sit)) {
    analogWrite(buzz, 255);
    Serial.println("Notificarion! Something is wrong with the user.");
    send_data(1, "field4");
  }
  else {
    analogWrite(buzz, 0);
    Serial.println("Everything is normal");
    send_data(0, "field4");
  }


  //Reading the keypad and acting accordingly
  if (!leaveS) {
    Serial.println("leave key is pressed");
    sit = false;
  }
  if (!toiletS) {
    Serial.println("Toilet key is pressed");
    go_toilet();
    send_data(1, "field3");
  }
  if (!livingS) {
    Serial.println("Living key is pressed");
    go_living();
    send_data(2, "field3");
  }
  if (!bedS) {
    Serial.println("Bedroom key is pressed");
    go_bedroom();
    send_data(3, "field3");
  }

  delay(500);
}
