#include <Arduino.h>
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 

#include <driver/adc.h> // depends on Espressif ESP32 libs

#define LARYNX_WAVE 0
#define SINE_WAVE 1
#define SQUARE_WAVE 2
#define SAWTOOTH_WAVE 3
#define TRIANGLE_WAVE 4

#define POTS 7
#define SWITCHES 5

I2sDAC dac;
TaskHandle_t AudioTask;

int potValue[POTS];
char potChannel[POTS];
bool switchValue[SWITCHES];
char switchChannel[SWITCHES];

void initInputs(){
  for(char i=0; i<POTS; i++) potValue[i] = 0;
  for(char i=0; i<SWITCHES; i++) switchValue[i] = 0;
// ADC1_CHANNEL_0  

/*
GPIO 32 7DC1_CHANNEL_CH4
GPIO 33 ADC1_CHANNEL_CH5
GPIO 34 ADC1_CHANNEL_CH6
GPIO 35 ADC1_CHANNEL_CH7
GPIO 36 ADC1_CHANNEL_CH0
GPIO 39 ADC1_CHANNEL_CH3
 */
}




// not used right now, values given via macros
// const int pitchIn = 27; // control pot
// const int waveSwitchIn = 39; // control pot

const int WAVEFORM = LARYNX_WAVE;

int pitchRaw = 0;

const unsigned int TABLESIZE = 2048;
float tablesize = (float)TABLESIZE; // save a bit of casting

float wavetable[TABLESIZE];
float tableStep = 1;

// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void setup()
{
  Serial.begin(115200);


  // begin(int sampleRate = 44100, int dataPort = 0, int bclk = 26, int wsel = 25, int dout = 33);
  dac.begin(16000, 0, 26, 25, 27);

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
    OutputDAC,
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
  for (unsigned int i = 0; i < TABLESIZE / 4; i++) {
    wavetable[i] = -0.99;
  }
  for (unsigned int i = TABLESIZE / 4; i < TABLESIZE / 2; i++) {
    wavetable[i] = (float)i * 2.0 / (tablesize / 4) - 1;
  }
  for (unsigned int i = TABLESIZE / 2; i < TABLESIZE; i++) {
    wavetable[i] = 1 - (float)i * 2.0 / (tablesize / 4);
  }
}

int flat = 0;

void initLarynxWave2() {
  Serial.println("Init larynx2 wavetable");
  int scale = flat / (TABLESIZE * 2);
  int bottomSize = TABLESIZE - TABLESIZE / scale;

  for (unsigned int i = 0; i < bottomSize / 2; i++) {
    wavetable[i] = (float)i * 2.0 / (bottomSize / 4) - 1;
  }
  for (unsigned int i = bottomSize / 2; i < bottomSize; i++) {
    wavetable[i] = 1 - (float)i * 2.0 / (bottomSize / 4);
  }

  for (unsigned int i = bottomSize; i < TABLESIZE; i++) {
    wavetable[i] = -0.99;
  }
}


// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void OutputDAC(void *pvParameter)
{
  unsigned int frameCount = 0;
  int16_t sample[2];
  float sampleFloat[2];

  Serial.print("Audio thread started at core: ");
  Serial.println(xPortGetCoreID());

  //unsigned int counter = 0;
  // float rawPointer = 0;
  int pointer = 0;
  float val = 0;
  float err = 0;

  while (1) {
    pointer = pointer + tableStep;

    if (pointer >= (float)TABLESIZE) pointer = pointer - (float)TABLESIZE;

    // interpolate between neighbouring values
    err = pointer - floor(pointer);
    val = wavetable[(int)floor(pointer)] * err + wavetable[((int)ceil(pointer)) % TABLESIZE] * (1 - err);

    dac.writeSample(val, val);

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 64 == 0) vTaskDelay(1); // was 64, 1
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

    tableStep = (float)512.0 * (float)pitchRaw / tablesize;


    sum = 0;
    for (int i = 0; i < samples; i++) {
      sum += adc1_get_raw(ADC1_CHANNEL_3); // is get_raw best input?
    }
    flat = sum / samples;

    // Serial.println(flat);

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
