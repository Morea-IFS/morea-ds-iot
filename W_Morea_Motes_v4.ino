// ############# LIBRARIES ############### //

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <stdio.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>

// Read Display
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte water[8] = {
    0b00100,
    0b00100,
    0b01110,
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b01110};

byte energy[8] = {
    0b00001,
    0b00011,
    0b00110,
    0b01100,
    0b11111,
    0b00110,
    0b01100,
    0b11000};

byte wifi[8] = {
    0b00000,
    0b01110,
    0b10001,
    0b00100,
    0b01010,
    0b00000,
    0b00100,
    0b00000};

byte Check[8] = {
    0b00000,
    0b00001,
    0b00011,
    0b10110,
    0b11100,
    0b01000,
    0b00000,
    0b00000};

byte fail[8] = {
    0b10001,
    0b01010,
    0b01010,
    0b00100,
    0b01010,
    0b01010,
    0b10001,
    0b00000};

// ############# VARIABLES ############### //

// Debug
const char *SSID = "IFSLIVRE"; // rede wifi
const char *PASSWORD = "";     // senha da rede wifi

// const char* BASE_URL = "http://192.168.240.73";

// Production
// const char* SSID = "Morea"; // rede wifi
// const char* PASSWORD = "p@assw0rd"; // senha da rede wifi

const char *BASE_URL = "http://api.morea-ifs.org";

const int moteId = 1;
const char *secretKey = "7$WMX70b9$";

char url[512];

#define PIN_SENSOR 13 // Pino a ser utilizado para o sensor. GPIO 13 (D7 no NodeMCU v3)
#define DEBUG 1       // DEBUG = 1 (habilita mensagens no monitor serial)

// Variaveis necessarias
volatile byte contaPulso; // Variável para a quantidade de pulsos
float freq;               // Variável para frequência em Hz (pulsos por segundo)
float vazao = 0;          // Variável para armazenar o valor em L/min
float litros = 0;         // Variável para o volume de água em cada medição
float volume = 0;         // Variável para o volume de água acumulado
byte i;
byte sentSamples = 0;
const byte intervalo = 5; // Intervalo de coleta das amostras (em segundos)

// ############# PROTOTYPES ############### //

void initSerial();
void initWiFi();
void httpRequest(String path);
String makeRequest(String path);
void setDefaultDisplay();
void setErrorDisplay();
void setSuccessDisplay();
void setIdDisplay();
void setVazaoDisplay();

void ICACHE_RAM_ATTR incpulso();

// ############### OBJECTS ################# //

WiFiClient client;
HTTPClient http;

// ############## SETUP ################# //

void setup()
{
  initSerial();

  pinMode(PIN_SENSOR, INPUT);                    // Configura o pino do sensor como entrada
  attachInterrupt(PIN_SENSOR, incpulso, RISING); // Associa o pino do sensor para interrupção

  // Display Initialization
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, water);
  lcd.createChar(1, energy);
  lcd.createChar(2, wifi);
  lcd.createChar(3, Check);
  lcd.createChar(4, fail);

  // LCD Manipulation
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.write(0);
  lcd.setCursor(3, 0);
  lcd.print("Morea Mote");
  lcd.setCursor(14, 0);
  lcd.write(1);

  lcd.setCursor(0, 1);
  lcd.print("Aguardando Dados");
}

// ############### LOOP ################# //

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {

    lcd.clear();

    lcd.setCursor(1, 0);
    lcd.write(0);
    lcd.setCursor(3, 0);
    lcd.print("Morea Mote");
    lcd.setCursor(14, 0);
    lcd.write(1);

    lcd.setCursor(1, 1);
    lcd.write(2);
    lcd.setCursor(3, 1);
    lcd.print("Conectando..");

    initWiFi();
  }

  // ############### CÓDIGO PARA O SENSOR ################# //
  contaPulso = 0; // Zera a variável

  sei();                   // Habilita interrupção
  delay(intervalo * 1000); // Aguarda um intervalo X (em milisegundos)
  // cli(); //Desabilita interrupção

  freq = (float)contaPulso / (float)intervalo; // Calcula a frequência em pulsos por segundo

  vazao = freq / 5.5;                         // Converte para L/min, sabendo que 8 Hz (8 pulsos por segundo) = 1 L/min
  litros = vazao / (60.0 / (float)intervalo); // Recebe o volume em Litros consumido no intervalo atual.
  volume += litros;                           // Acumular o volume total desde o início da execução

  i++;

  if (DEBUG)
  {
    // Exibe no monitor serial os dados coletados
    Serial.print("Vazao: ");
    Serial.println(vazao);
    Serial.print("Volume Atual: ");
    Serial.println(volume);
    Serial.print("Número de coletas realizadas no minuto atual: ");
    Serial.println(i);
  }

  sprintf(url, "%s/?secretKey=%s&nodeID=%d&consumoAtual=%f", BASE_URL, secretKey, moteId, volume);

  httpRequest(url);
}

