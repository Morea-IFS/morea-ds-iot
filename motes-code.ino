//Incluindo as bibliotecas necessárias
#include <ESP8266WiFi.h>

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

// Nome da sua rede Wifi
const char* ssid = "FELIPE-PC";
//const char* ssid = "";
// Senha da rede
const char* password = "196Y5@a7";
//const char* password = "";
// Site que receberá os dados - IMPORTANTE: SEM O HTTP://
const char* host = "172.25.108.176"; //www.site.com.br
//const char* host = "morea.ifslab.edu.br"; //www.site.com.br
//const char* host = ""; 
const int httpPort = 8000;

unsigned long startTime=0;
unsigned long respTime=0;
unsigned long connTime=0;
unsigned long transfTime=0;
void ICACHE_RAM_ATTR incpulso();

void setup() {
  // Iniciando o Serial
  Serial.begin(9600);

  pinMode(PIN_SENSOR, INPUT); //Configura o pino do sensor como entrada
  attachInterrupt(PIN_SENSOR, incpulso, RISING); //Associa o pino do sensor para interrupção

  delay(500);
  Serial.println("\nProjeto MOREA - Monitoramento em Tempo Real de Consumo de Eletricidade e Água");
  Serial.println("Exibindo tempos no formato: tempo_de_conexao,tempo_inicio_resposta,tempo_fim_resposta");
  conexaoWiFi();
}

void loop() {

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
  if(i == (60/intervalo)){
    
      i = 0;
      // Criando uma conexao TCP
      WiFiClient client;
      startTime=millis();
      if (!client.connect(host, httpPort)) {
        Serial.println("Erro!");
        return;
      }        
      connTime=millis()-startTime;
      //Envia dados para o servidor através do método GET do HTTP 
      client.print("GET /api/?consumoAtual=" + 
                  String(vazao,2) + "&consumoTotal=" + String(volume,2) + 
                  "&localColeta=CTI-Bebedouro&nodeID=3" + " HTTP/1.1\r\n" +
                  "Host: " + host + "\r\n" +
                  "Connection: close\r\n" +
                  "\r\n"
                  );
      Serial.println('h');

      int it=0;
      while (client.connected()){
        String response = client.readStringUntil('\n');
        if (it==0){ 
          respTime=millis()-startTime; 
        }
        
        it++;
        
        if (response == "/r"){
          Serial.println("Headers received");          
          break;
        }                    
      }
      transfTime=millis()-startTime;
      //Serial.print("Tempo de conexão: ");      
      sentSamples++;      
      Serial.print(sentSamples);
      Serial.print(",");
      Serial.print(connTime);
      Serial.print(",");      
      //Serial.print("Tempo de resposta: ");
      Serial.print(respTime);
      Serial.print(",");
      //Serial.print("Transference time: ");
      Serial.println(transfTime);
      if (DEBUG){
        //Exibe no monitor serial os dados enviados
        Serial.println("Dados enviados!");
        Serial.print("Vazao: ");
        Serial.println(vazao);
        Serial.print("Volume: ");
        Serial.println(volume);
      }
      
  }
  
  
}

void ICACHE_RAM_ATTR incpulso ()
{
 contaPulso++; //Incrementa a variável de pulsos
}

void conexaoWiFi(){
  // Conectando na rede wifi  
  Serial.print("Conectando");
  WiFi.begin(ssid,password);  
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Conectado a rede: ");
  Serial.println(ssid);
  
  printWifiData();  
}

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");  
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);

  // print your subnet mask:
  IPAddress subnet = WiFi.subnetMask();
  Serial.print("NetMask: ");
  Serial.println(subnet);

  // print your gateway address:
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(gateway);
}
