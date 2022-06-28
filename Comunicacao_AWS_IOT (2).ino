// Configurando bibliotecas a serem usadas no modelo

#include "Secrets.h"
#include <WiFiClientSecure.h> // Talvez esta biblioteca só seja inestalada quando o PC reconhecer em uma de suas portas USB, um ESP32
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#include "DHT.h"

#define DHTPIN 14 // Porta digital conectada ao DHT Sensor
#define DHTTYPE DHT11 // DHT 11

#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

float c;
float t;
float a;

DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Função para conectar na AWS IOT
void connectAWS(){

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  } 

  // Configurando a WiFiClientSecure para usar a credecial do dispositivo AWS IOT
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  
  // Conectando-se ao agente MQTT broker do ponto final da AWS definido anteriormente
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  
  // Criando uma mensagem de manipulação
  client.setCallback(messageHandler);
  Serial.println("Conectando ao AWS IOT");
  while(!client.connect(THINGNAME)){
    Serial.print(".");
    delay(100);
  }
  if(!client.connected()){
    Serial.println("AWS IOT Timeout!");
    return;
  }

  // Subescrevendo para um topico
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IOT Conectado!");
}

// Função para encaminhar menssagem
void publicar_mensagem(){
  StaticJsonDocument<200> doc;
  doc["Corrente"] = c;
  doc["Tensao"] = t;
  doc["Angulo"] = a;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print para o client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(char* topic, byte* payload, unsigned int length){
  Serial.print("Entrada: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* mensagem = doc["mensagem"];
  Serial.println(mensagem);
}
void setup() {
  // Talvez esteja faltando iniciar o sistema com o SSID e PASSWORD
  Serial.begin(9600);
  connectAWS();
  dht.begin();
}

void loop() {
  c = random(0,10); // usar uma biblioteca random para simular valores de corrente em um intervalo
  t = random(0,10); // usar uma biblioteca random para simular valores de tensão em um intervalo
  a = random(0,10); // usar uma biblioteca random para simular valores de angulo em um intervalo

  if(isnan(c) || isnan(t) || isnan(a)){
      Serial.println("Failed to read from DHT sensor!");
      return;
  }
  
  Serial.print("Corrente: ");
  Serial.println(c);
  Serial.print("Tensão: ");
  Serial.println(t);
  Serial.print("Angulo °: ");
  Serial.println(a);

  publicar_mensagem();
  client.loop();
  delay(1000);
}
