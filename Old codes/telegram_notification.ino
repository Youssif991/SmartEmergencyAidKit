#include <WiFi.h>
#include <HTTPClient.h>

String botToken = "8685668389:AAFU7-nuRck-qb4Gr5FAcVvDwq13-dUBx7w";
String chatID = "5325723751";


bool sent = false; 
void setup() {
//نفس بتاع ال for_APP.ino
}

void loop() {
  
bool isCritical = (heartRate > 120 || heartRate < 50 || temperature > 38.0 || spo2 < 90);  // mtnsa4 asmaa el variables te3adelha

  if (isCritical && !sent) {
    String mess = "🚨 The Status is CRITICAL! \n call 123 ! ";  //lw el emoji hy3ml error sheloh
    sendTelegram(mess);
    messageSent = true; 
  } 
  else if (!isCritical) {
    sent = false; 
  }

}

void sendTelegram(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      Serial.println("sent ");
    } else {
      Serial.println("Error ");
    }
    http.end();
  }
}