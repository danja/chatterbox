#include <Arduino.h>
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 

#include <driver/adc.h> // depends on Espressif ESP32 libs

I2sDAC dac;
TaskHandle_t AudioTask;

const int pitchIn = 27; // control pot

int pitchRaw = 0;

// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void setup()
{
  Serial.begin(115200);
  Serial.println("DAC TEST SETUP");

  // begin(int sampleRate = 44100, int dataPort = 0, int bclk = 26, int wsel = 25, int dout = 33);
  dac.begin(44100, 0, 26, 25, 33);

  // Serial.println("portTICK_RATE_MS = " + portTICK_RATE_MS);

  // High priority audio thread
  xTaskCreatePinnedToCore(
    Output,
    "audio",
    4096, // was 4096
    NULL,
    1,
    &AudioTask,
    0);

  /*
     BaseType_t xTaskCreatePinnedToCore(
     TaskFunction_t pvTaskCode,
     const char *constpcName,
     const uint32_t usStackDepth,
     void *constpvParameters, U
     BaseType_t uxPriority,
     TaskHandle_t *constpvCreatedTask,
     const BaseType_t xCoreID)
   * */

  // Try to start the DAC
  if (dac.begin()) {
    Serial.println("DAC init success");
  } else {
    Serial.println("DAC init fail");
  }

  initSinLookup();

  // Lower priority input thread
  xTaskCreate(ControlInput, "controls", 1024, NULL, 2, NULL);
  /*
    BaseType_t xTaskCreate(
                      TaskFunction_t pvTaskCode,
                      const char * const pcName,
                      uint16_t usStackDepth,
                      void *pvParameters,
                      UBaseType_t uxPriority,
                      TaskHandle_t *pvCreatedTask
                   );
  */
}

////////////////////////////
const unsigned int TABLESIZE = 1024;
float sinLookup[TABLESIZE];

void initSinLookup() {
  for (unsigned int i = 0; i < TABLESIZE; i++) {
    sinLookup[i] = sin(i);
  }
}


// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void Output(void *pvParameter)
{
  unsigned int frameCount = 0;
  int16_t sample[2];
  float sampleFloat[2];

  Serial.print("Audio thread started at core: ");
  Serial.println(xPortGetCoreID());

  //unsigned int counter = 0;
  float pointer = 0;

  while (1) {
    pointer = pointer + (float)pitchRaw / (float)8192;
    if ((unsigned int)pointer >= TABLESIZE) pointer = pointer - TABLESIZE;
    dac.writeSample(sinLookup[(unsigned int)(pointer + 0.5)], sinLookup[(unsigned int)(pointer + 0.5)]);

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 8192 == 0) vTaskDelay(1); // was 64, 1

  }
}


// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void loop() {}

void ControlInput(void *pvParameter)
{
  while (1) {
    // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000
    vTaskDelay(10);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);

    // take mean of ADC readings
    int samples = 128;
    int sum = 0;
    for (int i = 0; i < samples; i++) {
      sum += adc1_get_raw(ADC1_CHANNEL_0);
    }
    pitchRaw = sum / samples;


    // Serial.println(pitchRaw);

    // adcAttachPin(pitchIn);
    // pitchRaw = analogRead(pitchIn);
    // Serial.println(pitchRaw);
  }
}
