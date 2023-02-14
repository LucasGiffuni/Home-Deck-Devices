#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include "DHT.h"
#include "Wire.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <UptimeString.h>  // Library To Get Uptime In Char Array's

#include <Uptime.h>  // Library To Calculate Uptime
Uptime uptime;
#define DHTPIN D3
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

#define WIFI_SSID "Lusoto"
#define WIFI_PASSWORD "tel26828581"

#define API_KEY "AIzaSyBkoWX7mOp-DJ1BYhz4zGUK-_2tb6yDRuU"

#define DATABASE_URL "https://home-deck-sumy-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

UptimeString uptimeString;


unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

int digitalPin = D0;  //pin pour le capteur
int analogPin = A0;   //pin pour le capteur
int digitalVal;
int analogVal;


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//Month names
String months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

uint8_t Uptime_Years = 0U, Uptime_Months = 0U, Uptime_Days = 0U, Uptime_Hours = 0U, Uptime_Minutes = 0U, Uptime_Seconds = 0U;



#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5           /* Time ESP32 will go to sleep (in seconds) */


void setup() {

  dht.begin();


  pinMode(digitalPin, INPUT);

  Serial.begin(9600);
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  timeClient.begin();
  timeClient.setTimeOffset(-10800);


}

void loop() {

  delay(5000);


  timeClient.update();
  uptime.calculateUptime();  // Calculate The Uptime In Library

  float h = dht.readHumidity();         //Reading the humidity
  float t = dht.readTemperature();      //Reading the temperature in Celsius degree
  float f = dht.readTemperature(true);  //Reading the temperature in Fahrenheit degrees

  int detected = digitalRead(D0);   // read Hall sensor
  int detectedAn = analogRead(A0);  // read flame analog value



  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  String ch = String(currentHour);
  String cm = String(currentMinute);
  String cs = String(currentSecond);

  String date = ch + ":" + cm + ":" + cs;

  Uptime_Days = uptime.getDays();
  Uptime_Hours = uptime.getHours();
  Uptime_Minutes = uptime.getMinutes();
  String uptime = String(Uptime_Days) + "d, " + String(Uptime_Hours) + " h, " + String(Uptime_Minutes) + "m";



  if (Firebase.ready() && signupOK) {

    Firebase.RTDB.setInt(&fbdo, "Sensors/0/TemperatureValue", t);

    Firebase.RTDB.setInt(&fbdo, "Sensors/0/HumidityValue", h);

    if (detected == LOW) {
      Firebase.RTDB.setInt(&fbdo, "Sensors/0/HallValue", 1);
    } else {
      Firebase.RTDB.setInt(&fbdo, "Sensors/0/HallValue", 0);
    }

    Firebase.RTDB.setString(&fbdo, "Sensors/0/uptime", uptime);
  }


 
}
