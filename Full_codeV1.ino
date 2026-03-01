#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include <DHT.h>
#include "BluetoothSerial.h"
#include <math.h>

MAX30105 particleSensor;
BluetoothSerial SerialBT;

#define DHTPIN 16
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define MAX_BRIGHTNESS 255

uint32_t irBuffer[100];
uint32_t redBuffer[100];

int32_t bufferLength = 100;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

long lastBeat = 0;
float beatsPerMinute;
int beatAvg = 0, sp02Avg = 0; 
bool initialSetupComplete = false;

float temperature = 0.0;
unsigned long lastDHTRead = 0;
unsigned long lastsend = 0;
void sendBT_func(int hr, int temp_i , int temp_d, int spo2);

void setup() { 
  Serial.begin(115200);
  Serial.println("Initializing Health Monitor..");
  
  SerialBT.begin("ESP32_Health"); 
  dht.begin();
  
  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1) { yield(); }
  }

  byte ledBrightness = 60;
  byte sampleAverage = 1;
  byte ledMode = 2;
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop() {
  
  if (!initialSetupComplete) {
    for (byte i = 0 ; i < bufferLength ; i++) {
      while (particleSensor.available() == false)
        particleSensor.check();
  
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
    }
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    initialSetupComplete = true;
  }

  for (byte i = 25; i < 100; i++) {
    redBuffer[i - 25] = redBuffer[i];
    irBuffer[i - 25] = irBuffer[i];
  }

  for (byte i = 75; i < 100; i++) {
    while (particleSensor.available() == false)
      particleSensor.check();
  
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();

    long irValue = irBuffer[i];
    
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
    
      beatsPerMinute = 60 / (delta / 1000.0);
      
      if (beatsPerMinute > 20 && beatsPerMinute < 200) {
          if (beatAvg == 0) beatAvg = beatsPerMinute;
          else beatAvg = (beatAvg + beatsPerMinute) / 2;
      }
    }
  }
  
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  if(validSPO2 == 1 && spo2 < 100 && spo2 > 50) {
    if (sp02Avg == 0) sp02Avg = spo2;
    else sp02Avg = (sp02Avg + spo2) / 2;
  }

  if(millis() - lastBeat > 10000) {
    beatAvg = 0;
  }

  if (millis() - lastDHTRead >= 2000) {
    float tempRead = dht.readTemperature();
    if (!isnan(tempRead)) {
      temperature = tempRead;
    }
    lastDHTRead = millis();
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C  |  ");

  if (irBuffer[99] < 50000) {
    Serial.println("No finger detected. Waiting...");
    beatAvg = 0;
    sp02Avg = 0;
  } else {
    Serial.print("Heart Rate: ");
    if (beatAvg > 20 && beatAvg < 200) { 
      Serial.print(beatAvg, DEC);
      Serial.print(" bpm");
    } else {
      Serial.print("Calculating...");
    }

    Serial.print("  |  SpO2: ");
    if (validSPO2 && sp02Avg > 50 && sp02Avg <= 100) {
      Serial.print(sp02Avg, DEC);
      Serial.println(" %");
    } else {
      Serial.println("Calculating...");
    }
  }
if (millis() - lastsend >= 2000){
  int temp_int = (int)temperature;
  int temp_dec = (int)(fabs(temperature - temp_int) * 100); 
  
  sendBT_func(constrain(beatAvg, 0, 255), constrain(temp_int, 0, 255), temp_dec, constrain(sp02Avg, 0, 100));
  lastsend=lastsend+2000;
}
  yield(); 
}

void sendBT_func(int hr, int temp_i , int temp_d, int spo2) {
  SerialBT.write(255);
  SerialBT.write(hr);
  SerialBT.write(temp_i);
  SerialBT.write(temp_d);
  SerialBT.write(spo2);
  SerialBT.write(255);
  
  Serial.println("packet sended");
}
