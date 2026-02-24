#include <DHT.h>

#define DHTPIN 16
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(1000);
}

void loop() {
  float temperature = dht.readTemperature();
  
  if (!isnan(temperature)) {
    Serial.println(temperature);
  } else {
    Serial.println("Error");
  }
  
  delay(2000);
}