#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>
#include <string>
#include <WiFiManager.h> 
#include <Adafruit_Sensor.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//firebase
#define API_KEY "AIzaSyCQWpA1eEoeGtxL6W7jSlYW-5W0BjjlUBk"
#define USER_EMAIL "ionrail2@gmail.com"
#define USER_PASSWORD "ionrail123"
#define DATABASE_URL "https://ionrail-2-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;
bool signupOK = false;

//sensor and servo path
String frontirpath = "/front";
String rearirpath = "/rear";
String timePath = "/timestamp";

//time
String fullhour;
String fullminute;
String fulltime;

//db main path
String databasePath,readingpath;
String readparent;

//endpoint handler
FirebaseJson json;
WiFiClient client;
String uid;

//update data senosr 
String frontir;
String rearir;

//sensor hold data
int IR1 = 13; // D7 choose pin for the LED
int IR2 = 12; // D6 choose input pin (for Infrared sensor) 
int val1 = 0; // variable for reading the pin status
int val2 = 0; // variable for reading the pin status

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
int timestamp;

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}


void setup() 
{  Serial.begin(9600);
   pinMode(IR1, INPUT); // declare IR1 as output 
   pinMode(IR2, INPUT); // declare Infrared sensor as input
   sensor_t sensor;

   WiFiManager wifiManager;
   wifiManager.autoConnect("IonRail Obstacle Detection");
   timeClient.begin();

   //db
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);
    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;
    Firebase.begin(&config, &auth);

    //display db info
    Serial.println("Getting User UID");
    while ((auth.token.uid) == "") {
      Serial.print('.');
      delay(1000);
    }
  // Print user UID
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.println(uid);

     // Update database path
     readingpath = "/Obstaclecheck";

}


void loop() {

  timeClient.update();

  int currentHour = timeClient.getHours();
//  Serial.print("Hour: ");
//  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
//  Serial.print("Minutes: ");
//  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
//  Serial.print("Seconds: ");
//  Serial.println(currentSecond); 



  if(currentHour < 10)
  {
    fullhour = '0' + String(currentHour);
  }
  else
  {
    fullhour = String(currentHour);
  }

  if(currentMinute < 10)
  {
    fullminute = '0' + String(currentMinute);
  }
  else
  {
    fullminute = String(currentMinute);
  }

  fulltime = fullhour + ":" + fullminute;

  
  Serial.println(fulltime);


   val1 = digitalRead(IR1); // read input value 
   val2 = digitalRead(IR2); // read input value

   if (val1 == LOW)
   { // check if the input is HIGH
      frontir = "Caution Ahead !!";
      Serial.println("Front IR Sensor Value with out Object in front of the Sensor");
   }

   else if (val1 == HIGH)
   { // check if the input is HIGH
      frontir = "";
   }

   if (val2 == LOW)
   { // check if the input is HIGH
      rearir = "Caution Rear !!";
      Serial.println("Rear IR Sensor Value with out Object in front of the Sensor");
   } 

   else if (val2 == HIGH)
   { // check if the input is HIGH
      rearir = "";
   }

   if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0))
    {
      sendDataPrevMillis = millis();

      //current timestamp
      timestamp = getTime();
      Serial.print("time: ");
      Serial.println(timestamp);

      readparent = readingpath + "/" + String(timestamp);

      sensors_event_t event;
      json.set(frontirpath.c_str(), frontir);
      json.set(rearirpath.c_str(), rearir);
      json.set(timePath, String(timestamp));
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, readparent.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  
    }

    else 
          {
            Serial.println(fbdo.errorReason());
          }

    delay(2000);
   
}
