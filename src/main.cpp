// ############# LIBRARIES ############### //
// Wifi Librarie
#include <ESP8266WiFi.h>

// HTTP Request Libraries
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <stdio.h>

// Layout Libraries
#include <Icons.h>
#include <Layout.h>

// Json Librarie
#include <ArduinoJson.h>

// Sensor Librarie
#include "EmonLib.h"


// Creates an object for energy monitoring
EnergyMonitor monitor;

// ############# CONSTANTS ############### //

#define PIN_SENSOR A0       // Connect This Pin to The Sensor.(A0 on NodeMCU v3)
#define DEBUG_BUTTON_PIN 15 // Define debug button pin (D8 on NodeMCU v3)
// ############# VARIABLES ###############

// WiFi Network
const char *SSID = "Morea-Mobile";      // WiFi SSID
const char *PASSWORD = "p@ssw0rd1234**"; // WiFi Password
// const char *SSID = "Morea-Mobile";       // SSID do WiFi
// const char *PASSWORD = "p@ssw0rd1234**"; // Senha do WiFi

// URL Data
// String url = "https://192.168.0.105";                                                                                                                  // URL do site (usando HTTP e não HTTPS)
String url = "http://192.168.1.105";                                                                                                                    // URL do site (usando HTTP e não HTTPS)
const uint8_t fingerprint[20] = {0x44, 0x5D, 0x07, 0x68, 0x0F, 0xBF, 0x25, 0x26, 0xE4, 0xB5, 0x04, 0x35, 0x6D, 0x91, 0xAD, 0x96, 0xFE, 0xBF, 0x40, 0x8B}; // Impressão digital do servidor

String serializedData;
String apiToken;\
String path;

// Others
const byte interval = 5; // Sample Collection Interval (In Seconds)

// Measure Variables
volatile byte countPulse; // Variable for the Quantity of Pulses
float freq;               // Variable to the Frequency in Hz (Pulses per Second)
float amperes= 0;         // Variable to store the value in amps
float watt = 0;           // Variable to store the value in WATTS
float kWh = 0;            // Variable to calculate the kWh for each measurement
unsigned long lastMeasurement = 0;
const int tensao = 110;  // Tensão da rede elétrica em volts
int cycles = 60;
bool DEBUG = true; // DEBUG = true (Enables the Debug Mode)
byte i;            // Act as a Counter Variable

// ############# PROTOTYPES ############### //

void initWiFi();                 // Connect to the WiFi
void httpRequest(String path);   // Call the Request Function and Store the Payload
String makeRequest(String path); // Make the Request Itself

void ICACHE_RAM_ATTR incpulso();

// ############### OBJECTS ################# //

WiFiClient client;
HTTPClient http;
JsonDocument doc;
Layout layout(128, 64, -1);
Icons icons;

// ############### SETUP ################# //

void setup() {
  Serial.begin(115200);
  monitor.current(PIN_SENSOR, 60);  


  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setFingerprint(fingerprint);

  pinMode(PIN_SENSOR, INPUT);                    // Configure pin Sensor as Input
  pinMode(DEBUG_BUTTON_PIN, INPUT);              // Configure button sensor as input
  
  
  // Drawing Basic Layout on Display
  layout.beginLayout();
  layout.beginTimer();
  layout.drawLogo(icons.sparcLogo());
  layout.drawIcon(0, icons.electricityIcon());
  layout.drawIcon(5, icons.loadingIcon());
  layout.drawIcon(6, icons.loadingIcon());
  layout.drawIcon(7, icons.loadingIcon());

  // Get Mac Address
  String macAddress = WiFi.macAddress();
  Serial.println("Endereço Mac: " + macAddress);

  // Init Wifi And Connect
  initWiFi();

  path = url + "/api/authenticate";

  //  Sending Mac Address to the Server
  if (http.begin(*client, path)) {
    // Data serializing
    doc["macAddress"] = macAddress;
    doc["deviceIp"] = WiFi.localIP().toString();
    serializeJson(doc, serializedData);

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(serializedData);
    String payload = http.getString();

    if (httpResponseCode < 0) {
      Serial.println("request error - " + httpResponseCode);
    }

    if (httpResponseCode != HTTP_CODE_OK && httpResponseCode != HTTP_CODE_CREATED) {
      Serial.println("Falha no envio");
      Serial.println(httpResponseCode);
    }

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.println("Deserialization error");
      layout.drawIcon(6, icons.failedIcon());

      return;
    }

    String deviceName = doc["deviceName"].as<String>();
    apiToken = doc["api_token"].as<String>();

    Serial.println("Device Name: " + deviceName);
    Serial.println("API Token: " + apiToken);

    if (deviceName == "null") {
      layout.writeLine(3, "Device: Unnamed");
    } else {
      layout.writeLine(3, "Device: " + deviceName);
    }
    layout.drawIcon(6, icons.keyIcon());

    doc.clear();
    http.end();
  }
}

