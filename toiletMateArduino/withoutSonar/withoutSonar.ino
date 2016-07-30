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

bool currentMotionState = false;
bool isTimerOn = false;
bool stillHasMotion = false;
int retryTime = 30;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
//  wifiConnect();
  autoWifiConnect();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void autoWifiConnect() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("ToiletMate-F6-12");
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
  if (!isTimerOn) {
    if (newMotionState != currentMotionState) {
      if (newMotionState) {
        Serial.println("Motion detected");
        updateState(newMotionState);
      } else {
        Serial.print("No motion, enable Timer... ");
        isTimerOn = true;
      }
    }
  } else {
    if (newMotionState) {
      Serial.println("\nStill have motion, stop the Timer");
      resetTimer();
    } else {
      Serial.print(retryTime);
      Serial.print(" ");
      retryTime--;
      if (retryTime <= 0) {
        Serial.println("\nThe toilet is not occupied, stop the Timer");
        resetTimer();
        updateState(false);
      }
    }
  }
  delay(1000);
}

void resetTimer() {
    isTimerOn = false;
    retryTime = 30;
}

void updateState(bool inputState) {
  currentMotionState = inputState;
  Firebase.setBool("devices/" + WiFi.macAddress() + "/occupied", inputState);
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
  }
}
