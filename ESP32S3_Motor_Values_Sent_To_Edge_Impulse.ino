#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Create sensor objects
Adafruit_INA219 ina219;
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  // Manually specify I2C pins for ESP32-S3
  Wire.begin(21, 20);  // SDA, SCL

  // Initialize INA219 with custom Wire instance
  if (!ina219.begin(&Wire)) {
    Serial.println("Failed to find INA219 chip");
    while (1) delay(10);
  }

  sensors.begin();
}

void loop() {
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);

  float current_mA = ina219.getCurrent_mA();
  float power_mW = ina219.getPower_mW();

  // temperatureC+=20;

  // Print only raw numbers as CSV
  Serial.print(temperatureC); Serial.print(",");
  Serial.print(current_mA);   Serial.print(",");
  Serial.println(power_mW);

  delay(500);  // ~2 samples/sec
}
