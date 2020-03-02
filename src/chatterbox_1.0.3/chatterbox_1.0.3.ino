#include <Arduino.h>
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 

#include <driver/adc.h> // depends on Espressif ESP32 libs

#define ADC_TOP 4096

#define NWAVES 6

#define LARYNX_WAVE 0
#define SINE_WAVE 1
#define SQUARE_WAVE 2
#define SAWTOOTH_WAVE 3
#define TRIANGLE_WAVE 4
#define FILTERED_WAVE 5

#define ADC_SAMPLES 64 // pot reading takes mean over this number of values

#define POTS 6
#define SWITCHES 5

#define POT_PITCH 5
#define POT_LARYNX 4
#define SWITCH_WAVEFORM 0

I2sDAC dac;
TaskHandle_t AudioTask;

const int WAVEFORML = SQUARE_WAVE;
const int WAVEFORMR = FILTERED_WAVE;

int pitchRaw = 0;

const unsigned int TABLESIZE = 2048;
float tablesize = (float)TABLESIZE; // save a bit of casting

float wavetableL[TABLESIZE];
float wavetableR[TABLESIZE];
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

  initWavetable(wavetableL, WAVEFORML);
  initWavetable(wavetableR, WAVEFORMR);
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

/* INITIALIZE WAVETABLE */
char wave = 0;
int tableSplit = 1024;

void initWavetable(float wavetable[], char waveform) {
  switch (waveform) {
    case LARYNX_WAVE:
      {
        Serial.println("LARYNX_WAVE selected");
        Serial.println("Init larynx wavetableL");
        int bottomSize = (TABLESIZE * tableSplit) / ADC_TOP;

        Serial.println(potValue[1], DEC);
        Serial.println(bottomSize, DEC);

        for (unsigned int i = 0; i < bottomSize / 2; i++) {
          wavetable[i] = 2.0 * (float)i / (float)bottomSize - 1;
        }
        for (unsigned int i = bottomSize / 2; i < bottomSize; i++) {
          wavetable[i] = 1 - 2.0 * (float)i / (float)bottomSize ;
        }

        for (unsigned int i = bottomSize; i < TABLESIZE; i++) {
          wavetable[i] = -0.99;
        }

        for (unsigned int i = TABLESIZE / 2; i < TABLESIZE / 2; i++) {
          wavetable[i] = (float)i * 2.0 / (tablesize / 4) - 1;
        }
        break;
      }

    case SINE_WAVE:
      {
        Serial.println("Init sine wavetable");
        for (unsigned int i = 0; i < TABLESIZE; i++) {
          wavetable[i] = sin(2.0 * PI * (float)i / (float)TABLESIZE);
        }
        break;
      }

    case SQUARE_WAVE:
      {
        Serial.println("Init square wavetable");
           int bottomSize = (TABLESIZE * tableSplit) / ADC_TOP;
        for (unsigned int i = 0; i < bottomSize; i++) {
          wavetable[i] = -0.99;
        }
        for (unsigned int i = bottomSize; i < TABLESIZE; i++) {
          wavetable[i] = 0.99;
        }
        break;
      }

    case SAWTOOTH_WAVE:
      {
        Serial.println("Init sawtooth wavetable");
        for (unsigned int i = 0; i < TABLESIZE; i++) {
          wavetable[i] = 2 * i / tablesize - 1;
        }
        break;
      }

    case TRIANGLE_WAVE:
      {
        Serial.println("Init triangle wavetable");
        for (unsigned int i = 0; i < TABLESIZE / 2; i++) {
          wavetable[i] =  2 * i / (tablesize / 2) - 1;
        }
        for (unsigned int i = TABLESIZE / 2; i < TABLESIZE; i++) {
          wavetable[i] = 1 - 2 * i / (tablesize / 2);
        }
        break;
      }
      
    case FILTERED_WAVE:
      {
        Serial.println("Init filtered wavetable");
        for (unsigned int i = 0; i < TABLESIZE; i++) {
          wavetable[i] = -wavetableL[i];
        }
        break;
      }
      // default:
  }
}
/* END INITIALIZE WAVETABLE */

/* INITIALISE INPUTS */
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
/* END INITIALISE INPUTS */

/* INPUT THREAD */
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

    if (abs(tableSplit - potValue[POT_LARYNX]) > 8) {
      tableSplit = potValue[POT_LARYNX];
       ///////////////////////////////
      initWavetable(wavetableL, SQUARE_WAVE);
      initWavetable(wavetableR, FILTERED_WAVE);
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
    //  initWavetable(wavetableL, wave);
    }
  }
}
/* END INPUT THREAD */

/* OUTPUT THREAD */
void OutputDAC(void *pvParameter)
{
  unsigned int frameCount = 0;
  int16_t sample[2];
  float sampleFloat[2];

  Serial.print("Audio thread started at core: ");
  Serial.println(xPortGetCoreID());

  int pointer = 0;


  while (1) {
    pointer = pointer + tableStep;

    if (pointer >= (float)TABLESIZE) pointer = pointer - (float)TABLESIZE;

    float err = 0;

    // interpolate between neighbouring values
    err = pointer - floor(pointer);

    int lower = (int)floor(pointer);
    int upper = ((int)ceil(pointer)) % TABLESIZE;

    float valL = wavetableL[lower] * err + wavetableL[upper] * (1 - err);
    float valR = wavetableR[lower] * err + wavetableR[upper] * (1 - err);

    dac.writeSample(valL, valR);

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 64 == 0) vTaskDelay(1); // was 64, 1
  }
}
// END OUTPUT THREAD
