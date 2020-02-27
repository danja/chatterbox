#include <Arduino.h>
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 

#include <driver/adc.h> // depends on Espressif ESP32 libs

#define LARYNX_WAVE 0
#define SINE_WAVE 1
#define SQUARE_WAVE 2
#define SAWTOOTH_WAVE 3
#define TRIANGLE_WAVE 4

I2sDAC dac;
TaskHandle_t AudioTask;


// not used right now, values given via macros
const int pitchIn = 27; // control pot
const int waveSwitchIn = 39; // control pot

const int WAVEFORM = LARYNX_WAVE;

int pitchRaw = 0;

const unsigned int TABLESIZE = 4096;
float tablesize = (float)TABLESIZE; // save a bit of casting

float wavetable[TABLESIZE];
float tableStep = 1;

// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void setup()
{
  Serial.begin(115200);


  // begin(int sampleRate = 44100, int dataPort = 0, int bclk = 26, int wsel = 25, int dout = 33);
  dac.begin(44100, 0, 26, 25, 33);

  initWavetable(WAVEFORM);

  // Serial.println("portTICK_RATE_MS = " + portTICK_RATE_MS);

  // Try to start the DAC
  if (dac.begin()) {
    Serial.println("DAC init success");
  } else {
    Serial.println("DAC init fail");
  }

  // High priority audio thread
  xTaskCreatePinnedToCore(
    Output,
    "audio",
    4096, // was 4096
    NULL,
    1,
    &AudioTask,
    0);

  // Lower priority input thread
  // xTaskCreate(ControlInput, "controls", 1024, NULL, 2, NULL);
  xTaskCreatePinnedToCore(
    ControlInput,
    "ControlInput",
    4096, // was 4096
    NULL,
    1,
    NULL,
    1);

}

////////////////////////////

void initWavetable(int waveform) {
  switch (waveform) {
    case LARYNX_WAVE:
      Serial.println("LARYNX_WAVE selected");
      initLarynxWave();
      break;

    case SINE_WAVE:
      Serial.println("SINE_WAVE selected");
      initSineWave();
      break;

    case SQUARE_WAVE:
      Serial.println("SQUARE_WAVE selected");
      initSquareWave();
      break;

    case SAWTOOTH_WAVE:
      Serial.println("SAWTOOTH_WAVE selected");
      initSawtoothWave();
      break;

    case TRIANGLE_WAVE:
      Serial.println("TRIANGLE_WAVE selected");
      initTriangleWave();
      break;
      // default:
  }

  for (int i = 0; i < TABLESIZE; i++) {
    // Serial.println(wavetable[i]);
  }
}


void initSineWave() {
  Serial.println("Init sine wavetable");
  for (unsigned int i = 0; i < TABLESIZE; i++) {
    wavetable[i] = sin(2.0 * PI * (float)i / (float)TABLESIZE);
  }
}

void initSquareWave() {
  Serial.println("Init square wavetable");
  for (unsigned int i = 0; i < TABLESIZE / 2; i++) {
    wavetable[i] = -0.99;
  }
  for (unsigned int i = TABLESIZE / 2; i < TABLESIZE; i++) {
    wavetable[i] = 0.99;
  }
}

void initSawtoothWave() {
  Serial.println("Init sawtooth wavetable");
  for (unsigned int i = 0; i < TABLESIZE; i++) {
    wavetable[i] = 2 * i / tablesize - 1;
  }
}

void initTriangleWave() {
  Serial.println("Init triangle wavetable");
  for (unsigned int i = 0; i < TABLESIZE / 2; i++) {
    wavetable[i] =  2 * i / (tablesize / 2) - 1;
  }
  for (unsigned int i = TABLESIZE / 2; i < TABLESIZE; i++) {
    wavetable[i] = 1 - 2 * i / (tablesize / 2);
  }
}

void initLarynxWave() {
  Serial.println("Init larynx wavetable");
  for (unsigned int i = 0; i < TABLESIZE / 2; i++) {
    wavetable[i] = -0.99;
  }
  for (unsigned int i = TABLESIZE / 2; i < 3 * TABLESIZE / 4; i++) {
    wavetable[i] = (float)i * 2.0 / (tablesize / 4) - 1;
  }
  for (unsigned int i = 3 * TABLESIZE / 4; i < TABLESIZE; i++) {
    wavetable[i] = 1 - (float)i * 2.0 / (tablesize / 4);
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
  float rawPointer = 0;
  int pointer = 0;

  while (1) {
    rawPointer = rawPointer + tableStep;
    
    if (rawPointer >= (float)TABLESIZE) {
      rawPointer = rawPointer - (float)TABLESIZE;

   pointer = 
    
    dac.writeSample(wavetable[(unsigned int)(pointer + 0.5)], wavetable[(unsigned int)(pointer + 0.5)]);

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 16 == 0) vTaskDelay(1); // was 64, 1
  }
}


// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void loop() {}

void ControlInput(void *pvParameter)
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  /* Disable input for testing
    while (1) {
    pitchRaw = 2048;
    vTaskDelay(10);
    tableStep = (float)pitchRaw / (float)8192;
    Serial.println("tick");
    }
  */

  int currentWaveChoice = 0;

  while (1) {
    // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000
    vTaskDelay(1);

    // take mean of ADC readings, they are prone to noise
    int samples = 256;
    int sum = 0;
    for (int i = 0; i < samples; i++) {
      sum += adc1_get_raw(ADC1_CHANNEL_0); // is get_raw best input?
    }
    pitchRaw = sum / samples;
    // -- mean

    tableStep = (float)128.0 * (float)pitchRaw / (float)TABLESIZE;

    // Serial.println(waveSwitch);

    // take mean of ADC readings, they are prone to noise
    sum = 0;
    for (int i = 0; i < samples; i++) {
      sum += adc1_get_raw(ADC1_CHANNEL_4); // is get_raw best input?
    }
    int waveSwitch = sum / (samples * 820);
    // -- mean

    if (waveSwitch != currentWaveChoice) {
      currentWaveChoice = waveSwitch;
      initWavetable(waveSwitch);
    }
  }
}
