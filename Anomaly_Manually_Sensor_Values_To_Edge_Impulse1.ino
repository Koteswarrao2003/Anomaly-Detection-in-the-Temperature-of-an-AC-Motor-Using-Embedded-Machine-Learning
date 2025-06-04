#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Include your Edge Impulse model
#include <Project-5_inferencing.h>

// Sensor setup
Adafruit_INA219 ina219;
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Edge Impulse feature buffer
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

// Needed for inference
int ei_signal_from_buffer(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize I2C manually for ESP32-S3
  Wire.begin(21, 20);  // SDA, SCL

  // Initialize INA219
  if (!ina219.begin(&Wire)) {
    Serial.println("Failed to find INA219 chip");
    while (1) delay(10);
  }

  // Initialize temperature sensor
  sensors.begin();

  Serial.println("Edge Impulse Inference with Live Sensor Data");
}

void loop() {
  // Collect live sensor data
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float current_mA = ina219.getCurrent_mA();
  float power_mW = ina219.getPower_mW();

  // Feed values into feature array
  // Assuming your model expects 3 values per inference
  features[0] = temperatureC;
  features[1] = current_mA;
  features[2] = power_mW;

  // Wrap data in signal object for Edge Impulse
  signal_t signal;
  signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  signal.get_data = &ei_signal_from_buffer;

  // Run the classifier
  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
    Serial.print("Classifier error: ");
    Serial.println(res);
    return;
  }

  // Print inference results
  Serial.println("Prediction:");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print("  ");
    Serial.print(result.classification[ix].label);
    Serial.print(": ");
    Serial.println(result.classification[ix].value, 3);
  }

  Serial.println("-----------------------");
  delay(1000);  // Adjust to control prediction frequency
}
