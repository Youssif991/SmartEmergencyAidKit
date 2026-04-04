#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"   
#include "addons/RTDBHelper.h"    

#define WIFI_SSID     "------"      //esm el shaabaka
#define WIFI_PASSWORD "------"      //password

#define API_KEY       "AIzaSyC-ad77Wyw4_9FCQIbQwq53y-BRY02oe_o"
#define DATABASE_URL  "https://health-71a04-default-rtdb.europe-west1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;  // Track if signup succeeded

void setup() {
  Serial.begin(115200);

  // --- WiFi ---
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  // --- Firebase config ---
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Token status callback (required by the library)
  config.token_status_callback = tokenStatusCallback;

  // Anonymous sign-up — only do this ONCE, then use Firebase.begin() directly
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase signup failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);  
}

void loop() {
  // Only push data when Firebase is ready
  if (Firebase.ready() && signupOK) {

    int   hr   = 100;   // Replace with real sensor reads
    float temp = 37.5;
    int   spo2 = 95;

    if (Firebase.RTDB.setInt(&fbdo, "/health/hr", hr)) {
      Serial.println("HR sent: " + String(hr));
    } else {
      Serial.println("HR failed: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/health/temp", temp)) {
      Serial.println("Temp sent: " + String(temp));
    } else {
      Serial.println("Temp failed: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "/health/spo2", spo2)) {
      Serial.println("SpO2 sent: " + String(spo2));
    } else {
      Serial.println("SpO2 failed: " + fbdo.errorReason());
    }
  }

  delay(2000);
}
