#include <Wire.h>
#include <Project-6_inferencing.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int Buzzer_pin=18;

/* INA219 current sensor setup */
Adafruit_INA219 ina219;

/* OneWire bus for temperature sensor */
const int oneWireBus = 15;     
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

/* Constants */
#define FREQUENCY_HZ        2
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

/* Buffer for features (input to the model) */
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;

/* Timing */
static unsigned long last_interval_ms = 0;

/* Setup function */
void setup() {
  Serial.begin(115200);

  /* Initialize Dallas temperature sensor */
  sensors.begin();
  lcd.init();
  lcd.backlight();
  pinMode(Buzzer_pin,OUTPUT);

  /* Initialize INA219 current sensor */
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { 
      delay(10); 
    }
  }
  
  Serial.println("Measuring voltage, current, and temperature...");

  Serial.println("");
  delay(100);

  /* Output model info */
  Serial.print("Features: ");
  Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  Serial.print("Label count: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);
}

/* Loop function */
void loop() {
  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();

    /* Request temperature from Dallas temperature sensor */
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);

    /* Read power from INA219 sensor */
    float power_mW = ina219.getPower_mW();

    /* Store temperature and power in features array */
    features[feature_ix++] = temperatureC;
    features[feature_ix++] = power_mW;

    /* Check if we have enough data to run inference */
    if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      Serial.println("Running the inference...");

      //Create signal from buffer 
      signal_t signal;
      ei_impulse_result_t result;
      int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
      if (err != 0) {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
      }

      //Run the classifier
      EI_IMPULSE_ERROR res = run_classifier(&signal, &result, true);
      if (res != 0) return;

      // Print predictions
      ei_printf("Predictions ");
      ei_printf("(DSP: %d ms., Classification: %d ms.)",
                result.timing.dsp, result.timing.classification);
      ei_printf(": \n");

      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        if(result.classification[ix].value>0.8)
        {
            lcd.clear();
            lcd.setCursor(2,0);
            lcd.print(result.classification[ix].label);
      }
      }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf("    anomaly score: %.3f\r\n", result.anomaly);
#endif

if(result.anomaly>0.8)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Anomaly Detected");
}


if((result.classification[2].value>0.8)||(result.anomaly>0.8))
{
  digitalWrite(Buzzer_pin,HIGH);
  delay(1000);
}
else 
{
  digitalWrite(Buzzer_pin,LOW);
}
      // Reset feature index
      feature_ix = 0;
    }
  }
}

/* Custom printf function */
void ei_printf(const char *format, ...) {
  static char print_buf[1024] = { 0 };

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0) {
    Serial.write(print_buf);
  }
}
