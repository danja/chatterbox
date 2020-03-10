#include <Arduino.h>
#include <driver/adc.h> // depends on Espressif ESP32 libs
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 
// #include "Biquad.h";
#include "SvfLinearTrapOptimised2.hpp";

#define SAMPLERATE 22050

// I2C DAC interface
#define GPIO_DAC_DATAPORT 0 
#define GPIO_DAC_BCLK 26
#define GPIO_DAC_WSEL 25
#define GPIO_DAC_DOUT 27

#define ADC_TOP 4096

#define NWAVES 7

#define LARYNX_WAVE 0
#define SINE_WAVE 1
#define SQUARE_WAVE 2
#define SAWTOOTH_WAVE 3
#define TRIANGLE_WAVE 5
#define MAX_WAVE 6

#define ADC_SAMPLES 64 // pot reading takes mean over this number of values

#define POTS 6
// #define SWITCHES 5

#define POT_F1     0
#define POT_F2     1
#define POT_F3_F   2
#define POT_F3_Q   3
#define POT_LARYNX 4
#define POT_PITCH  5

#define SWITCH_WAVEFORM 0

#define F_MIN 0
#define F_MAX 500

#define LARYNX_MIN 0
#define LARYNX_MAX 4096
#define F1_LOW  150
#define F1_HIGH  1400
#define F2_LOW  500
#define F2_HIGH  5000
#define F3_LOW  50
#define F3_HIGH  7000

#define F1_Q 10
#define F2_Q 15
#define F3_Q_MIN  0
#define F3_Q_MAX  5

#define SF1_F  2100
#define SF2_F  3700
#define SF3_F  5600

#define SF1_Q  10
#define SF2_Q  12
#define SF3_Q  14

float inputScale[POTS];
int inputOffset[POTS];

I2sDAC dac;
TaskHandle_t AudioTask;

const int WAVEFORML = LARYNX_WAVE;
// const int WAVEFORMR = FILTERED_WAVE;

char currentWave = WAVEFORML;

int pitchRaw = 0;

const unsigned int TABLESIZE = 2048;

float samplerate = (float)SAMPLERATE;
float tablesize = (float)TABLESIZE; // save a bit of casting

float wavetableL[TABLESIZE];
float wavetableR[TABLESIZE];
float tableStep = 1;

int bufferIndex = 0;

// Inputs
int potChannel[POTS]; // adc1_channel_t
int potValue[POTS];

