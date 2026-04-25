#include "Config.h" // All headers and shared variables are included here

// Define the objects
MAX30105 particleSensor;
IRTherm therm;

// Define the temperature variables
float    objectTemp;
float    ambientTemp;
uint32_t lastTempMs = 0;
uint32_t lastPrintMs = 0;

// Define the buffer arrays
uint32_t irBuffer[BUFFER_LENGTH];
uint32_t redBuffer[BUFFER_LENGTH];

// Define the SpO2/HR variables
int32_t  spo2;
int8_t   validSPO2;
int32_t  heartRate;
int8_t   validHeartRate;

// Define the beat detection variables
float    beatsPerMinute;
int      beatAvg;
int      spo2Avg;
byte     rates[RATE_SIZE];
byte     rateSpot = 0;
long     lastBeat = 0;

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("MAX30105 (SpO2/HR) + MLX90614 (Temp)");

  Wire.begin();
  
  // Initialize MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found. Check wiring.");
    while (true);
  }

  // Initialize MLX90614
  if (therm.begin() == false) {
    Serial.println("MLX90614 not found. Check wiring.");
    while(true);
  }
  therm.setUnit(TEMP_C); // Set to Celsius for our baseline reading

  // Configure MAX30105: powerLevel, sampleAverage, ledMode (2=Red+IR), sampleRate, pulseWidth
  particleSensor.setup(
    MAX30105_SETUP_POWER, 
    MAX30105_AVERAGE, 
    MAX30105_LED_MODE, 
    MAX30105_RATE_HZ, 
    MAX30105_PULSE_US, 
    MAX30105_ADC_RANGE
  );
  
  particleSensor.setPulseAmplitudeRed(MAX30105_LED_AMP);
  particleSensor.setPulseAmplitudeIR(MAX30105_LED_AMP);

  // Fill buffer with an initial 4-second window before starting the loop
  Serial.println("Collecting initial samples...");
  collectSamples(redBuffer, irBuffer, BUFFER_SIZE);
  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
                                         &spo2, &validSPO2,
                                         &heartRate, &validHeartRate);
  Serial.println("Ready.");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  // Add one new sample at the end 
  for (int i = 0; i < BUFFER_SIZE - 1; i++) {
    redBuffer[i] = redBuffer[i + 1];
    irBuffer[i]  = irBuffer[i + 1];
  }

  while (particleSensor.available() == 0)
    particleSensor.check();

  redBuffer[BUFFER_SIZE - 1] = particleSensor.getRed();
  irBuffer[BUFFER_SIZE - 1]  = particleSensor.getIR();
  particleSensor.nextSample();

  // Heart Beat Detection
  long irValue = irBuffer[BUFFER_SIZE - 1];

  if (irValue < 50000) {
    Serial.println("No finger detected.");
  } else {
    if (checkForBeat(irValue)) {
      long delta  = millis() - lastBeat;
      lastBeat    = millis();
      beatsPerMinute = 60.0 / (delta / 1000.0);

      if (beatsPerMinute > 20 && beatsPerMinute < 255) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

    // Recalculate SpO2 each loop iteration
    maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
                                           &spo2, &validSPO2,
                                           &heartRate, &validHeartRate);

    // Print HR and SpO2
    Serial.print("BPM=");        Serial.print(beatsPerMinute, 1);
    Serial.print("  Avg BPM=");  Serial.print(beatAvg);

    Serial.print("  |  SpO2=");
    if (validSPO2)  Serial.print(spo2); else Serial.print("--");
    Serial.print("%");

    Serial.print("  HR(algo)=");
    if (validHeartRate) Serial.print(heartRate); else Serial.print("--");
    Serial.print(" bpm");

    // Read and print Temperature from MLX90614
    if (therm.read()) {
      float tempC = therm.object(); 
      float tempF = (tempC * 9.0 / 5.0) + 32.0; // Calculate Fahrenheit locally

      Serial.print("  |  Obj Temp=");
      Serial.print(tempC, 1); Serial.print("C / ");
      Serial.print(tempF, 1); Serial.println("F");
    } else {
      Serial.println("  |  Temp=Error");
    }
  }
}