#include "Config.h" // All headers and shared variables are included here

// Define the objects
MAX30105 particleSensor;
Adafruit_MLX90614 therm; // Changed to Adafruit object

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
int hrState = 0;
int fillCount = 0;
int rollCount = 0;

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  
  // Give USB time to connect
  delay(10000); 
  Serial.println("\n--- Sensor Initialization ---");

  // 1. Start I2C at a slow, safe speed for the MLX90614
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 50000); 
  
  Serial.println("Initializing MLX90614...");
  // Using 0x5A (default) and the current Wire configuration
  if (!therm.begin(0x5A, &Wire)) {
    Serial.println("MLX90614 not found. Check wiring.");
    // Don't freeze the whole board, just note the error
  } else {
    Serial.println("MLX90614 OK.");
  }

  // 2. Now crank the speed up for the MAX30105
  // Most MAX30105 libraries will internally call setClock, 
  // but we'll set it manually to be sure.
  Wire.setClock(400000); 
  
  Serial.println("Initializing MAX30105...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found. Check wiring.");
    while (1) { delay(10); } // Freeze here if the HR sensor fails
  }
  Serial.println("MAX30105 OK.");

  // Configure MAX30105 settings
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

  Serial.println("Collecting initial samples...");
  collectSamples(redBuffer, irBuffer, BUFFER_SIZE);
  
  // Initial calculation
  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
                                         &spo2, &validSPO2,
                                         &heartRate, &validHeartRate);
  Serial.println("System Ready.");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  // 1. NON-BLOCKING TEMPERATURE TASK
  if (millis() - lastTempMs >= TEMP_INTERVAL_MS) {
    lastTempMs = millis();
    
    Wire.setClock(50000); // Slow down for MLX
    double obj = therm.readObjectTempC();
    double amb = therm.readAmbientTempC();
    Wire.setClock(400000); // Speed back up for MAX
    
    // Simple validation
    if (!isnan(obj)) objectTemp = (float)obj;
    if (!isnan(amb)) ambientTemp = (float)amb;
  }

  // 2. HEART RATE / SPO2 DATA PUMP
  particleSensor.check(); // Look for new data in the sensor FIFO

  if (!particleSensor.available()) {
    // If no new sample, check if we've lost the pulse for too long
    if (lastBeat > 0 && (millis() - lastBeat > 10000)) {
      beatAvg = 0;
    }
  } else {
    uint32_t irVal  = particleSensor.getIR();
    uint32_t redVal = particleSensor.getRed();
    particleSensor.nextSample(); // Advance the sensor pointer

    // --- State 0: No Finger ---
    if (irVal < FINGER_THRESHOLD) {
      if (hrState != 0) {
        Serial.println("No finger detected. HR/SpO2 paused.");
        beatAvg   = 0;
        spo2Avg   = 0;
        fillCount = 0;
        rollCount = 0;
        hrState   = 0;
      }
    } 
    else {
      // --- State 1: Finger detected, filling initial 100-sample buffer ---
      if (hrState == 0) {
        Serial.println("Finger detected. Filling buffer...");
        fillCount = 0;
        hrState   = 1;
      }

      if (hrState == 1) {
        redBuffer[fillCount] = redVal;
        irBuffer[fillCount]  = irVal;
        fillCount++;

        if (fillCount >= BUFFER_LENGTH) {
          maxim_heart_rate_and_oxygen_saturation(
            irBuffer, BUFFER_LENGTH, redBuffer,
            &spo2, &validSPO2, &heartRate, &validHeartRate);
          
          rollCount = 0;
          hrState   = 2;
          Serial.println("Buffer ready. Streaming...");
        }
      }
      // --- State 2: Buffer full, performing rolling updates ---
      else if (hrState == 2) {
        // Instant beat detection for the BPM display
        if (checkForBeat(irVal)) {
          long delta     = millis() - lastBeat;
          lastBeat       = millis();
          float instantBPM = 60.0f / (delta / 1000.0f);
          
          if (instantBPM > 20 && instantBPM < 200) {
            // Using your ema (Exponential Moving Average) helper
            beatAvg = (int)ema((float)beatAvg, instantBPM);
          }
        }

        // Add newest sample to the end of the buffer
        // (75 is BUFFER_LENGTH - BUFFER_SHIFT)
        redBuffer[75 + rollCount] = redVal;
        irBuffer[75 + rollCount]  = irVal;
        rollCount++;

        // Every 25 samples, shift the window and recalculate SpO2
        if (rollCount >= BUFFER_SHIFT) {
          for (int i = 0; i < 75; i++) {
            redBuffer[i] = redBuffer[i + 25];
            irBuffer[i]  = irBuffer[i + 25];
          }

          maxim_heart_rate_and_oxygen_saturation(
            irBuffer, BUFFER_LENGTH, redBuffer,
            &spo2, &validSPO2, &heartRate, &validHeartRate);

          if (validSPO2 && spo2 > 50 && spo2 <= 100) {
            if (spo2Avg == 0) spo2Avg = spo2; // Initial seed
            else spo2Avg = (int)ema((float)spo2Avg, (float)spo2);
          }

          rollCount = 0;  
        }
      }
    }
  }

  // 3. PERIODIC PRINTING TASK
  if (millis() - lastPrintMs >= PRINT_INTERVAL_MS) {
    lastPrintMs = millis();
    
    Serial.print("Temp (obj): ");
    Serial.print(objectTemp, 1);
    Serial.print(" C  |  Temp (amb): ");
    Serial.print(ambientTemp, 1);
    Serial.print(" C");

    if (hrState == 0) {
      Serial.println("  |  HR: -- (no finger)  |  SpO2: --");
    } else if (hrState == 1) {
      Serial.print("  |  HR: filling (");
      Serial.print(fillCount);
      Serial.println("/100)  |  SpO2: filling");
    } else {
      Serial.print("  |  HR: ");
      if (beatAvg > 20) Serial.print(String(beatAvg) + " bpm");
      else Serial.print("Calculating...");

      Serial.print("  |  SpO2: ");
      if (spo2Avg > 50) Serial.println(String(spo2Avg) + " %");
      else Serial.println("Calculating...");
    }
  }

  yield(); // Let ESP32 background tasks run
}