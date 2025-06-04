#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <Project-5_inferencing.h>

// Edge Impulse feature buffer
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {
  59.0999, 53.1358, 646.4360, 59.0957, 52.9608, 643.6419, 59.0915, 52.7787, 640.7513, 59.0875, 52.5897, 637.7687, 59.0836, 52.3940, 634.6987, 
  59.0798, 52.1917, 631.5463, 59.0761, 51.9831, 628.3167, 59.0726, 51.7684, 625.0155, 59.0692, 51.5478, 621.6483, 59.0660, 51.3215, 618.2211, 
  59.0629, 51.0899, 614.7399, 59.0600, 50.8531, 611.2111, 59.0573, 50.6115, 607.6409, 59.0548, 50.3654, 604.0363, 59.0524, 50.1151, 600.4036, 
  59.0503, 49.8608, 596.7496, 59.0483, 49.6030, 593.0814, 59.0465, 49.3420, 589.4057, 59.0449, 49.0780, 585.7295, 59.0435, 48.8115, 582.0597, 
  59.0424, 48.5428, 578.4032, 59.0414, 48.2723, 574.7668, 59.0406, 48.0004, 571.1575, 59.0401, 47.7273, 567.5817, 59.0398, 47.4535, 564.0463, 
  59.0397, 47.1794, 560.5576, 59.0397, 46.9053, 557.1219, 59.0401, 46.6316, 553.7453, 59.0406, 46.3587, 550.4339, 59.0413, 46.0870, 547.1932, 
  59.0422, 45.8167, 544.0288, 59.0434, 45.5484, 540.9458, 59.0448, 45.2823, 537.9492, 59.0463, 45.0187, 535.0436, 59.0481, 44.7581, 532.2333, 
  59.0500, 44.5007, 529.5224, 59.0522, 44.2470, 526.9144, 59.0546, 43.9971, 524.4125, 59.0571, 43.7515, 522.0198, 59.0598, 43.5104, 519.7388, 
  59.0627, 43.2740, 517.5715, 59.0658, 43.0428, 515.5198, 59.0690, 42.8169, 513.5851, 59.0724, 42.5965, 511.7681, 59.0760, 42.3820, 510.0694, 
  59.0796, 42.1735, 508.4892, 59.0835, 41.9712, 507.0270, 59.0874, 41.7753, 505.6823, 59.0914, 41.5860, 504.4538, 59.0956, 41.4035, 503.3399


};

int ei_signal_from_buffer(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Running Edge Impulse inference with manual input data...");

  signal_t signal;
  signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  signal.get_data = &ei_signal_from_buffer;

  ei_impulse_result_t result = {0};
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
    Serial.print("Classifier error: ");
    Serial.println(res);
    while(1);
  }

  Serial.println("Prediction results:");
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    Serial.print("  ");
    Serial.print(result.classification[i].label);
    Serial.print(": ");
    Serial.println(result.classification[i].value, 6);
  }
}

void loop() {
  // nothing here, run inference once in setup
}
