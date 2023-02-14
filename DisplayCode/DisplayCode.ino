

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"


#define DHTPIN D3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);


const char* ssid = "Lusoto";
const char* password = "tel26828581";
const char* mqtt_server = "192.168.1.6";

#define RELAY 0  // relay connected to  GPIO0


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

char LamparaStatus[] = "0";

String clientId = "Display 1";
const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";
const char* MQTT_LIGHT_STATE_TOPIC = "devices/Display 1/status";


int digitalPin = 4;  //pin pour le capteur
int analogPin = A0;  //pin pour le capteur
int digitalVal;
int analogVal;


LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.println("RELAY=ON");
    digitalWrite(RELAY, LOW);
    digitalWrite(BUILTIN_LED, LOW);

    LamparaStatus[0] = '1';
    publishDeviceState();


  } else {
    Serial.println("RELAY=OFF");
    digitalWrite(RELAY, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);

    LamparaStatus[0] = '0';
    publishDeviceState();
  }
}



void publishDeviceState() {
  if (client.connected()) {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF, true);
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //client ID

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("device/temp", "MQTT Server is Connected");
      client.publish("device/register", clientId.c_str());
      publishDeviceState();
      // ... and resubscribe
      client.subscribe("device/led/Device 1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  dht.begin();

  pinMode(digitalPin, INPUT);  //la pin est en entrée

  pinMode(BUILTIN_LED, OUTPUT);  // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  setup_lcd();
  client.setServer(mqtt_server, 1833);
  client.setCallback(callback);
  lcd.clear();
}

void setup_lcd() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() {
  float h = dht.readHumidity();         //Reading the humidity
  float t = dht.readTemperature();      //Reading the temperature in Celsius degree
  float f = dht.readTemperature(true);  //Reading the temperature in Fahrenheit degrees


  lcd.setCursor(0, 0);
  lcd.print(t);

  lcd.setCursor(0, 1);
  lcd.print((String)h + " %");

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;

    dtostrf(t, 3, 2, LamparaStatus);


    snprintf(msg, MSG_BUFFER_SIZE, LamparaStatus);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(clientId.c_str(), msg);
  }


  digitalVal = digitalRead(digitalPin);
  if (digitalVal == HIGH)  //condition "si" : la valeur numérique est au niveau haut
  {
    Serial.print("DETECTADO");
  } else {
    Serial.print("NO DETECTADO");
  }
  analogVal = analogRead(analogPin);
  Serial.println(analogVal);  //afficher la valeur analogique
  delay(100);                 //délai 100ms
}
