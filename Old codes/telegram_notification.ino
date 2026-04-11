
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_SSID     "esm"       //matensa4 el wifi
#define WIFI_PASSWORD "password"

String botToken = "8685668389:AAFU7-nuRck-qb4Gr5FAcVvDwq13-dUBx7w";

#define DATABASE_URL "https://health-71a04-default-rtdb.europe-west1.firebasedatabase.app"
#define API_KEY "AIzaSyC-ad77Wyw4_9FCQIbQwq53y-BRY02oe_o"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int chatCount = 4;
String chatIDs[chatCount] = {"5325723751", "1131440350", "1600412047", "1431425153"};

bool telegramSent = false;

void setup() {
  Serial.begin(115200);
  Serial.println();


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  
  }

  Serial.println("\nWiFi Connected!");
}

void loop() {

  int hr   = 80;
  float temp = 36;
  int spo2 = 90;

  sendToFirebase(hr, temp, spo2);
  bool isCritical = heartRate > 120 || heartRate < 50 || temperature > 38.0 || spo2 < 90 || temperature < 35.0;
  bool isWarning = (heartRate > 100 && heartRate <= 120) ||(heartRate >= 50 && heartRate < 60) ||(temperature >= 37.5 && temperature <= 38.0) || 
  (temperature >= 35.0 && temperature < 36.0)||(spo2 >= 90 && spo2 < 95);
  if (isCritical) {
 String alertMsg =
        "🚨 EMERGENCY ALERT 🚨\nhelp the patient!\ncall 123!";
  sendTelegram(alertMsg);
  telegramSent = true;    
  }
  else if(isWarning){
 String alertMsg =
        ""⚠️ Warning ⚠️\n Vitals slightly abnormal!";
  sendTelegram(alertMsg);
    telegramSent = true; 
  }
  else {
    telegramSent = false;
  }

}

void sendToFirebase(int hr, float temp, int spo2) {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  http.begin(String(DATABASE_URL) + "/health/hr.json");
  http.PUT(String(hr));
  http.end();

  http.begin(String(DATABASE_URL) + "/health/temp.json");
  http.PUT(String(temp));
  http.end();

  http.begin(String(DATABASE_URL) + "/health/spo2.json");
  http.PUT(String(spo2));
  http.end();
 
}

void sendTelegram(String message) {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  message.replace(" ", "%20");
  message.replace("\n", "%0A");

 
  for (int i = 0; i < chatCount; i++) {

    String url =
      "https://api.telegram.org/bot" + botToken +
      "/sendMessage?chat_id=" + chatIDs[i] +
      "&text=" + message;

    http.begin(url);
    int httpCode = http.GET();
    http.end();
  } 
}