// ############### LOOP ################# //

void loop() {
  if (WiFi.status() != WL_CONNECTED) // Verify if WiFi is Connected
  {
    layout.drawIcon(7, icons.loadingIcon());

    initWiFi(); // Try to Reconnect if Not
  }

  if (digitalRead(DEBUG_BUTTON_PIN) == HIGH && DEBUG == false) {
    DEBUG = true;
    cycles = 3;
    i = 0;

    layout.drawIcon(1, icons.wrenchIcon());

    Serial.println("Debug mode ativado.");
  } else if (digitalRead(DEBUG_BUTTON_PIN) == HIGH && DEBUG == true) {
    DEBUG = false;
    cycles = 60;
    i = 0;

    layout.eraseIcon(1);

    Serial.println("Debug mode desativado.");
  }

  // ############### SENSOR CODE ################# //
    countPulse = 0; // Zero the Variable

  sei();                  // Enable Interruption
  delay(interval * 1000); // Await the Interval X (in Millisecond)
  // cli();               // Disable Interruption

  freq = (float)countPulse / (float)interval; // Calculate the Frequency in Pulses per Second

  
  
 // Calculate the current using the `calcIrms` function
  amperes = monitor.calcIrms(1480);  
  // Calculate power by multiplying current by voltage
  watt = amperes * tensao;  

  // Update the energy spent
  unsigned long currentTime  = millis();  
  if (lastMeasurement > 0) {
    // Calculate elapsed time in hours
    float tempoDecorrido = (currentTime  - lastMeasurement) / 3600000.0;  
    // Calculates energy spent in kWh
    kWh += watt * tempoDecorrido / 1000.0;  
  }
  // Update the time of the last measurement
  lastMeasurement = currentTime ;
  
  i++;

  layout.writeLine(0, "Corrente: " + String(amperes) + "A");
  layout.writeLine(1, "Potencia: " + String(watt) + "w");
  layout.writeLine(2, "Consumo: " + String(kWh) + "kWh");


  if (DEBUG)  // Show the Data in the Monitor Serial
  {
    Serial.println("Corrente: " + String(amperes));
    Serial.println("Potência: " + String(watt));
    Serial.println("Energia gasta: " + String(kWh) + "kWh");
    Serial.println("Número de Coletas Atuais: " + String(i));
  }

  String macAddress = WiFi.macAddress();
  path = url + "/api/store-data";

  std::unique_ptr<BearSSL::WiFiClientSecure>
      client(new BearSSL::WiFiClientSecure);
  client->setFingerprint(fingerprint);

  //  Sending Ip Address to the Server
  if (http.begin(*client, path) && i == cycles) {
    // Data serializing
    doc["apiToken"] = apiToken;
    doc["macAddress"] = macAddress;
    doc["measure"][0]["type"] = 2;
    doc["measure"][0]["value"] = kWh;
    doc["measure"][1]["type"] = 3;
    doc["measure"][1]["value"] = watt;
    doc["measure"][2]["type"] = 4;
    doc["measure"][2]["value"] = amperes;

    serializeJson(doc, serializedData);

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(serializedData);
    String payload = http.getString();

    if (httpResponseCode < 0) {
      Serial.println("request error - " + String(httpResponseCode));

      layout.drawIcon(5, icons.failedIcon());

      i = 0;
      kWh = 0;
      watt = 0;
      amperes = 0;
      

      layout.updateTimer(i, interval, cycles);
    }

    if (httpResponseCode != HTTP_CODE_OK) {
      Serial.println("Falha no envio");
      Serial.println(httpResponseCode);

      layout.drawIcon(5, icons.failedIcon());

      i = 0;
      kWh = 0;
      watt = 0;
      amperes = 0;

      layout.updateTimer(i, interval, cycles);
    }

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.println("Deserialization error");

      layout.drawIcon(5, icons.failedIcon());

      i = 0;
      kWh = 0;
      watt = 0;
      amperes = 0;

      layout.updateTimer(i, interval, cycles);

      doc.clear();
      http.end();

      return;
    }

    String responseMessage = doc["message"].as<String>();
    Serial.println("Response Message:  " + responseMessage);

    i = 0;
    kWh = 0;
    watt = 0;
    amperes = 0;

    layout.updateTimer(i, interval, cycles);
    layout.drawIcon(5, icons.successIcon());

    doc.clear();
    http.end();
  }
  layout.updateTimer(i, interval, cycles);
}

void ICACHE_RAM_ATTR incpulso() {
  countPulse++; // Increments Pulse Variable
}


void initWiFi() {
  delay(10);
  Serial.println("Connecting to: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to Netowk: " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());

  layout.drawIcon(7, icons.wifiIcon());
}
