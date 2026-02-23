#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255

uint32_t irBuffer[100];
uint32_t redBuffer[100];

int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

long lastBeat = 0;

float beatsPerMinute;
int beatAvg = 0, sp02Avg = 0; 

void setup()
{ 
  Serial.begin(115200);
  
  Serial.println("Initializing Pulse Oximeter..");
  
  Wire.begin(21, 22); //SDA = 21, SCL = 22

  
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  
  byte ledBrightness = 60;
  byte sampleAverage = 1;
  byte ledMode = 2;
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop()
{
  bufferLength = 100;
  
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false)
      particleSensor.check();
  
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }
  

  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  
  
  while (1)
  {
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
  
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false)
        particleSensor.check();
    
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();

      long irValue = irBuffer[i];

       
      if (checkForBeat(irValue) == true)
      {
        
        long delta = millis() - lastBeat;
        lastBeat = millis();
      
        beatsPerMinute = 60 / (delta / 1000.0);
        beatAvg = (beatAvg+beatsPerMinute)/2;
      }
      if(millis() - lastBeat > 10000)
      {
        beatsPerMinute = 0;
        beatAvg = (beatAvg+beatsPerMinute)/2;
      }
    }
  
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    if(validSPO2 == 1 && spo2 < 100 && spo2 > 0)
    {
      sp02Avg = (sp02Avg+spo2)/2;
    }
    else
    {
      spo2 = 0;
      sp02Avg = (sp02Avg+spo2)/2;
    }

    if (irBuffer[99] < 50000) {
      Serial.println("No finger detected. Waiting...");
      beatAvg = 0;
      sp02Avg = 0;
    } 
    else 
    {
      Serial.print("Heart Rate: ");
      if (validHeartRate && beatAvg > 20 && beatAvg < 200) { 
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
  }
}
