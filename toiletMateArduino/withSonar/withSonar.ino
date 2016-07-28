#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FirebaseArduino.h>

//AP definitions
//#define AP_SSID "MMNet"
//#define AP_PASSWORD "MMValuableNET"
#define AP_SSID "TL"
#define AP_PASSWORD "qwerty321"

//Firebase definitions
#define FIREBASE_HOST "mentortoilet.firebaseio.com"
#define FIREBASE_AUTH "KT0fMWTFfSWK7pmJNuF82gU7w287OA9W4iH4Axss"

// GPIOs
#define PIR_PIN 2
#define SONAR_PIN 0

#define MINIMUM_DISTANCE_CM 60

bool currentMotionState = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(SONAR_PIN,OUTPUT);
  wifiConnect();
//  autoWifiConnect();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void autoWifiConnect() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("MentorToilet-F6-12");
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void wifiConnect() {
  Serial.print("\nConnecting to AP");
  WiFi.begin(AP_SSID, AP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Motion Detecting
  int newMotionState = digitalRead(PIR_PIN);
  if (newMotionState != currentMotionState) {
    if (newMotionState) {
      Serial.println("Motion detected");
      updateState(newMotionState);
    } else {
      Serial.print("No motion, start Pinging... ");
      // Pinging
      if (noObstacle()) {
        Serial.println("No obstacle found");
        updateState(newMotionState);
      } else {
        Serial.println("Obstacle found");
      }
    }
  }
}

void updateState(bool inputState) {
  currentMotionState = inputState;
  Firebase.setBool("devices/" + WiFi.macAddress() + "/occupied", inputState);
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
  }
}

bool noObstacle() {
  long distanceCm = getDistanceCm();
  Serial.println();
  Serial.print("Current distance in Centimeters: ");
  Serial.println(distanceCm);
  return 0 >= distanceCm || distanceCm >= MINIMUM_DISTANCE_CM;
}

unsigned long getDistanceCm() {
  long minimumDistanceCm = 5000;
  for (int i = 0; i < 3; i++) {
    Serial.print(i);
    Serial.print(" ");
    long currentDistanceCm = microsecondsToCentimeters(ping());
    if (currentDistanceCm < minimumDistanceCm) {
      minimumDistanceCm = currentDistanceCm;
    }
    delay(250);
  }
  return minimumDistanceCm;
}

unsigned long ping() { 
  pinMode(SONAR_PIN, OUTPUT);
  digitalWrite(SONAR_PIN, LOW);
  delayMicroseconds(2); 
  digitalWrite(SONAR_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(SONAR_PIN, LOW);
  pinMode(SONAR_PIN, INPUT);
  digitalWrite(SONAR_PIN, HIGH);
  long duration = pulseIn(SONAR_PIN, HIGH);
  return duration;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
