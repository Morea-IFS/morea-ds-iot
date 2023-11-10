// ############# LIBRARIES ############### //

// Wifi Librarie
#include <ESP8266WiFi.h>

// HTTP Request Libraries
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <stdio.h>
#include <Wire.h>

// Display Libraries
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ############# Display Reading ############### //

// Screen Size Variables
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);     // Display Reading Declaration

// ############# ICONS ############### //

static const unsigned char PROGMEM loadingIcon[] = {
    B00001000,
    B01100100,
    B00100010,
    B10000001,
    B10000001,
    B01000100,
    B00100110,
    B00010000
};

static const unsigned char PROGMEM wifiIcon[] = {
    B00111000,
    B01000100,
    B10000010,
    B00111000,
    B01000100,
    B00000000,
    B00010000,
    B00000000
};

static const unsigned char PROGMEM waterIcon[] = {
    B00011000,
    B00011000,
    B00111100,
    B00111100,
    B01111110,
    B01111110,
    B00111100,
    B00011000
};

static const unsigned char PROGMEM failedIcon[] = {
    B10000010,
    B01000100,
    B00101000,
    B00010000,
    B00101000,
    B01000100,
    B10000010,
    B00000000
};

static const unsigned char PROGMEM successIcon[] = {
    B00000000,
    B00000001,
    B00000010,
    B00000100,
    B10001000,
    B01010000,
    B00100000,
    B00000000
};

// ############# CONSTANTS ############### //

#define PIN_SENSOR 13                                   // Connect This Pin to The Sensor. GPIO 13 (D7 on NodeMCU v3)
#define DEBUG 1                                         // DEBUG = 1 (Enables the Debug Mode)

// ############# VARIABLES ############### //

// WiFi Network
const char* SSID = "Morea";                             // WiFi SSID
const char* PASSWORD = "morea123**";                    // WiFi Password

// URL Data
const char *BASE_URL = "http://api.morea-ifs.org";      // WebSite URL (using HTTP and not HTTPS)

const int moteId = 1;                                   // MoteId, Defined in the WebSite Database
const char *secretKey = "7$WMX70b9$";                   // Mote Key, The Server Only Accept Requests if The Key is Correct

char url[512];                                          // This Variable Will be Concatenated With Others to Make the Complete URL 

// Others
const byte interval = 5;                                // Sample Collection Interval (In Seconds)

// Measure Variables
volatile byte countPulse;                               // Variable for the Quantity of Pulses
float freq;                                             // Variable to the Frequency in Hz (Pulses per Second)
float flowRate = 0;                                     // Variable to Store The Value in L/min
float liters = 0;                                       // Variable for the Water Volume in Each Measurement
float volume = 0;                                       // Variable for the Accumulated Water Volume
byte i;                                                 // Act as a Counter Variable

// ############# PROTOTYPES ############### //

void initWiFi();                                        // Connect to the WiFi
void httpRequest(String path);                          // Call the Request Function and Store the Payload
String makeRequest(String path);                        // Make the Request Itself
void setDefaultDisplay();
void setErrorDisplay();
void setSuccessDisplay();
void updateDisplayTimer();

void ICACHE_RAM_ATTR incpulso();

// ############### OBJECTS ################# //

WiFiClient client;
HTTPClient http;

// ############### SETUP ################# //

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_SENSOR, INPUT);                             // Configure pin Sensor as Input
  attachInterrupt(PIN_SENSOR, incpulso, RISING);          // Associate Sensor pin for Interrupt

  // Display Initialization

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))          // Try to Detect and Inicialize Display
  {        
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);                                              // Don't proceed, loop forever
  }

  //Drawing Basic Layout on Display
  display.clearDisplay();

  display.writeFastHLine(0, 13, 128, SSD1306_WHITE);
  display.drawBitmap(3, 3, waterIcon, 8, 8, 1);
  display.drawBitmap(109, 3, loadingIcon, 8, 8, 1);
  display.drawBitmap(119, 3, loadingIcon, 8, 8, 1);

  // Write Timer
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(54, 3);
    display.println("0:00");

  display.display();
}

// ############### LOOP ################# //