char switchChannel[] = {22, 23, 12, 13, 14};
float switchGain[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
char nswitches =  (int)(sizeof(switchChannel) / sizeof(switchChannel[0]));

float attackTime = 0.01f; // 10mS
float attackStep = (float)ADC_TOP / (samplerate*attackTime);

float decayTime = 0.01f; // 10mS
float decayStep = (float)ADC_TOP / (samplerate*decayTime);


void loop() {};

//   Biquad(int type, float Fc, float Q, float peakGainDB);
float fc = 0.1;
// Biquad *formant1 = new Biquad(LOWPASS, fc, 3.0, 0);
// Biquad *formant2 = new Biquad(LOWPASS, fc, 3.0, 0);
// Biquad *formant3 = new Biquad(HIGHSHELF, fc, 2.0, 0);
SvfLinearTrapOptimised2 formant1;
SvfLinearTrapOptimised2 formant2;
SvfLinearTrapOptimised2 formant3;

SvfLinearTrapOptimised2::FLT_TYPE formant1_type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE formant2_type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE formant3_type = SvfLinearTrapOptimised2::HIGH_SHELF_FILTER;

float formant1_Q;
float formant2_Q;
float formant3_Q;

SvfLinearTrapOptimised2 sf1;
SvfLinearTrapOptimised2 sf2;
SvfLinearTrapOptimised2 sf3;

SvfLinearTrapOptimised2::FLT_TYPE sf1_type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf2_type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf3_type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;

float sf1_Q = 2;
float sf2_Q = 2;
float sf3_Q = 2;


// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void setup()
{
  Serial.begin(115200);
  Serial.println("\n*** Starting Chatterbox ***\n");
  
  dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT);

  initWavetable(wavetableL, WAVEFORML);
  //  initWavetable(wavetableR, WAVEFORMR);
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

int tableSplit = 1024;

void initWavetable(float wavetable[], char waveform) {
  switch (waveform) {
    case LARYNX_WAVE:
      {
        //      Serial.println("LARYNX_WAVE selected");
        //    Serial.println("Init larynx wavetableL");
        tableSplit = inputOffset[POT_LARYNX] + inputScale[POT_LARYNX] * potValue[POT_LARYNX];
        int bottomSize = (TABLESIZE * tableSplit);

        //     Serial.println(potValue[1], DEC);
        //     Serial.println(bottomSize, DEC);

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

    case MAX_WAVE:
      {
        Serial.println("Init max wavetable");
        for (unsigned int i = 0; i < TABLESIZE - 1; i = i + 2) {
          wavetable[i] =  -0.99;
          wavetable[i + 1] =  0.99;
        }

        break;
      }

      /*
        // 2nd order IIR
        float a0 = 1;
        float a1 = 1.194;
        float a2 = -0.436;
        float b0 = 0.0605;
        float b1 = 0.121;
        float b2 = 0.0605;
        float c = 0;

        int n1, n2;
        float xn;
        float xn_1;
        float xn_2;
        float yn = 0;
        float yn_1 = 0;
        float yn_2 = 0;

        Serial.println("Init filtered wavetable");
        for (int i = 0; i < TABLESIZE; i++) {

        n1 = i - 1;
        n2 = i - 2;
        if (i == 0) {
          n1 = TABLESIZE - 1;
          n2 = TABLESIZE - 2;
        }
        if (i == 1) {
          n1 = 0;
          n2 = TABLESIZE - 1;
        }
        xn = wavetableL[i];
        xn_1 = wavetableL[n1];
        xn_2 = wavetableL[n2];

        yn = b0 * xn;
        yn += b1 * xn_1;
        yn += b2 * xn_2;
        yn += a1 * yn_1;
        yn += a2 * yn_2;

        yn_1 = yn;
        yn_2 = yn_1;

        wavetableR[i] = yn;

        }
      */
      // default:
  }
}
/* END INITIALIZE WAVETABLE */

/* INITIALISE INPUTS */
void initInputs() {

  // init ADC inputs (pots)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  potChannel[0] = 36; // GPIO 36
  potChannel[1] = 39; // GPIO 39
  potChannel[2] = 32; // GPIO 32
  potChannel[3] = 33; // GPIO 33
  potChannel[4] = 34; // GPIO 34
  potChannel[5] = 35; // GPIO 35

  for (char i = 0; i < POTS; i++) {
    adcAttachPin(potChannel[i]);
    potValue[i] = 0;
    inputScale[i] = 1.0f;
    inputOffset[i] = 0;
  }

  inputOffset[POT_F1] = F1_LOW;
  inputScale[POT_F1] = (float)(F1_HIGH - F1_LOW) / (float)ADC_TOP;
  inputOffset[POT_F2] = F2_LOW;
  inputScale[POT_F2] = (float)(F2_HIGH - F2_LOW) / (float)ADC_TOP;
  inputOffset[POT_F3_F] =  F3_LOW;
  inputScale[POT_F3_F] = (float)(F3_HIGH - F3_LOW) / (float)ADC_TOP;
  inputOffset[POT_F3_Q] =  F3_Q_MIN;
  inputScale[POT_F3_Q] = (float)(F3_Q_MAX - F3_Q_MIN) / (float)ADC_TOP;
  inputOffset[POT_LARYNX] =  F3_Q_MIN;
  inputScale[POT_LARYNX] =  (float)(LARYNX_MAX - LARYNX_MIN) / (float)ADC_TOP;
  inputOffset[POT_PITCH] =  F_MIN;
  inputScale[POT_PITCH] = (float)(F_MAX - F_MIN) / (float)ADC_TOP;
  //   (sizeof(x) / sizeof((x)[0]))


  // init switch inputs
  /*
    switchChannel[0] = 22; //  GPIO 22
    switchChannel[1] = 23; //  GPIO 23
    switchChannel[2] = 12; // GPIO 12
    switchChannel[3] = 13; // GPIO 13
    switchChannel[4] = 14; // GPIO 14
  */
  for (char i = 0; i < nswitches; i++) {
    // switchInput[i] = 0;
    switchGain[i] = 0;
    pinMode (switchChannel[i], INPUT);
    pinMode(switchChannel[i], INPUT_PULLDOWN);
  }
}
/* END INITIALISE INPUTS */


/*
   see https://mathr.co.uk/blog/2017-09-06_approximating_hyperbolic_tangent.html
*/
float softClip(float x) {
  float x2 = x * x;
  float x4 = x2 * x2;
  return x * (10395 + 1260 * x2 + 21 * x4) / (10395 + 4725 * x2 + 210 * x4 + x2 * x4);
}


//* INPUT THREAD */
void ControlInput(void *pvParameter)
{
  Serial.print("Input thread started at core: ");
  Serial.println(xPortGetCoreID());

  initInputs();
  /* Disable input for testing
    while (1) {
    pitchRaw = 2048;
    vTaskDelay(10);
    tableStep = (float)pitchRaw / (float)8192;
    Serial.println("tick");
    }
  */

  float pitchHz = 100; // Pitch
  float leveldB = 0;  // Level

  float filterGaindB = -6; // Gain to boost or cut the cutoff
  //float linearLevel = pow(10.0, leveldB / 20.0);
  //  float Q = 5; // Q

  // createSaw(nbSamples, pitchHz, leveldB, type, cutoffStartHz, cutoffEndHz, Q, filterGaindB);

/*
  formant1.setGain(filterGaindB);
  formant1.updateCoefficients(fc, formant1_Q, formant1_type, SAMPLERATE);

  formant2.setGain(filterGaindB);
  formant2.updateCoefficients(fc, formant2_Q, formant2_type, SAMPLERATE);

  formant3.setGain(filterGaindB);
  formant3.updateCoefficients(fc, formant3_Q, formant3_type, SAMPLERATE);

  sf1.setGain(filterGaindB);
  sf1.updateCoefficients(fc, sf1_Q, sf1_type, SAMPLERATE);

  sf2.setGain(filterGaindB);
  sf2.updateCoefficients(fc, sf2_Q, sf2_type, SAMPLERATE);

  sf3.setGain(filterGaindB);
  sf3.updateCoefficients(fc, sf3_Q, sf3_type, SAMPLERATE);
*/

  while (1) {
    // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000
    vTaskDelay(1);

    // take mean of ADC readings, they are prone to noise

    Serial.println("\nPots -------------");

    for (char pot = 0; pot < POTS; pot++) {
      int sum = 0;
      for (int j = 0; j < ADC_SAMPLES; j++) {
        sum +=  analogRead(potChannel[pot]); // adc1_get_raw(ADC1_CHANNEL_0);
        // vTaskDelay(1);
      }
      potValue[pot] = sum / ADC_SAMPLES;
    //  Serial.println(potValue[pot], DEC);
    }

    // PITCH POT
   // tableStep = (float)inputOffset[POT_PITCH] + (float)potValue[POT_PITCH]  * (float)inputScale[POT_PITCH] / (float)tablesize;
    
  //  tableStep = (float)512.0*(float)potValue[POT_PITCH] / tablesize;

float pitch = ((float)inputOffset[POT_PITCH] + inputScale[POT_PITCH] *  (float)potValue[POT_PITCH]);

   tableStep = pitch*tablesize/samplerate;

    /*
    Serial.println("(float)inputOffset[POT_PITCH] = ");
    Serial.print((float)inputOffset[POT_PITCH], DEC);
    Serial.println("(float)potValue[POT_PITCH] = ");
    Serial.print((float)potValue[POT_PITCH], DEC);
    Serial.println("inputScale[POT_PITCH] = ");
    Serial.print(inputScale[POT_PITCH], DEC);
    Serial.println("tableStep = ");
    Serial.print(tableStep, DEC);
*/
 
    // LARYNX (OQ) POT
    if (abs(tableSplit - potValue[POT_LARYNX]) > 8) {
      tableSplit = potValue[POT_LARYNX];
     // tableSplit = 1024; /////////////////////////////////////////
      initWavetable(wavetableL, currentWave);
    }

    //  float formant1freq = inputOffset[POT_F1] + (float)potValue[POT_F1] * inputScale[POT_F1];
    // Serial.println("formant1freq = ");
    // Serial.print(formant1freq, DEC);
    // formant1.updateCoefficients(formant1freq, F1_Q, formant1_type, samplerate);

    // float formant2freq = inputOffset[POT_F2] + (float)potValue[POT_F2] * inputScale[POT_F2];
    // Serial.println("formant2freq = ");
    // Serial.print(formant2freq, DEC);
    // formant2.updateCoefficients(formant2freq, F2_Q, formant2_type, samplerate);


    // float f3freq = inputOffset[POT_F3_F] + (float)potValue[POT_F3_F] * inputScale[POT_F3_F];
    // float f3Q = inputOffset[POT_F3_Q] + (float)potValue[POT_F3_Q] * inputScale[POT_F3_Q];

    /*
        Serial.println("f3freq = ");
        Serial.print(f3freq, DEC);
        Serial.println("f3Q = ");
        Serial.print(f3Q, DEC);
    */
    // formant3.updateCoefficients(f3freq, f3Q, formant3_type, samplerate);
    // updateCoefficients(double cutoff, double q = 0.5, FLT_TYPE type = LOW_, double sampleRate = 44100)
    /*
        fc = potValue[POT_F1];
        formant1->setFc((float)fc / (float)SAMPLERATE);

        fc = potValue[POT_F2];
        formant2->setFc((float)fc / (float)SAMPLERATE);

        fc = potValue[POT_F3_F];
        formant3->setFc((float)fc / (float)SAMPLERATE);

        float f3Q = (float)potValue[POT_F3_Q]/2048.0;
        formant3->setQ(f3Q);
    */




    /*
          Serial.println("----");
          Serial.println(filter->getA0(), DEC);
          Serial.println(filter->getA1(), DEC);
          Serial.println(filter->getA2(), DEC);
          Serial.println(filter->getB0(), DEC);
          Serial.println(filter->getB1(), DEC);
          Serial.println(filter->getB2(), DEC);
          Serial.println(filter->getC1(), DEC);
    */
    //Serial.println("ONE");
    // Serial.println(nswitches, DEC);
    // Serial.println("TWO");
    for (char i = 0; i < nswitches; i++) {
      if (digitalRead(switchChannel[i]) == 1) {
        switchGain[i] += attackStep;
        if (switchGain[i] > 1) switchGain[i] = 1;
      } else {
        switchGain[i] -= decayStep;
        if (switchGain[i] < 1) switchGain[i] = 0;
      }
      //  Serial.println(switchGain[i], DEC);
      //   vTaskDelay(1);
    }
  }

  // Serial.println("\nSwitches -------------");

  /*
    for (char svitch = 0; svitch < nswitches; svitch++) {
      switchInput[svitch] = digitalRead(switchChannel[svitch]);
      // Serial.print(svitch, DEC);
      // Serial.print(" : ");
      // Serial.println(switchState[svitch], DEC);
      // vTaskDelay(1);
    }
  */
  /*
    if (switchInput[SWITCH_WAVEFORM] != switchState[SWITCH_WAVEFORM]) {
      switchState[SWITCH_WAVEFORM] = switchInput[SWITCH_WAVEFORM];
      if (currentWave++ > NWAVES) currentWave = 0;
      initWavetable(wavetableL, currentWave);
    }
  */
}
/* END INPUT THREAD */

/* OUTPUT THREAD */
void OutputDAC(void *pvParameter)
{
  Serial.print("Output thread started at core: ");
  Serial.println(xPortGetCoreID());

  unsigned int frameCount = 0;
  int16_t sample[2];
  float sampleFloat[2];



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

    //--------------------------------

    //  valR = formant1->process(valL / 2.0);
    //  valR = formant2->process(valR / 2.0);
    //  valR = formant3->process(valR / 2.0);
    // valR = softClip(formant1.tick(valL));
    // valR = softClip(formant2.tick(valR));
    // valR = softClip(formant3.tick(valR));

    valR = switchGain[0] * random(-32768, 32767) / 32768.0f;
    // (float)randy()/16384.0f - 1.0f;

    // valR = softClip(formant1.tick(valL));

    dac.writeSample(valL, valR);


    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 64 == 0) vTaskDelay(1); // was 64, 1
  }
}
// END OUTPUT THREAD
