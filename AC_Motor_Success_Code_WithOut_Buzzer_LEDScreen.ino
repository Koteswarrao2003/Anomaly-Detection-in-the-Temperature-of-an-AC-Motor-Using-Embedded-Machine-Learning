#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Project-5_inferencing.h>  // Your AI model header

// INA219 (current & power)
Adafruit_INA219 ina219;

// DS18B20 (temperature)
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define TOTAL_FEATURES 150
#define FEATURES_PER_READING 3
#define NUM_READINGS (TOTAL_FEATURES / FEATURES_PER_READING)

// Buffer to store sensor data
float features[TOTAL_FEATURES];

// Function to supply data to Edge Impulse classifier
int ei_signal_from_buffer(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 20);   // Initialize I2C with SDA=21, SCL=20
  ina219.begin();
  sensors.begin();

  Serial.println("Edge Impulse inference from live sensor data...");
}

// Function to collect sensor data in a loop
void collect_sensor_data() {
  Serial.println("Collecting real-time sensor data...");
  int feature_index = 0;

  for (int i = 0; i < NUM_READINGS; i++) {
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);
    float current_mA = ina219.getCurrent_mA();
    float power_mW = ina219.getPower_mW();

    
    // temperature+=20;

    // Serial.print("Temperature: ");
    // Serial.print(temperature);
    // Serial.print(" °C, Current: ");
    // Serial.print(current_mA);
    // Serial.print(" mA, Power: ");
    // Serial.print(power_mW);
    // Serial.println(" mW");

    features[feature_index++] = temperature;
    features[feature_index++] = current_mA;
    features[feature_index++] = power_mW;


  }
}

void run_inference() {
  Serial.println("\nFinished data collection. Running inference...");

  signal_t signal;
  ei_impulse_result_t result;

  int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    Serial.println("Failed to create signal from buffer");
    return;
  }

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) return;

  Serial.println("Prediction results:");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print("  ");
    Serial.print(result.classification[ix].label);
    Serial.print(": ");
    Serial.println(result.classification[ix].value, 6);

    if(result.classification[ix].value>0.9){
      Serial.println(result.classification[ix].label);
    }
  }
}

void loop() {
  collect_sensor_data();
  run_inference();
  delay(1000);  // Delay before next cycle
}
