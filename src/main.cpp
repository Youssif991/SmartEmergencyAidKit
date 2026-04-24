#include "config.h" // Central configuration for all includes and shared variables

FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig config;
bool           signupOK = false;

PulseOximeter  pox;
MLX90614       mlx;

uint32_t tsLastPush = 0;
uint32_t tsLastTemp = 0;

float    tempC = 0.0;
float    hr    = 0.0;
uint8_t  spo2  = 0;

// ─────────────────────────────────────────────────────────────────────────────
void onBeatDetected() {
  Serial.println("Beat!");
}

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // ── WiFi ──────────────────────────────────────────────────────────────────
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  // ── Firebase ──────────────────────────────────────────────────────────────
  config.api_key               = API_KEY;
  config.database_url          = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase signup failed: %s\n",
                  config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // ── Sensors ───────────────────────────────────────────────────────────────
  Wire.begin();

  if (!pox.begin()) {
    Serial.println("ERROR: MAX30100 not found. Check wiring.");
    while (true);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
  Serial.println("MAX30100 ready.");

  if (!mlx.begin()) {
    Serial.println("ERROR: MLX90614 not found. Check wiring.");
    while (true);
  }
  Serial.println("MLX90614 ready.");
  Serial.println("--------------------------------------------------");
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  pox.update();   // Must run every iteration — no delays

  uint32_t now = millis();

  // ── Temperature ───────────────────────────────────────────────────────────
  if (now - tsLastTemp >= TEMP_READ_MS) {
    tsLastTemp = now;
    tempC = (float)mlx.readTemp(MLX90614::MLX90614_SRC01, MLX90614::MLX90614_TC);
  }

  // ── HR & SpO2 ─────────────────────────────────────────────────────────────
  hr   = pox.getHeartRate();
  spo2 = pox.getSpO2();

  // ── Firebase push ─────────────────────────────────────────────────────────
  if (Firebase.ready() && signupOK && (now - tsLastPush >= FIREBASE_PUSH_MS)) {
    tsLastPush = now;

    if (Firebase.RTDB.setFloat(&fbdo, "/health/hr",   hr))
      Serial.println("HR sent: "     + String(hr, 1));
    else
      Serial.println("HR failed: "   + fbdo.errorReason());

    if (Firebase.RTDB.setInt(&fbdo,   "/health/spo2", spo2))
      Serial.println("SpO2 sent: "   + String(spo2));
    else
      Serial.println("SpO2 failed: " + fbdo.errorReason());

    if (Firebase.RTDB.setFloat(&fbdo, "/health/temp", tempC))
      Serial.println("Temp sent: "   + String(tempC, 1) + "C");
    else
      Serial.println("Temp failed: " + fbdo.errorReason());

    Serial.println("--------------------------------------------------");
  }
}