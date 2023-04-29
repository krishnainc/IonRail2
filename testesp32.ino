#include <Arduino.h>
#include <ESP32Servo.h>
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
String usonicpath = "/usonic";
String servopath = "/servodoor";
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

//servo
#define TURN_TIME 175
#define servoPin 18
Servo servo;

//usonic 
 int trigPin = 5;
 int echoPin = 17;
 long duration, dist, average;   
 long aver[3]; 
 String gatestatus;

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
{
    Serial.begin(9600);
    servo.attach(servoPin);  
    pinMode(trigPin, OUTPUT);  
    pinMode(echoPin, INPUT);  
    servo.write(0);         //close cap on power on
    delay(100);
    servo.detach(); 

    sensor_t sensor;

    WiFiManager wifiManager;
    wifiManager.autoConnect("IonRail2");
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
     readingpath = "/Traindata";

}

void loop() 
{
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

  for (int i=0;i<=2;i++) 
  {   
    measure();               
    aver[i]=dist;            
    delay(10);             
  }

  dist=(aver[0]+aver[1]+aver[2])/3;    

  if ( dist<7 ) 
  {
  //Change distance as per your need
    gatestatus = "Open";
    servo.attach(servoPin);
    delay(1);
    servo.write(0);  
    delay(170);       
    servo.write(80);     //SERVO MOTOR ROTATING DEGREE
    servo.detach(); 
    delay(2000);

    servo.attach(servoPin);
    delay(1);
    servo.write(180);  
    delay(170);       
    servo.write(80);
    servo.detach(); 
    delay(2000);

  }

  else
  {
    gatestatus = "Close";
  }

  //db update

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0))
    {
      sendDataPrevMillis = millis();

      //current timestamp
      timestamp = getTime();
      Serial.print("time: ");
      Serial.println(timestamp);

      readparent = readingpath + "/" + String(timestamp);

      sensors_event_t event;
      json.set(usonicpath.c_str(), dist);
      json.set(servopath.c_str(), gatestatus);
      json.set(timePath, String(timestamp));
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, readparent.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  
      Serial.println(dist);
      Serial.println(gatestatus);
    }

    else 
          {
            Serial.println(fbdo.errorReason());
          }

    delay(2000);

}
  

void measure() 
{  
  digitalWrite(10,HIGH);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(15);
  digitalWrite(trigPin, LOW);
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  dist = (duration/2) / 29.1;   
}