void loop()
{
  if (WiFi.status() != WL_CONNECTED)                      // Verify if WiFi is Connected
  {
    // Display Manipulation
    display.writeFillRect(119, 3, 8, 8, SSD1306_BLACK);
    display.drawBitmap(119, 3, loadingIcon, 8, 8, 1);

    display.display();

    initWiFi();                                           // Try to Reconnect if Not
  }

  // ############### CODE FOR THE SENSOR ################# //
  countPulse = 0;                                         // Zero the Variable

  sei();                                                  // Enable Interruption
  delay(interval * 1000);                                 // Await the Interval X (in Millisecond)
  // cli();                                               // Disable Interruption

  freq = (float)countPulse / (float)interval;             // Calculate the Frequency in Pulses per Second

  flowRate = freq / 7.5;                                  // Convet to L/min, Knowing That 8 Hz (8 Pulses per Second) = 1 L/min
  liters = flowRate / (60.0 / (float)interval);           // Liters Consumed in the Current Interval   --Recebe o volume em liters consumido no interval atual.
  volume += liters;                                       // Variable for the Accumulated Water Volume

  i++;

  if (DEBUG)                                              // Show the Data in the Monitor Serial
  {                                             
    Serial.println("Flow Rate: " + String(flowRate));
    Serial.println("Current Volume: " + String(volume));
    Serial.println("Numbers of Current Collections: " + String(i));
  }

  sprintf(url, "%s/?secretKey=%s&nodeID=%d&consumoAtual=%f", BASE_URL, secretKey, moteId, volume);    // Concatenates the Char Variables and Form the Request URL with the Data

  httpRequest(url);                                       // Call the HTTPRequest Function
}

// ############# HTTP REQUEST ################ //

void httpRequest(String path)
{
  String payload = makeRequest(path);

  Serial.println(payload);

  if (!payload)
  {
    setErrorDisplay();

    return;
  }
}

String makeRequest(String path)
{
  if (http.begin(client, path) && i == 60)
  {
    int httpCode = http.GET();

    if (httpCode < 0)
    {
      Serial.println("Request Error - " + httpCode);

      i = 0;
      volume = 0;
      
      updateDisplayTimer();
      
      return "";
    }

    if (httpCode != HTTP_CODE_OK)
    {
      setErrorDisplay();

      Serial.println("Sending Failed:");
      Serial.println(httpCode);

      i = 0;
      volume = 0;

      updateDisplayTimer();

      return "";
    }
    else
    {
      setSuccessDisplay();

      i = 0;
      volume = 0;
      
      updateDisplayTimer();

      return "";
    }

    String response = http.getString();
    http.end();

    Serial.println("Request Response:" + response);

    i = 0;
    volume = 0;

    updateDisplayTimer();

    return response;
  }
  else if (i == 60)
  {
    setErrorDisplay();

    i = 0;
    volume = 0;

    Serial.println("Request failed.");

    updateDisplayTimer();

    return "";
  }
  else
  {
    updateDisplayTimer();
    Serial.println("Wait for the request.");
    return "";
  }
}

// ###################################### //

// implementacao dos prototypes

void setDefaultDisplay()
{
  // LCD Manipulation
}

void setErrorDisplay()
{    
  // Display Manipulation
  display.writeFillRect(109, 3, 8, 8, SSD1306_BLACK);
  display.drawBitmap(109, 3, failedIcon, 8, 8, 1);

  display.display();
}

void setSuccessDisplay()
{
  // Display Manipulation
  display.writeFillRect(109, 3, 8, 8, SSD1306_BLACK);
  display.drawBitmap(109, 3, successIcon, 8, 8, 1);

  display.display();
}

void updateDisplayTimer()
{
  // Calculating the Timer Values
  int minutes = 4 - floor((i * 5) / 60);
  int seconds = 60 - ((i * 5) % 60);

  if(seconds == 60)
  {
    minutes = minutes + 1;
    seconds = 0;
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.writeFillRect(54, 3, 25, 8, SSD1306_BLACK);
  display.setCursor(54, 3);

  if(seconds < 10)
  {
    display.println(String(minutes) + ":0" + String(seconds));
  } else {
    display.println(String(minutes) + ":" + String(seconds));
  }

  display.display();
}

void ICACHE_RAM_ATTR incpulso()
{
  countPulse++; // Increments Pulse Variable
}

void initWiFi()
{
  delay(10);
  Serial.println("Connecting to: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to Netowk: " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());

  // Display Manipulation
  display.writeFillRect(119, 3, 8, 8, SSD1306_BLACK);
  display.drawBitmap(119, 3, wifiIcon, 8, 8, 1);

  display.display();
}