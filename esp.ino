#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
// Replace these with your Wi-Fi and MQTT broker details
const char* ssid = "bijeli64";
const char* password = "ferit.2023";
const char* mqttServer = "192.168.23.104";
const int mqttPort = 1883;
const char* mqttTopicTemp = "sensors/temperature";
const char* mqttTopicKeypad = "sensors/keypad";

 
String readData;

float temp;
String keypad;

bool publishTemperature = false;

WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() {
  Serial.begin(9600); 
  setupWifi();
  client.setServer(mqttServer, mqttPort);
}
 
void loop() {
  if (!client.connected()) {
    reconnect();
  }
 
  if(Serial.available() > 0){
    String readData = Serial.readStringUntil('\n');
    readData.trim();
      if(readData.charAt(0) == 'p'){
        keypad = readData.substring(1);    
        Serial.println(keypad);
        if(keypad.equals("1234")){
          publishTemperature = true;
        }
        client.publish(mqttTopicKeypad,keypad.c_str());
      }
      else if (readData.charAt(0) == 't' && publishTemperature){
        temp = readData.substring(1).toFloat();
        Serial.println(String(temp));
        String tempStr = String(temp);
        client.publish(mqttTopicTemp,tempStr.c_str());
        publishTemperature = false;
      }
  }
  
  //delay(1000); // Adjust the delay based on your requirements
}
 
void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
