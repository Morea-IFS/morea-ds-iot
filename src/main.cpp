// ############# LIBRARIES ############### //
// Wifi Librarie
#include <ESP8266WiFi.h>

// HTTP Request Libraries
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>

// Layout Libraries
#include <Icons.h>
#include <Layout.h>

// Json Librarie
#include <ArduinoJson.h>

// Sensor Librarie
#include "EmonLib.h"

// OTA and Telnet Libraries
#include <ArduinoOTA.h>
#include <TelnetStream.h>

// Creates an object for energy monitoring
EnergyMonitor monitor;

// ############# CONSTANTS ############### //

#define PIN_SENSOR A0       // Connect This Pin to The Sensor (A0 on NodeMCU v3)
#define DEBUG_BUTTON_PIN 15 // Define debug button pin (D8 on NodeMCU v3)

// ############# VARIABLES ###############

// WiFi Network
const char *SSID = "Morea-Mobile";     // WiFi SSID
const char *PASSWORD = "p@ssw0rd1234**"; // WiFi Password

// OTA Config
const char* OTA_HOSTNAME = "E-MOTE-OTA";
const char* OTA_PASSWORD = "total123**"; 

// URL Data
String url = "https://morea-ifs.org/api";
const uint8_t fingerprint[20] = {0x6F, 0x53, 0x23, 0x2D, 0x04, 0x09, 0x2F, 0xF1, 0x9D, 0xBD, 0x43, 0xA4, 0xA3, 0xF6, 0xF3, 0x2F, 0x39, 0xAC,0xAF,0xCB};

// API and Device Variables
String apiToken;

// Others
const unsigned long interval = 5000; // Sample Collection Interval (In Milliseconds)
unsigned long previousMillis = 0;    // Variable to store the last time the code was executed

// Measure Variables
float amperes = 0;
float med_amperes = 0;
float watt = 0;
float kWh = 0;
unsigned long lastMeasurement = 0;
int voltage = 127;
int cycles = 60;
bool DEBUG = true;
byte i = 0;

// Button Debounce Variables
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// ############# PROTOTYPES ############### //

void initWiFi();
void handleDebugButton();
void printLog(const String& message);
void setupOTA();
void setupTelnet();

// ############### OBJECTS ################# //

BearSSL::WiFiClientSecure client;
HTTPClient http;
JsonDocument doc;
Layout layout(128, 64, -1);
Icons icons;


// ############### SETUP ################# //

void setup() {
  Serial.begin(115200);
  monitor.current(PIN_SENSOR, 88.188);
  client.setInsecure();

  pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);

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
  printLog("Endereço Mac: " + macAddress);

  initWiFi();
  setupOTA();
  setupTelnet();

  String path = url + "/authenticate";

  if (http.begin(client, path)) {
    String serializedData;
    doc["macAddress"] = macAddress;
    doc["deviceIp"] = WiFi.localIP().toString();
    serializeJson(doc, serializedData);

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(serializedData);
    String payload = http.getString();

    if (httpResponseCode > 0) {
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        printLog("Deserialization error");
        layout.drawIcon(6, icons.failedIcon());
      } else {
        String deviceName = doc["deviceName"].as<String>();
        apiToken = doc["api_token"].as<String>();
        
        if (doc.containsKey("voltage")) {
          voltage = doc["voltage"].as<int>();
          printLog("Voltage: " + String(voltage) + "V");
          layout.writeLine(4, "Voltage: " + String(voltage) + "V");
        } else {
          voltage = 127;
          layout.writeLine(4, "Voltage: N/A");
        }

        printLog("Device Name: " + deviceName);
        printLog("API Token: " + apiToken);

        if (deviceName == "null") {
          layout.writeLine(3, "Device: Unnamed");
        } else {
          layout.writeLine(3, "Device: " + deviceName);
        }
        layout.drawIcon(6, icons.keyIcon());
      }
    } else {
      printLog("Authentication request error: " + String(httpResponseCode));
      layout.drawIcon(6, icons.failedIcon());
    }

    doc.clear();
    http.end();
  } else {
      printLog("Unable to connect for authentication");
      layout.drawIcon(6, icons.failedIcon());
  }
}

// ############### LOOP ################# //

