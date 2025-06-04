#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// INA219 Power Sensor
Adafruit_INA219 ina219;

// DS18B20 Temperature Sensor on GPIO 5
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  // Initialize INA219
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) delay(10);
  }
  Serial.println("INA219 Initialized.");

  // Initialize DS18B20
  sensors.begin();
  Serial.println("DS18B20 Initialized.");
}

void loop() {
  // ---- Read Temperature ----
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);

  // ---- Read Power from INA219 ----
  float busVoltage = ina219.getBusVoltage_V();
  float shuntVoltage = ina219.getShuntVoltage_mV();
  float current_mA = ina219.getCurrent_mA();
  float power_mW = ina219.getPower_mW();

  temperatureC-=10;

  // ---- Display All Values ----
  Serial.println("-----------");
  Serial.print("Temperature: "); Serial.print(temperatureC); Serial.println(" °C");

  Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power: "); Serial.print(power_mW); Serial.println(" mW");

  delay(2000);  // Read every 2 seconds
}
