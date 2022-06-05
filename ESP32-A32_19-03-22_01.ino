/*
 * 
 
 * topic A32...
 * ArduinoOTA hostname: "luciEstSUD-A32"
*/
//#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "config.h"
String programma="ESP32-A32_19-03-22_01";
//-----------------------------------------------------------
long rssi=0.0;       //RSSI.
char invio[6];
char temp[8];        //Ausiliario conversione float to c_str().
byte ESP_bloccata=0;
String lampada;
byte term=0;         //Ausiliario termometri.
float temperatura;   //Temperatura.

long t_rec=3000;
unsigned long t_prec_rec=0;
byte led1=1;
int t_led1=300;
byte led2=1;
int t_led2=500;
unsigned long t_prec_led1=0;
unsigned long t_prec_led2=0;
//ADC_MODE(ADC_VCC);
WiFiClientSecure mqttClient; //Client WiFi.
PubSubClient client; //Client MQTT.

byte contrMQTT=0;
//------------------------------------------------------------
void MQTT_connette(){
  client.setClient(mqttClient);
  mqttClient.setCACert(test_root_ca);
  mqttClient.setHandshakeTimeout(30); //16-01-22 Giorgio
  //mqttClient.setInsecure();
  client.setServer(mqtt_broker, mqtt_port);
  delay(5);
  client.setCallback(callback);
  while (!client.connected()) { //espClient
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
    } else {
      Serial.print("failed with state ");
      Serial.println(client.state());
    }
  }
  Serial.println("MQTT connesso");
  client.subscribe("A32/HE/stato");
  client.subscribe("A32/HE/lamp");
  delay(5);
}
//---Riceve messaggi da app .html--------------------------------
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("topic: ");
  Serial.print(topic);
  Serial.print("\nMessaggio: ");
  String messaggio;
  for (int i = 0; i < length; i++) {
    messaggio = messaggio + (char) payload[i];  //Converte *byte in String.
  }
  Serial.println(messaggio);
  if (strcmp(topic,"A32/HE/stato") == 0){       //HTML chiede stato ESP8266.
    Serial.println("Richiesta stato da HTML");
    client.publish("A32/EH/stato", "ok");
    client.publish("A32/EH/lamp",lampada.c_str());
    accessori(); //vBatt e RSSI
  }
  if (strcmp(topic,"A32/HE/lamp") == 0){
    accessori(); //vBatt e RSSI
    if(messaggio=="1"){
      Serial.print("LUCE ACCESA");
      digitalWrite(releA,HIGH);
      lampada="ACCESA";
    }else{
      Serial.print("luce spenta");
      digitalWrite(releA,LOW);
      lampada="spenta";
    }
  }
  Serial.println("-----------------------");
}
//----------------------------------------------------
void setup() {
  Serial.begin(115200);                //Velocità Serial monitor.
  Serial.println();
  pinMode(releA,OUTPUT); 
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  digitalWrite(releA,LOW);
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW); 
  connessioni();

  delay(20);
  for(int i=0;i<5;i++){
    digitalWrite(LED1,HIGH);
    delay(250);
    digitalWrite(LED1,LOW);
    delay(250);
  }
  if(digitalRead(releA)){
    lampada="ACCESA";
  }else{
    lampada="spenta";  
  }
  Serial.print("\nProgramma: ");
  Serial.println(programma);
}
//--------------------------------------------------------
void loop() {
  client.loop();
  if(WiFi.status()==WL_CONNECTED){
    lamp_LED1();  
  }
  if(WiFi.status()!=WL_CONNECTED){
    delay(1000);
    Serial.println("Sconnessione, restart()");
    WiFi.disconnect();
    delay(10000);
    ESP.restart();
    delay(10000);
  }  
  delay(30);  
} //loop()
//-------------------------------------------------
void connessioni(void){
  connetteWiFi();
  MQTT_connette();
}
//-------------------------------------------------
void connetteWiFi(void){
  Serial.print(ssid);                  //Vedi config.h.
  WiFi.begin(ssid, pass);              // "      "
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi CONNESSO");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  //WiFi.setAutoReconnect(true);
  //WiFi.persistent(true);
}
//-------------------------------------------------
void espSecure(void){
//  espClient.setFingerprint(fingerprint);
//  client.setClient(espClient);
  //espClient.setInsecure();
//  Serial.println("espSecure");
}
//-------------------------------------------------
void accessori(void){
  rssi = WiFi.RSSI();
  dtostrf(rssi, 6, 2, temp);
  client.publish("A32/EH/rssi",temp);

}
//-------------------------------------------------
void lamp_LED1(void){
  if(millis()-t_prec_led1>t_led1){
    if(led1){
      led1=0;
      digitalWrite(LED1,HIGH);
    }else{
      led1=1;
      digitalWrite(LED1,LOW);
    } 
    t_prec_led1=millis();
  }
}
//-------------------------------------------------
void lamp_LED2(void){
  if(millis()-t_prec_led2>t_led2){
    if(led2){
      led2=0;
      digitalWrite(LED2,HIGH);
    }else{
      led1=2;
      digitalWrite(LED2,LOW);
    } 
    t_prec_led2=millis();
  }
}
