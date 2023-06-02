#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>
#include <ESP8266WiFi.h>
#include <EspMQTTClient.h>

SoftwareSerial SWSerial( D2, D1 ); // RX, TX
#define GLED D8
#define RLED D7

 
const char* ssid     = "Nestori_2G";
const char* password = "Giulia10";

char BrokerURL[] = "broker.hivemq.com"; //URL do broker MQTT
char BrokerUserName[] = ""; //nome do usuario para autenticar no broker MQTT
char BrokerPassword[] = ""; //senha para autenticar no broker MQTT
char MQTTClientName[] = "mqtt-mack-pub"; //nome do cliente MQTT
int BrokerPort = 1883;

String TopicoPrefixo = "TESTMACK32152922"; //prefixo do topico
String Topico_01 = TopicoPrefixo+"/Entrada"; //nome do topico 01

EspMQTTClient clienteMQTT(ssid, password, BrokerURL, BrokerUserName, BrokerPassword, MQTTClientName, BrokerPort);

PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );
String tagId = "None", dispTag = "None";
byte nuidPICC[4];

void onConnectionEstablished() {
}
 
void setup(void)
{
  Serial.begin(115200);
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);
  clienteMQTT.enableDebuggingMessages();
  delay(10);

  // Connecting to a WiFi network
  Serial.println();
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

  Serial.println("Hello Maker!");
  //  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata)
  {
    Serial.print("Didn't Find PN53x Module");
    while (1); // Halt
  }
  // Got valid data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); 
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  // Configure board to read RFID tags
  nfc.SAMConfig();
  //Serial.println("Waiting for an ISO14443A Card ...");
}
 
 
void loop()
{
  readNFC();
  clienteMQTT.loop();

  if (clienteMQTT.isWifiConnected() == 1) {
    Serial.println("Conectado ao WiFi!");
  } else {
    Serial.println("Nao conectado ao WiFi!");
  }

  if (clienteMQTT.isMqttConnected() == 1) {
    Serial.println("Conectado ao broker MQTT!");
  } else {
    Serial.println("Nao conectado ao broker MQTT!");
  }

  Serial.println("Nome do cliente: " + String(clienteMQTT.getMqttClientName())
    + " / Broker MQTT: " + String(clienteMQTT.getMqttServerIp())
    + " / Porta: " + String(clienteMQTT.getMqttServerPort())
  );

  delay(1000);
}
 
 
void readNFC()
{
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  if (success)
  {
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
      nuidPICC[i] = uid[i];
      Serial.print(" "); Serial.print(uid[i], DEC);
    }
    Serial.println();
    tagId = tagToString(nuidPICC);
    dispTag = tagId;
    Serial.print(F("tagId is : "));
    Serial.println(tagId);
    Serial.println("");
    if(tagId == "147.194.106.27" || tagId == "101.48.147.106"){
      digitalWrite(GLED, HIGH);
      Serial.print("Acesso liberado");
      String entrada = tagId + " Acesso liberado";
      clienteMQTT.publish(Topico_01, String(entrada));
      yield();
      delay(2000);  // 1 second halt
      digitalWrite(GLED, LOW);
    }
    else{
      digitalWrite(RLED, HIGH);
      Serial.print("Acesso negado");
      String entrada = tagId + " Acesso negado";
      clienteMQTT.publish(Topico_01, String(entrada));
      yield();
      delay(2000);  // 1 second halt
      digitalWrite(RLED, LOW);
    }
  }
}
String tagToString(byte id[4])
{
  String tagId = "";
  for (byte i = 0; i < 4; i++)
  {
    if (i < 3) tagId += String(id[i]) + ".";
    else tagId += String(id[i]);
  }
  return tagId;
}