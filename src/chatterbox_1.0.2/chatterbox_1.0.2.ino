#include <Arduino.h>
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 

#include <driver/adc.h> // depends on Espressif ESP32 libs

#define ADC_TOP 4096

#define NWAVES 5

#define LARYNX_WAVE 0
#define SINE_WAVE 1
#define SQUARE_WAVE 2
#define SAWTOOTH_WAVE 3
#define TRIANGLE_WAVE 4

#define ADC_SAMPLES 64 // reading takes mean 

#define POTS 6
#define SWITCHES 5

#define POT_PITCH 5
#define POT_LARYNX 4
#define SWITCH_WAVEFORM 0

I2sDAC dac;
TaskHandle_t AudioTask;

const int WAVEFORM = LARYNX_WAVE;

int pitchRaw = 0;

const unsigned int TABLESIZE = 2048;
float tablesize = (float)TABLESIZE; // save a bit of casting

float wavetable[TABLESIZE];
float tableStep = 1;

// Inputs
int potChannel[POTS]; // adc1_channel_t
int potValue[POTS];
char switchChannel[SWITCHES];
char switchInput[SWITCHES];
char switchState[SWITCHES];

void loop() {};

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
char wave = 0;

void initWavetable(char waveform) {
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

int larynxFlat = 1024;

void initLarynxWave() {
  Serial.println("Init larynx wavetable");
  int bottomSize = (TABLESIZE * larynxFlat) / ADC_TOP;

  Serial.println(potValue[1], DEC);
  Serial.println(bottomSize, DEC);

  for (unsigned int i = 0; i < bottomSize / 2; i++) {
    // wavetable[i] = 4.0 * (float)i /tablesize - 1;
    wavetable[i] = 2.0 * (float)i /(float)bottomSize - 1;
  }
  for (unsigned int i = bottomSize / 2; i < bottomSize; i++) {
    wavetable[i] = 1 - 2.0 * (float)i/(float)bottomSize ;
  }

  for (unsigned int i = bottomSize; i < TABLESIZE; i++) {
    wavetable[i] = -0.99;
  }

    for (unsigned int i = TABLESIZE / 2; i < TABLESIZE / 2; i++) {
    wavetable[i] = (float)i * 2.0 / (tablesize / 4) - 1;
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
// void loop() {}

void initInputs() {

  // init ADC inputs (pots)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  // ADC1_CHANNEL_0

  potChannel[0] = 36; // GPIO 36
  potChannel[1] = 39; // GPIO 39
  potChannel[2] = 32; // GPIO 32
  potChannel[3] = 33; // GPIO 33
  potChannel[4] = 34; // GPIO 34
  potChannel[5] = 35; // GPIO 35

  for (char i = 0; i < POTS; i++) {
    adcAttachPin(potChannel[i]);
    potValue[i] = 0;
  }

  // init switch inputs
  switchChannel[0] = 22; //  GPIO 22
  switchChannel[1] = 23; //  GPIO 23
  switchChannel[2] = 12; // GPIO 12
  switchChannel[3] = 13; // GPIO 13
  switchChannel[4] = 14; // GPIO 14

  for (char i = 0; i < SWITCHES; i++) {
    switchInput[i] = 0;
    switchState[i] = 0;
    pinMode (switchChannel[i], INPUT);
    pinMode(switchChannel[i], INPUT_PULLDOWN);
  }
}

void ControlInput(void *pvParameter)
{
  initInputs();
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

    // Serial.println("\nPots -------------");

    for (char pot = 0; pot < POTS; pot++) { /////////////////////////////////////////////
      int sum = 0;
      for (int j = 0; j < ADC_SAMPLES; j++) {
        //   sum += adc1_get_raw((adc1_channel_t)potChannel[pot]); // (adc1_channel_t)potChannel[i]

        sum +=  analogRead(potChannel[pot]); // adc1_get_raw(ADC1_CHANNEL_0);

        // vTaskDelay(1);
      }
      potValue[pot] = sum / ADC_SAMPLES;

      // Serial.print(pot, DEC);
      // Serial.print(" : ");
      // Serial.println(potValue[pot], DEC);

    }
    // -- mean
    tableStep = (float)512.0 * (float)potValue[POT_PITCH] / tablesize;

    if (abs(larynxFlat - potValue[POT_LARYNX])> 8) {
      larynxFlat = potValue[POT_LARYNX];
      initLarynxWave();
    }

    // Serial.println("\nSwitches -------------");

    for (char svitch = 0; svitch < SWITCHES; svitch++) {
      switchInput[svitch] = digitalRead(switchChannel[svitch]);
      // Serial.print(svitch, DEC);
      // Serial.print(" : ");
      // Serial.println(switchState[svitch], DEC);
      // vTaskDelay(1);
    }

    if (switchInput[SWITCH_WAVEFORM] != switchState[SWITCH_WAVEFORM]) {
      switchState[SWITCH_WAVEFORM] = switchInput[SWITCH_WAVEFORM];
      if (wave++ > NWAVES) wave = 0;
      initWavetable(wave);
    }
  }
}
