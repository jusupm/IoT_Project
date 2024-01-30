#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>
#include <Servo.h>

#define ROW_NUM    4  // four rows
#define COLUMN_NUM 4  // four columns
#define SERVO_PIN  A0 // servo motor
#define ONE_WIRE_BUS 10 //dallas
#define photoResistorPin 1 //photoresistor
const int LED_PIN = 13;
const int buzzer = 12;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; //row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2}; //column pinouts of the keypad

Servo servo; 
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String input_password;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const String password_1 = "1234"; // change password here
int angle = 0; // the current angle of servo motor
int failedAttempts = 0; //password failed attempts
int light = 0; // store the current light value
unsigned long lastTime;
int motionStateCurrent = LOW; // current  state of motion sensor's pin
int motionStatePrevious = LOW; // previous state of motion sensor's pin

 
void setup() {
  sensors.begin();
	Serial.begin(9600); // sets the serial port to 9600

	//Serial.println("MQ2 warming up!");

  pinMode(buzzer, OUTPUT);
  pinMode(LED_PIN, OUTPUT); // configure digital pin  13 as an output

  input_password.reserve(8); // maximum password size is 8, change if needed
  servo.attach(SERVO_PIN);
  servo.write(0); // rotate servo motor to 0°
  lastTime = millis();
}
 
void loop() {
  char key = keypad.getKey();

  if (key) {
    //Serial.println(key);
    if (key == '*') {
      input_password = ""; // * reset the input password
    } else if (key == '#') {
      if (input_password == password_1) {
        //Serial.println("The password is correct, rotating Servo Motor to 90°");
        angle = 90;
        servo.write(angle);
        lastTime = millis();
        failedAttempts = 0;
      } else {
       //Serial.println("The password is incorrect, try again");
        failedAttempts++;
        tone(buzzer, 1000); 
        delay(1000); 
        noTone(buzzer);
        delay(100);

        if (failedAttempts >= 3) {
          //Serial.println("ALARM: Too many failed attempts");
          for(int i = 0; i < 5; i++){
            tone(buzzer, 1000); 
            delay(1000); 
            noTone(buzzer);
            delay(100);
          }
        }
      }
      Serial.println("p"+input_password);
      input_password = ""; // reset the input password
    } else {
      input_password += key; // append new character to input password string
    }
  }

  if (angle == 90 && (millis() - lastTime) > 5000) { 
    angle = 0;
    servo.write(angle);
    //Serial.println("Rotating Servo Motor to 0°");
  }

  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  String sTemp = String(temp);
  Serial.println("t"+sTemp);

  light = analogRead(photoResistorPin);  
  //Serial.println(temp);
  if(Serial.available() > 0){
    String readData = Serial.readStringUntil('\n');
    readData.trim();
    Serial.println(readData);
    if(readData.equals("M1") || readData.equals("M0")){
      if(readData.equals("M1")){
        motionStatePrevious = motionStateCurrent;             // store old state
        motionStateCurrent  = HIGH; // read new state
      }
      else if(readData.equals("M0")){
        motionStatePrevious = motionStateCurrent;
        motionStateCurrent = LOW;
      }
      if (motionStatePrevious == LOW && motionStateCurrent == HIGH && light < 200) { // pin state change: LOW -> HIGH
        //Serial.println("Motion detected! and light < 200");
        digitalWrite(LED_PIN, HIGH); // turn on
      }
      else
      if (motionStatePrevious == HIGH && motionStateCurrent == LOW || light >= 200) { // pin state change: HIGH -> LOW
        //Serial.println("Motion stopped! or light >= 200");
        digitalWrite(LED_PIN, LOW);  // turn off
      }
      //delay(100);
    }
    
    if(readData == "G1" && temp > 27){
      tone(buzzer, 1000); 
      delay(200); 
      noTone(buzzer);
      delay(100);
    }
  }
}
