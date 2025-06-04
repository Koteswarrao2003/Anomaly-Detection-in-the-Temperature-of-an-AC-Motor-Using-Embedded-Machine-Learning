#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Project-5_inferencing.h>  // Edge Impulse model

// INA219 (current & power)
Adafruit_INA219 ina219;

// DS18B20 temperature sensor
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LCD: I2C 16x2 (address may vary: 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // If not working, try 0x3F

// Buzzer
#define BUZZER_PIN 2

// Feature buffer for Edge Impulse
#define TOTAL_FEATURES 150
#define FEATURES_PER_READING 3
#define NUM_READINGS (TOTAL_FEATURES / FEATURES_PER_READING)
float features[TOTAL_FEATURES];

// Signal input for Edge Impulse
int ei_signal_from_buffer(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 20);  // SDA = 21, SCL = 20 for ESP32
  ina219.begin();
  sensors.begin();

  lcd.begin(16,2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AI Inference Init");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("System initialized.");
}

void collect_sensor_data() {
  Serial.println("Collecting sensor data...");
  int feature_index = 0;

  for (int i = 0; i < NUM_READINGS; i++) {
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);
    float current_mA = ina219.getCurrent_mA();
    float power_mW = ina219.getPower_mW();

    features[feature_index++] = temperature;
    features[feature_index++] = current_mA;
    features[feature_index++] = power_mW;

  }
}

void run_inference() {
  Serial.println("Running inference...");
  signal_t signal;
  ei_impulse_result_t result;

  int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    Serial.println("Signal buffer failed");
    return;
  }

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) return;

  Serial.println("Prediction Results:");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print("  ");
    Serial.print(result.classification[ix].label);
    Serial.print(": ");
    Serial.println(result.classification[ix].value, 6);

    if (result.classification[ix].value > 0.5) {
      // Print to Serial
      Serial.println(">> Detected:");
      Serial.println(result.classification[ix].label);

      // Show on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Detected:");
      lcd.setCursor(0, 1);
      lcd.print(result.classification[ix].label);

      if (strcmp(result.classification[ix].label, "Known Anomaly Detection") == 0) {
        tone(BUZZER_PIN, 1000);
        delay(500);
        noTone(BUZZER_PIN);
      } 
    }
  }
}

void loop() {
  collect_sensor_data();
  run_inference();
  delay(1000);
}