void loop() {
  ArduinoOTA.handle(); // Must be at the beginning of the loop

  if (WiFi.status() != WL_CONNECTED) {
    layout.drawIcon(7, icons.loadingIcon());
    initWiFi();
  }

  handleDebugButton();

  if (millis() - previousMillis >= interval) {
    previousMillis = millis(); // Update the timer

    // ############### SENSOR CODE ################# //

    amperes = monitor.calcIrms(1480);
    med_amperes +=  amperes;
    watt = amperes * voltage; 

    unsigned long currentTime = millis();
    if (lastMeasurement > 0) {
      float tempoDecorrido = (currentTime - lastMeasurement) / 3600000.0;
      kWh += (watt * tempoDecorrido) / 1000.0;
    }
    lastMeasurement = currentTime;

    i++;

    layout.writeLine(0, "Corrente: " + String(amperes) + "A");
    layout.writeLine(1, "Potencia: " + String(watt) + "w");
    layout.writeLine(2, "Consumo: " + String(kWh, 4) + "kWh");

    if (DEBUG) {
      printLog("--- Leitura ---");
      printLog("Corrente: " + String(amperes) + " A");
      printLog("Potência: " + String(watt) + " W");
      printLog("Energia gasta (acumulada): " + String(kWh, 6) + " kWh");
      printLog("Número de Coletas Atuais: " + String(i) + "/" + String(cycles));
    }

    if (i >= cycles) {
      med_amperes = (med_amperes/cycles); 
      String path = url + "/store-data";

      if (http.begin(client, path)) {
        String serializedData;
        doc["apiToken"] = apiToken;
        doc["macAddress"] = WiFi.macAddress();
        doc["measure"][0]["type"] = 2;
        doc["measure"][0]["value"] = kWh;
        doc["measure"][1]["type"] = 3;
        doc["measure"][1]["value"] = watt;
        doc["measure"][2]["type"] = 4;
        doc["measure"][2]["value"] = med_amperes;

        serializeJson(doc, serializedData);

        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(serializedData);
        String payload = http.getString();

        if (httpResponseCode == HTTP_CODE_OK || httpResponseCode == HTTP_CODE_CREATED) {
          DeserializationError error = deserializeJson(doc, payload);
          if (error) {
            printLog("Deserialization error on response");
            layout.drawIcon(5, icons.failedIcon());
          } else {
            String responseMessage = doc["message"].as<String>();
            printLog("Response Message:  " + responseMessage);
            layout.drawIcon(5, icons.successIcon());
          }
        } else {
          printLog("Falha no envio de dados. HTTP Code: " + String(httpResponseCode));
          printLog("Payload: " + payload);
          layout.drawIcon(5, icons.failedIcon());
        }

        doc.clear();
        http.end();
      } else {
        printLog("Falha ao iniciar conexão HTTP para envio de dados.");
        layout.drawIcon(5, icons.failedIcon());
      }
      i = 0;
      kWh = 0;
      med_amperes = 0;
    }
    layout.updateTimer(i, interval/1000, cycles);
  }
}

// ############# FUNCTIONS ############### //

void printLog(const String& message) {
  Serial.println(message);
  TelnetStream.println(message);
}

void initWiFi() {
  delay(10);
  printLog("Connecting to: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("."); // Using Serial here to show progress on one line
  }
  Serial.println(); // New line after connecting
  printLog("Connected to Network: " + String(SSID) + " | IP => " + WiFi.localIP().toString());

  layout.drawIcon(7, icons.wifiIcon());
}

void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    printLog("Iniciando OTA...");
  });
  ArduinoOTA.onEnd([]() {
    printLog("\nOTA Finalizado.");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char buffer[64];
    sprintf(buffer, "Progresso: %u%%", (progress / (total / 100)));
    TelnetStream.println(buffer); // Progress only to Telnet to avoid flooding Serial
  });
  ArduinoOTA.onError([](ota_error_t error) {
    printLog("Erro OTA: " + String(error));
  });

  ArduinoOTA.begin();
  printLog("OTA Pronto.");
}

void setupTelnet() {
  TelnetStream.begin();
  printLog("Telnet pronto.");
}

void handleDebugButton() {
  int reading = digitalRead(DEBUG_BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      DEBUG = !DEBUG;
      if (DEBUG) {
        cycles = 3;
        layout.drawIcon(1, icons.wrenchIcon());
        printLog("Debug mode ativado. Enviando dados a cada 3 coletas.");
      } else {
        cycles = 60;
        layout.eraseIcon(1);
        printLog("Debug mode desativado. Enviando dados a cada 60 coletas.");
      }
      i = 0;
    }
  }

  lastButtonState = reading;
}