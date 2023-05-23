// ############# LIBRARIES ############### //

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
//WiFiClient wifiClient;

// ############# VARIABLES ############### //

const char* SSID = "ssid"; // rede wifi
const char* PASSWORD = "password"; // senha da rede wifi

String BASE_URL = "http://url";

// ############# PROTOTYPES ############### //

void initSerial();
void initWiFi();
void httpRequest(String path);

// ############### OBJECTS ################# //

WiFiClient client;
HTTPClient http;

// ############## SETUP ################# //

void setup() {
  initSerial();
  initWiFi();
}

// ############### LOOP ################# //

void loop() {
  Serial.println("[GET] /sensors - sending request...");
  Serial.println("");

  httpRequest("/");

  Serial.println("");
  delay(1000);

}

// ############# HTTP REQUEST ################ //

void httpRequest(String path)
{
  String payload = makeRequest(path);

  if (!payload) {
    return;
  }

  Serial.println("##[RESULTADO]## ==> " + payload);

}

String makeRequest(String path)
{
  
  if (http.begin(client, BASE_URL + path)){
   int httpCode = http.GET();

   if (httpCode < 0) {
     Serial.println("request error - " + httpCode);
     Serial.println(httpCode);
     return "";

   }

   if (httpCode != HTTP_CODE_OK) {
    return "";
   }

  String response =  http.getString();
  http.end();
  return response;
    } else{ Serial.println("begin error"); return "";
  }
}

// ###################################### //

// implementacao dos prototypes

void initSerial() {
  Serial.begin(9600);
}

void initWiFi() {
  delay(10);
  Serial.println("Conectando-se em: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado na Rede " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());
}