//Bibliotecas
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <stdio.h>

//Dados para conexão Wi-Fi
const char* SSID = "Felipe"; // rede wifi
const char* PASSWORD = "total123**"; // senha da rede wifi

const char* BASE_URL = "http://www.google.com/";

#define PIN_SENSOR  13 //Pino a ser utilizado para o sensor. GPIO 13 (D7 no NodeMCU v3)
#define DEBUG  1 //DEBUG = 1 (habilita mensagens no monitor serial)

//Variaveis necessarias
volatile byte contaPulso; //Variável para a quantidade de pulsos
float freq; //Variável para frequência em Hz (pulsos por segundo)
float vazao=0; //Variável para armazenar o valor em L/min
float litros = 0; //Variável para o volume de água em cada medição
float volume = 0; //Variável para o volume de água acumulado
byte i;
byte sentSamples=0;
const byte intervalo = 5; //Intervalo de coleta das amostras (em segundos) 

char buffer[512];

void ICACHE_RAM_ATTR incpulso();

WiFiClient client;
HTTPClient http;

void setup() {
  Serial.begin(9600);

  initWifi();

  pinMode(PIN_SENSOR, INPUT); //Configura o pino do sensor como entrada
  attachInterrupt(PIN_SENSOR, incpulso, RISING); //Associa o pino do sensor para interrupção
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    return;
  }

  contaPulso = 0;//Zera a variável
   
  sei(); //Habilita interrupção 
  delay (intervalo*1000); //Aguarda um intervalo X (em milisegundos)
  cli(); //Desabilita interrupção
  
  freq = (float) contaPulso / intervalo; // Calcula a frequência em pulsos por segundo 
    
  vazao = freq / 8.0 ; //Converte para L/min, sabendo que 8 Hz (8 pulsos por segundo) = 1 L/min
  litros = vazao / (60/intervalo); //Recebe o volume em Litros consumido no intervalo atual.
  volume += litros; //Acumular o volume total desde o início da execução
   
  i++;

  if (DEBUG){ 
    //Exibe no monitor serial os dados coletados
    Serial.print("Vazao: ");
    Serial.println(vazao);
    Serial.print("Volume: ");
    Serial.println(volume);
    Serial.print("Número de coletas realizadas no minuto atual: ");
    Serial.println(i);
  }
  
  // Envia os dados a cada 60 segundos
  if(i == 2){
    i = 0;

    sprintf(buffer, "%sapi/?consumoAtual=%d&consumoTotal=%d&localColeta=CTI-Bebedouro&nodeID=1", BASE_URL, vazao, volume);

    Serial.println(buffer);

    if (http.begin(client, BASE_URL)){
      int httpCode = http.GET();
      Serial.println(httpCode);
      if (httpCode > 0){
        String payload = http.getString();
        Serial.println(payload);
      }
    }
  }

  http.end();
}

void ICACHE_RAM_ATTR incpulso (){
 contaPulso++; //Incrementa a variável de pulsos
}

void initWifi() {
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