// ############# HTTP REQUEST ################ //

void httpRequest(String path)
{
  String payload = makeRequest(path);

  Serial.println(payload);

  if (!payload)
  {
    return;
  }
}

String makeRequest(String path)
{
  setDefaultDisplay();

  if (http.begin(client, path) && i == 60)
  {

    int httpCode = http.GET();

    if (httpCode < 0)
    {
      Serial.println("request error - " + httpCode);

      i = 0;
      volume = 0;
      return "";
    }

    if (httpCode != HTTP_CODE_OK)
    {
      setErrorDisplay();

      Serial.println("Falha no Envio");
      Serial.println(httpCode);

      i = 0;
      volume = 0;
      return "";
    }
    else
    {
      setSuccessDisplay();

      i = 0;
      volume = 0;

      return "";
    }

    String response = http.getString();
    http.end();

    Serial.println("Request Response:" + response);

    i = 0;
    volume = 0;

    return response;
  }
  else if (i == 60)
  {
    i = 0;
    volume = 0;
    Serial.println("Request failed.");
    setErrorDisplay();
    return "";
  }
  else
  {
    Serial.println("Wait for the request.");
    return "";
  }
}

// ###################################### //

// implementacao dos prototypes

void setDefaultDisplay()
{
  // LCD Manipulation
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.write(0);
  lcd.setCursor(3, 0);
  lcd.print("Morea Mote");
  lcd.setCursor(14, 0);
  lcd.write(1);

  lcd.setCursor(0, 1);
  lcd.print("Consumo:");
  lcd.setCursor(11, 1);
  lcd.print(volume);
  lcd.setCursor(15, 1);
  lcd.print("L");
}

void setIdDisplay()
{
  // LCD Manipulation
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.write(0);
  lcd.setCursor(3, 0);
  lcd.print("Morea Mote");
  lcd.setCursor(14, 0);
  lcd.write(1);

  lcd.setCursor(3, 1);
  lcd.print("moteID:");
  lcd.setCursor(11, 1);
  lcd.print(moteId);
}

void setVazaoDisplay()
{
  // LCD Manipulation
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.write(0);
  lcd.setCursor(3, 0);
  lcd.print("Morea Mote");
  lcd.setCursor(14, 0);
  lcd.write(1);

  lcd.setCursor(0, 1);
  lcd.print("Vazao:");
  lcd.setCursor(9, 1);
  lcd.print(vazao);
  lcd.setCursor(13, 1);
  lcd.print("L/m");
}

void setErrorDisplay()
{
  // LCD Manipulation
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.write(0);
  lcd.setCursor(3, 0);
  lcd.print("Morea Mote");
  lcd.setCursor(14, 0);
  lcd.write(1);

  lcd.setCursor(1, 1);
  lcd.print("Falha no Envio");
}

void setSuccessDisplay()
{
  // LCD Manipulation
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.write(0);
  lcd.setCursor(3, 0);
  lcd.print("Morea Mote");
  lcd.setCursor(14, 0);
  lcd.write(1);

  lcd.setCursor(1, 1);
  lcd.print("Dados Enviados");
}

void initSerial()
{
  Serial.begin(115200);
}

void ICACHE_RAM_ATTR incpulso()
{
  contaPulso++; // Incrementa a variável de pulsos
}

void initWiFi()
{
  delay(10);
  Serial.println("Conectando-se em: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado na Rede " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());

  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.write(0);
  lcd.setCursor(3, 0);
  lcd.print("Morea Mote");
  lcd.setCursor(14, 0);
  lcd.write(1);

  lcd.setCursor(1, 1);
  lcd.write(2);
  lcd.setCursor(3, 1);
  lcd.print("Conectado!");
  lcd.setCursor(14, 1);
  lcd.write(3);

  delay(2500);

  setDefaultDisplay();
}