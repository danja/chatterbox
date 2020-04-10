/**********************************************************************************/
/*    Chatterbox                                                                  */
/*                                                                                */
/*    a voice-like sound generator                                                */
/*                                                                                */
/*    see http://github.com/danja/chatterbox                                      */
/*                                                                                */
/*    unless otherwise stated: MIT license, attribution appreciated               */
/*                                                                                */
/*    Danny Ayers 2020 | danny.ayers@gmail.com | #danja | https://hyperdata.it    */
/**********************************************************************************/


#include <Arduino.h>
#include <driver/adc.h> // depends on Espressif ESP32 libs
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 

#include "SvfLinearTrapOptimised2.hpp";

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#define SERIAL_RATE 115200

#define SAMPLERATE 22050

// I2C DAC interface
#define GPIO_DAC_DATAPORT 0
#define GPIO_DAC_BCLK 26
#define GPIO_DAC_WSEL 25
#define GPIO_DAC_DOUT 27

#define ADC_TOP 4096

#define TABLESIZE 1024

#define ADC_SAMPLES 64 // pot reading takes mean over this number of values

#define INPUT_LOCAL 0
#define INPUT_WEB 1

/* CONTROLS */
#define N_POTS_ACTUAL 6
#define N_POTS_VIRTUAL 7

// ACTUAL Pots
#define POT_P0 0
#define POT_P1 1
#define POT_P2 2
#define POT_P3 3
#define POT_P4 4
#define POT_P5 5

// VIRTUAL Pots
#define POT_GROWL 6 // ????

#define POT_ID_F1F      "f1f"
#define POT_ID_NASAL    "nasal"
#define POT_ID_F2F      "f2f"
#define POT_ID_F3F      "f3f"
#define POT_ID_F3Q      "f3q"
#define POT_ID_LARYNX   "larynx"
#define POT_ID_PITCH    "pitch"
#define POT_ID_GROWL    "growl"

#define N_SWITCHES 12
#define N_PUSH_SWITCHES 8

#define SWITCH_VOICED 3
#define SWITCH_ASPIRATED 4
#define SWITCH_SF1 0 // Sibilance Filter 1
#define SWITCH_SF2 1
#define SWITCH_SF3 2

#define SWITCH_NASAL 5
#define SWITCH_DESTRESS 6
#define SWITCH_STRESS 7

#define TOGGLE_HOLD 8
#define TOGGLE_CREAK 9
#define TOGGLE_SING 10
#define TOGGLE_SHOUT 11

#define PUSH 0
#define TOGGLE 1

// Mixing
#define SIGNAL_GAIN 0.5f
#define F1_GAIN 1.0f
#define F2_GAIN 1.0f
#define F1PLUS_GAIN 0.2f
#define F2PLUS_GAIN 0.2 f
#define F3_GAIN 0.5f
#define SF1_GAIN 0.4f
#define SF2_GAIN 0.8f
#define SF3_GAIN 1.0f
#define VOICED_GAIN_DESTRESSED 0.7f
#define VOICED_GAIN_STRESSED 1.5f
#define NASAL_LP_GAIN 0.5f
#define SING1_GAIN 0.7f
#define SING2_GAIN 0.5f
#define NASAL_FIXEDBP_GAIN 0.8f
#define NASAL_FIXEDNOTCH_GAIN 1.0f

#define DEFAULT_SINE_RATIO 0.4f // .4
#define DEFAULT_SAWTOOTH_RATIO 0.0f
#define DEFAULT_LARYNX_RATIO 0.6f // .6

#define CREAK_SINE_RATIO 0.4f // .4
#define CREAK_SAWTOOTH_RATIO 0.2f // .
#define CREAK_LARYNX_RATIO 0.4f

#define SING_SINE_RATIO 0.7f
#define SING_SAWTOOTH_RATIO 0.0f
#define SING_LARYNX_RATIO 0.3f

#define SHOUT_SINE_RATIO 0.0f
#define SHOUT_SAWTOOTH_RATIO 0.8f
#define SHOUT_LARYNX_RATIO 0.2f

// Variable parameter ranges
#define PITCH_MIN 20
#define PITCH_MAX 500

#define LARYNX_MIN 5 // % of wave is larynx open
#define LARYNX_MAX 95

#define F1F_LOW  150 // formant filter 1 centre frequency lowest value
#define F1F_HIGH  1400

#define NASAL_LOW  1000 // nasal filter 1 centre frequency lowest value
#define NASAL_HIGH  3000

#define F2F_LOW  500
#define F2F_HIGH  5000

#define F3F_LOW  50 // F3 is auxiliary filter, may or may not be a formant
#define F3F_HIGH  7000
#define F3Q_MIN  1.0f
#define F3Q_MAX  10.0f

#define GROWL_MIN 0.0f
#define GROWL_MAX 2.0f

// Fixed parameter values
#define F1Q 5.0f
#define F2Q 8.0f
#define F1PLUSQ 5.0f
#define F2PLUSQ 8.0f

// for stressed/de-stressed
#define F1_LOWQ 2.0f
#define F2_LOWQ 3.0f
#define F1PLUS_LOWQ 2.0f
#define F2PLUS_LOWQ 3.0f

#define F1_NASALQ 2.0f // notch
#define F2_NASALQ 8.0f // bandpass

#define TILT_LOW_Q 5.0f
#define TILT_HIGH_Q 5.0f

#define ATTACK_TIME 0.04f // switch envelope 
#define DECAY_TIME 0.04f  // 
#define ATTACK_TIME_SING 0.2f // switch envelope 
#define DECAY_TIME_SING 0.2f  // 

#define SF1F  2100 // Sibilance filter 1 centre frequency
#define SF2F  3700
#define SF3F  5600

#define NASAL_LPF 3500
#define NASAL_FIXEDBPF 250
#define NASAL_FIXEDNOTCHF 1000
#define SING1F 3000
#define SING2F 8000

#define SF1Q  10.0f
#define SF2Q  12.0f
#define SF3Q  14.0f
#define NASAL_LPQ 0.7f
#define NASAL_FIXEDBPQ 5.0f
#define NASAL_FIXEDNOTCHQ 5.0f
#define SING1Q 5.0f
#define SING2Q 5.0f

/*** WEB COMMS ***/
#define HTTP_PORT 80

/**** Local Network specific - TODO must pull out ****/
const char* ssid = "TP-LINK_AC17DC"; // HIDE ME!
const char* password =  "33088297"; // HIDE ME!

IPAddress localIP(192, 168, 0, 142);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
/***********************************************/

/* KEY COMPONENTS */
I2sDAC dac;
TaskHandle_t AudioTask;
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

/*********************/
/*** INPUT RELATED ***/
/*********************/

// short string identifier
String potID[N_POTS_VIRTUAL];

// for scaling ADC readings to required values
float inputScale[N_POTS_VIRTUAL];
int inputOffset[N_POTS_VIRTUAL];

// Inputs TODO move to defines
int potChannel[] = {36, 39, 32, 33, 34, 35};
int potValue[N_POTS_VIRTUAL];
int previousPotValue[N_POTS_VIRTUAL];

char switchType[] = {PUSH, PUSH, PUSH, PUSH, PUSH, PUSH, PUSH, PUSH,
                     TOGGLE, TOGGLE, TOGGLE, TOGGLE
                    };

String switchID[N_SWITCHES];

// TODO move to defines
char switchChannel[] = {19, 23, 12, 13, 14,    17, 18, 5,     16, 15, 2, 4}; // TODO move this to a define,
// is N_SWITCHES
// char nswitches =  (int)(sizeof(switchChannel) / sizeof(switchChannel[0]));

bool switchHold[N_PUSH_SWITCHES];

float switchGain[N_SWITCHES];
float switchValue[N_SWITCHES];
float previousSwitchValue[N_SWITCHES];

float larynxRatio = DEFAULT_LARYNX_RATIO;
float sineRatio = DEFAULT_SINE_RATIO;
float sawtoothRatio = DEFAULT_SAWTOOTH_RATIO;

// save a bit of casting - better just to define as floats, will cast to int automatically when needed?
float samplerate = (float)SAMPLERATE;
float tablesize = (float)TABLESIZE;

float attackTime = ATTACK_TIME; // 10mS
float attackStep = 1.0f / (attackTime * samplerate);

float decayTime = DECAY_TIME; // 10mS
float decayStep = 1.0f / (decayTime * samplerate);

float larynxWavetable[TABLESIZE];
float sawtoothWavetable[TABLESIZE];
float sineWavetable[TABLESIZE];

float tableStep = 1;

int bufferIndex = 0;

void loop() { // do nothing
  // vTaskDelay(1);
};


/* *** START SETUP() *** */
void setup()
{
  Serial.begin(SERIAL_RATE);
  Serial.println("\n*** Starting Chatterbox ***\n");

  dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT);

  initLarynxWavetable();
  initFixedWavetables();

  // Serial.println("portTICK_RATE_MS = " + portTICK_RATE_MS);


  // Try to start the DAC
  if (dac.begin()) {
    Serial.println("DAC init success");
  } else {
    Serial.println("DAC init fail");
  }

  // Highest priority audio thread
  xTaskCreatePinnedToCore(
    OutputDAC,
    "audio",
    2048, // was 4096
    NULL,
    1,
    &AudioTask,
    0);

  // Lower priority input thread
  xTaskCreatePinnedToCore(
    ControlInput,
    "ControlInput",
    2048,
    NULL,
    2, // priority
    NULL,
    1); // core

  potID[0] = POT_ID_F1F;
  potID[1] = POT_ID_F2F;
  potID[2] = POT_ID_F3F;
  potID[3] = POT_ID_F3Q;
  potID[4] = POT_ID_LARYNX;
  potID[5] = POT_ID_PITCH;

  potID[6] = POT_ID_GROWL;

  for (int i = 0; i < N_SWITCHES; i++) {
    switchValue[i] = false;
    previousSwitchValue[i] = false;
  }

  switchID[0] = "sf1";
  switchID[1] = "sf2";
  switchID[2] = "sf3";
  switchID[3] = "voiced";
  switchID[4] = "aspirated";

  switchID[5] = "nasal";
  switchID[6] = "destressed";
  switchID[7] = "stressed";

  switchID[8] = "hold";
  switchID[9] = "creak";
  switchID[10] = "sing";
  switchID[11] = "shout";

  startWebServer();
}
/*** END SETUP() ***/

/* INITIALIZE WAVETABLE */
int larynx = 1024; // point in wavetable corresponding to closed larynx

void initLarynxWavetable() { // creates wavetable of this shape, approximating : /\__

  for (unsigned int i = 0; i < larynx / 2; i++) { // up slope /
    larynxWavetable[i] = 2.0f * (float)i / (float)larynx - 1.0f;
  }
  for (unsigned int i = larynx / 2; i < larynx; i++) { // down slope \
    larynxWavetable[i] = 1.0f - 2.0f * (float)i / (float)larynx ;
  }

  for (unsigned int i = larynx; i < TABLESIZE; i++) { // flat section __
    larynxWavetable[i] = -0.99;
  }
}
/* END INITIALIZE WAVETABLE */

/* INITIALISE STANDARD WAVETABLES */
void initFixedWavetables() {

  float sawScale = 2.0f / (float)TABLESIZE;
  float sinScale = 2.0f * PI / (float)TABLESIZE;

  for (unsigned int i = 0; i < TABLESIZE; i++) { // flat section __
    sawtoothWavetable[i] = ((float)i * sawScale - 1) ;
    sineWavetable[i] = sin((float)i * sinScale);
  }
}

char controlSource = INPUT_LOCAL;

/* INITIALISE INPUTS */
void initInputs() {

  // init ADC inputs (pots)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  for (char i = 0; i < N_POTS_ACTUAL; i++) {
    adcAttachPin(potChannel[i]);
  }
  for (char i = 0; i < N_POTS_VIRTUAL; i++) {
    potValue[i] = 0;
    previousPotValue[i] = 0;
    inputScale[i] = 1.0f;
  }

  for (char i = 0; i < N_PUSH_SWITCHES; i++) {
    switchHold[i] = 0; // off
  }

  for (char i = 0; i < N_SWITCHES; i++) {
    switchGain[i] = 0;
    pinMode (switchChannel[i], INPUT);
    pinMode(switchChannel[i], INPUT_PULLDOWN);
  }

  // calculate values for offset/scaling from pots to parameters
  inputOffset[POT_P5] =  PITCH_MIN;
  inputScale[POT_P5] = (float)(PITCH_MAX - PITCH_MIN) / (float)ADC_TOP;
  inputScale[POT_P4] =  tablesize * (float)(LARYNX_MAX - LARYNX_MIN) / 100.0f;
  inputOffset[POT_P4] = tablesize * (float)LARYNX_MIN / 100.0f;

  inputOffset[POT_P0] = F1F_LOW;
  inputScale[POT_P0] = (float)(F1F_HIGH - F1F_LOW) / (float)ADC_TOP;
  inputOffset[POT_P1] = F2F_LOW;
  inputScale[POT_P1] = (float)(F2F_HIGH - F2F_LOW) / (float)ADC_TOP;
  inputOffset[POT_P2] =  F3F_LOW;
  inputScale[POT_P2] = (float)(F3F_HIGH - F3F_LOW) / (float)ADC_TOP;
  inputOffset[POT_P3] =  F3Q_MIN;
  inputScale[POT_P3] = (float)(F3Q_MAX - F3Q_MIN) / (float)ADC_TOP;

  inputOffset[POT_GROWL] =  GROWL_MIN;
  inputScale[POT_GROWL] = (float)(GROWL_MAX - GROWL_MIN) / (float)ADC_TOP;
}
/* END INITIALISE INPUTS */

/**********************************************/
/*** LITTLE GENERATION/PROCESSING FUNCTIONS ***/
/**********************************************/
/*
   tanh approximation
   see https://mathr.co.uk/blog/2017-09-06_approximating_hyperbolic_tangent.html
*/
float softClip(float x) {
  float x2 = x * x;
  float x4 = x2 * x2;
  return x * (10395 + 1260 * x2 + 21 * x4) / (10395 + 4725 * x2 + 210 * x4 + x2 * x4);
}

// https://web.archive.org/web/20150226171912/http://home.earthlink.net/~ltrammell/tech/newpink.htm
// http://www.firstpr.com.au/dsp/pink-noise/
float b0 = 0.0f;
float b1 = 0.0f;
float b2 = 0.0f;

float pink(float white) {
  //float white = (float)random(0, 32767) / 32768.0f; // TODO check white is +/- 1

  b0 = softClip(0.99765f * b0 + white * 0.0990460f);
  b1 = softClip(0.96300f * b1 + white * 0.2965164f);
  b2 = softClip(0.57000f * b2 + white * 1.0526913f);
  return softClip(b0 + b1 + b2 + white * 0.1848f);
}

// Logistic map chaotic signal
float lx = 0.5f;
float logisticK = 3.5f;
float x;
int logisticLoops = 1;

float logistic() {
  for (int i = 0; i < logisticLoops; i++) {
    x = lx * logisticK * (1 - lx);
    if (x > 1) x = 0.5;
    lx = x;
  }
  /*
    Serial.println();
    Serial.println("logisticK");
    Serial.println(logisticK, DEC);
    Serial.println(x, DEC);
  */
  return x;
}

/* from https://en.wikipedia.org/wiki/Smoothstep */
float smootherstep(float edge0, float edge1, float x) {
  // Scale, and clamp x to 0..1 range
  x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  // Evaluate polynomial
  return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

float clamp(float x, float lowerlimit, float upperlimit) {
  if (x < lowerlimit)
    x = lowerlimit;
  if (x > upperlimit)
    x = upperlimit;
  return x;
}

int stretch = 64;
int is = 0;
float stretchStartLevel = 0.5f;
float stretchEndLevel = 0.5f;
float stretchStep = 0.0f;
float currentX = 0;

float stretchedNoise() { // TODO refactor me
  if (is < stretch) {
    float diff = stretchEndLevel - stretchStartLevel;
    currentX = currentX + stretchStep;

    is++;
    //smootherstep(-0.99f, 0.99f, raw);
    //  return current;
    return 2.0f * smootherstep(0.0f, 0.99f, currentX) - 1;
  }
  is = 0;
  stretchStartLevel = stretchEndLevel;
  currentX = stretchStartLevel;
  stretchEndLevel = (float)random(0, 32767) / 32768.0f;
  stretchStep = (stretchEndLevel - stretchStartLevel) / (float)stretch;
  return 2.0f * smootherstep(0.0f, 0.99f, stretchStartLevel) - 1;
  //  return startLevel;
}
/**************************************************/
/*** END LITTLE GENERATION/PROCESSING FUNCTIONS ***/
/**************************************************/

// *** Initialise filters ***

// Variable formant filters
SvfLinearTrapOptimised2 f1;
SvfLinearTrapOptimised2 f2;
SvfLinearTrapOptimised2 f3;

SvfLinearTrapOptimised2 f1Plus;
SvfLinearTrapOptimised2 f2Plus;

//   Biquad(int type, float Fc, float Q, float peakGainDB);
//Biquad *n1 = new Biquad(HIGHSHELF, 1000.0f / samplerate, F1_NASALQ, F1_NASAL_GAIN);

SvfLinearTrapOptimised2 fTiltLow;
SvfLinearTrapOptimised2 fTiltHigh;


SvfLinearTrapOptimised2::FLT_TYPE f1Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE f2Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;

SvfLinearTrapOptimised2::FLT_TYPE f1PlusType = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE f2PlusType = SvfLinearTrapOptimised2::BAND_PASS_FILTER;

SvfLinearTrapOptimised2::FLT_TYPE nasalF1Type = SvfLinearTrapOptimised2::NOTCH_FILTER;

SvfLinearTrapOptimised2::FLT_TYPE f3Type = SvfLinearTrapOptimised2::LOW_PASS_FILTER;

// fixed sibilant/fricative filters
SvfLinearTrapOptimised2 sf1;
SvfLinearTrapOptimised2 sf2;
SvfLinearTrapOptimised2 sf3;

// fixed others
SvfLinearTrapOptimised2 nasalLP;
SvfLinearTrapOptimised2 nasalFixedBP;
SvfLinearTrapOptimised2 nasalFixedNotch;
SvfLinearTrapOptimised2 sing1;
SvfLinearTrapOptimised2 sing2;

SvfLinearTrapOptimised2::FLT_TYPE sf1Type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf2Type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf3Type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;

SvfLinearTrapOptimised2::FLT_TYPE nasalLPType = SvfLinearTrapOptimised2::LOW_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE nasalFixedBPType = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE nasalFixedNotchType = SvfLinearTrapOptimised2::NOTCH_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sing1Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sing2Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;

SvfLinearTrapOptimised2::FLT_TYPE tiltLowType = SvfLinearTrapOptimised2::LOW_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE tiltHighType = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;

float pitch;
// larynx is initialized at wavetable
float f1f;
float f2f;
float f1Plusf;
float f2Plusf;
float f3f;
float f3q;

float growl;

float initialGain = 1.0f;


/************************/
/* *** INPUT THREAD *** */
/************************/
void ControlInput(void *pvParameter)
{
  initInputs();

  while (1) {
    // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000
    vTaskDelay(2);

    // take mean of ADC readings, they are prone to noise

    for (char pot = 0; pot < N_POTS_ACTUAL; pot++) {
      int sum = 0;
      for (int j = 0; j < ADC_SAMPLES; j++) {
        sum +=  analogRead(potChannel[pot]); // get value from pot / ADC

      }
      potValue[pot] = sum / ADC_SAMPLES;
    }

    potValue[POT_GROWL] = potValue[POT_P4]; // same as larynx

    if (switchValue[TOGGLE_CREAK]) {
      potID[POT_P4] = POT_ID_GROWL;
    } else {
      potID[POT_P4] = POT_ID_LARYNX;
    }


    // pitch control
    pitch = ((float)inputOffset[POT_P5] + inputScale[POT_P5] *  (float)potValue[POT_P5]);

    //logisticK = ((float)inputOffset[POT_GROWL] + inputScale[POT_GROWL] *  (float)potValue[POT_GROWL]);
    growl = ((float)inputOffset[POT_GROWL] + inputScale[POT_GROWL] *  (float)potValue[POT_GROWL]);;

    if (controlSource == INPUT_LOCAL) {
      tableStep = pitch * tablesize / samplerate; // tableStep aka delta
    }
    //if (TOGGLE_SHOUT) {

    //} else {
      // larynx control
      if (abs(larynx - potValue[POT_P4]) > 8) {

        float potFraction = (float)potValue[POT_P4] / (float)ADC_TOP;

        if (controlSource == INPUT_LOCAL) {
          larynx = inputOffset[POT_P4] + potFraction * inputScale[POT_P4];
        }
        initLarynxWavetable();
      }
    //}

    if (controlSource == INPUT_LOCAL) {
      f1f = inputOffset[POT_P0] + (float)potValue[POT_P0] * inputScale[POT_P0];
      f2f = inputOffset[POT_P1] + (float)potValue[POT_P1] * inputScale[POT_P1];
      f3f = inputOffset[POT_P2] + (float)potValue[POT_P2] * inputScale[POT_P2];
      f3q = inputOffset[POT_P3] + (float)potValue[POT_P3] * inputScale[POT_P3];
    }

    f1Plusf = 3 * f1f;
    f2Plusf = 3 * f2f;

    if (switchValue[SWITCH_NASAL]) {
      potID[POT_P0] = POT_ID_NASAL;
      inputOffset[POT_P0] = NASAL_LOW;
      inputScale[POT_P0] = (float)(NASAL_HIGH - NASAL_LOW) / (float)ADC_TOP;
      f1.updateCoefficients(f1f, F1_NASALQ, nasalF1Type, samplerate);
      f2.updateCoefficients(f2f, F2_NASALQ, f2Type, samplerate);
    } else {
      potID[POT_P0] = POT_ID_F1F;
      inputOffset[POT_P0] = F1F_LOW;
      inputScale[POT_P0] = (float)(F1F_HIGH - F1F_LOW) / (float)ADC_TOP;
      f1.updateCoefficients(f1f, F1Q, f1Type, samplerate); // TODO allow variable Q?
      f2.updateCoefficients(f2f, F2Q, f2Type, samplerate);
    }

    f1Plus.updateCoefficients(f1Plusf, F1PLUSQ, f1PlusType, samplerate); // TODO allow variable Q?
    f2Plus.updateCoefficients(f2Plusf, F1PLUSQ, f2PlusType, samplerate);

    fTiltLow.updateCoefficients(f1f, TILT_LOW_Q, tiltLowType, samplerate);
    fTiltHigh.updateCoefficients(f2f, TILT_HIGH_Q, tiltHighType, samplerate);

    // TILT_LOW_Q

    f3.updateCoefficients(f3f, f3q, f3Type, samplerate);
    // updateCoefficients(double cutoff, double q = 0.5, FLT_TYPE type = LOW_, double sampleRate = 44100)

    /********************/
    /*** SWITCH INPUT ***/
    /********************/
    bool toggleChange = false;

    for (char i = 0; i < N_SWITCHES; i++) { // switch envelope generator TODO only needed for push switches
      switchValue[i] = digitalRead(switchChannel[i]);
      if (switchValue[i] != previousSwitchValue[i]) {
        if (switchType[i] == TOGGLE) toggleChange = true;
        previousSwitchValue[i] = switchValue[i];
        togglePushSwitch(i); // for HOLD toggle
        pushSwitchChange(i);
      }

      if (switchType[i] == PUSH) {
        switchValue[i] = switchValue[i] || (switchHold[i] && switchValue[TOGGLE_HOLD]);

        if (switchValue[TOGGLE_HOLD]) { // override envelope
          if (switchValue[i]) {
            switchGain[i] = 1;
          } else {
            switchGain[i] = 0;
          }
        }
      }
    }

    if (switchGain[SWITCH_DESTRESS] > 0.5f) {
      initialGain = VOICED_GAIN_DESTRESSED;
      f1.updateCoefficients(f1f, F1_LOWQ, f1Type, samplerate); // TODO allow variable Q?
      f2.updateCoefficients(f2f, F2_LOWQ, f2Type, samplerate);
      f1Plus.updateCoefficients(f1Plusf, F1PLUS_LOWQ, f1PlusType, samplerate); // TODO allow variable Q?
      f2Plus.updateCoefficients(f2Plusf, F2PLUS_LOWQ, f2PlusType, samplerate);
    } else {
      f1.updateCoefficients(f1f, F1Q, f1Type, samplerate); // TODO allow variable Q?
      f2.updateCoefficients(f2f, F2Q, f2Type, samplerate);
      f1Plus.updateCoefficients(f1Plusf, F1PLUSQ, f1PlusType, samplerate); // TODO allow variable Q?
      f2Plus.updateCoefficients(f2Plusf, F2PLUSQ, f2PlusType, samplerate);
    }
    if (switchGain[SWITCH_STRESS] > 0.5f) {
      initialGain = VOICED_GAIN_STRESSED;
    } else { // TODO tidy logic
      //     f1.updateCoefficients(f1f, F1Q, f1Type, samplerate); // TODO allow variable Q?
      //     f2.updateCoefficients(f2f, F2Q, f2Type, samplerate);
    }

    // Toggle switches
    if (toggleChange) { // feels like this would be better as a switch/case

      larynxRatio = DEFAULT_LARYNX_RATIO;
      sineRatio = DEFAULT_SINE_RATIO;
      sawtoothRatio = DEFAULT_SAWTOOTH_RATIO;

      //  Serial.println("HANDLE TOGGLE");
      if (switchValue[TOGGLE_CREAK]) {
        larynxRatio = CREAK_LARYNX_RATIO;
        sineRatio = CREAK_SINE_RATIO;
        sawtoothRatio = CREAK_SAWTOOTH_RATIO;
      }

      // float attackStep;
      // float decayStep;

      if (switchValue[TOGGLE_SING]) {
        larynxRatio = SING_LARYNX_RATIO;
        sineRatio = SING_SINE_RATIO;
        sawtoothRatio = SING_SAWTOOTH_RATIO;

        attackTime = ATTACK_TIME_SING;
        decayTime = DECAY_TIME_SING;
        attackStep =  1.0f / (attackTime * samplerate); //  (float)ADC_TOP / (samplerate * attackTime); //  (float)ADC_TOP
        decayStep =  1.0f / (decayTime * samplerate);
      } else {
        attackTime = ATTACK_TIME;
        decayTime = DECAY_TIME;
        attackStep = 1.0f / (attackTime * samplerate);
        decayStep =  1.0f / (decayTime * samplerate);
      }

      if (switchValue[TOGGLE_SHOUT]) {
        larynxRatio = SHOUT_LARYNX_RATIO;
        sineRatio = SHOUT_SINE_RATIO;
        sawtoothRatio = SHOUT_SAWTOOTH_RATIO;
      }
      /*
            Serial.println();
            Serial.println("larynxRatio");
            Serial.println(larynxRatio);
            Serial.println("sineRatio");
            Serial.println(sineRatio);
            Serial.println("sawtoothRatio");
            Serial.println(sawtoothRatio);
      */
    }
    pushToWebSocket();
  }
}
/* END INPUT THREAD */

const int potResolution = 32; // TODO sort out noise!!!!

void pushToWebSocket() {
  for (int i = 0; i < N_POTS_ACTUAL; i++) {
    if (abs(potValue[i] - previousPotValue[i]) >= potResolution) {
      convertAndPush(potID[i], potChannel[i]);
      convertAndPush(potID[i], potValue[i]);
      previousPotValue[i] = potValue[i];
    }
  }
}

void togglePushSwitch(char i) {
  if (i >= N_PUSH_SWITCHES) return;

  if (switchValue[TOGGLE_HOLD]) {
    if (switchValue[i]) {
      switchHold[i] = !switchHold[i]; // toggle
    }
  }
}

void pushSwitchChange(char i) {
  convertAndPush(switchID[i], switchChannel[i]);
  if (switchValue[i]) {
    convertAndPush(switchID[i], 1);
  } else {
    convertAndPush(switchID[i], 0);
  }
}

void convertAndPush(String id, int value) {
  char snum[5];
  itoa(value, snum, 10);
  String message = id + ":" + snum;
  Serial.println(message);
  ws.textAll(message);
  //   ws.textAll((char*)text);
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {

  if (type == WS_EVT_CONNECT) {

    Serial.println("Websocket client connection received");
    client->text("Hello from ESP32 Server");

  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WS Client disconnected");
  }  else if (type == WS_EVT_DATA) {

    Serial.println("Data received: ");

    for (int i = 0; i < len; i++) {
      Serial.print((char) data[i]);
    }
    Serial.println();
  }
}

/*****************/
/* OUTPUT THREAD */
/*****************/
void OutputDAC(void *pvParameter)
{
  unsigned int frameCount = 0;
  int16_t sample[2];
  float sampleFloat[2];

  Serial.print("Audio thread started at core: ");
  Serial.println(xPortGetCoreID());

  sf1.updateCoefficients(SF1F, SF1Q, sf1Type, samplerate); // TODO make SF1F etc variable?
  sf2.updateCoefficients(SF2F, SF2Q, sf2Type, samplerate);
  sf3.updateCoefficients(SF3F, SF3Q, sf3Type, samplerate);
  sf3.updateCoefficients(SF3F, SF3Q, sf3Type, samplerate);

  nasalLP.updateCoefficients(NASAL_LPF, NASAL_LPQ, nasalLPType, samplerate);
  nasalFixedBP.updateCoefficients(NASAL_FIXEDBPF, NASAL_FIXEDBPQ, nasalFixedNotchType, samplerate);
  nasalFixedNotch.updateCoefficients(NASAL_FIXEDNOTCHF, NASAL_FIXEDNOTCHQ, nasalFixedNotchType, samplerate);

  sing1.updateCoefficients(SING1F, SING1Q, sing1Type, samplerate);
  sing2.updateCoefficients(SING2F, SING2Q, sing2Type, samplerate);

  int pointer = 0;

  while (1) {
    // *** Read wavetable voice ***

    if (switchValue[TOGGLE_CREAK]) {
      pointer = pointer + tableStep * (1.0f - stretchedNoise() * growl);
      if (pointer < 0) pointer = 0;
    } else {
      pointer = pointer + tableStep;
    }

    if (pointer >= tablesize) pointer = pointer - tablesize;

    float err = 0;

    // interpolate between neighbouring values
    err = pointer - floor(pointer);

    int lower = (int)floor(pointer);
    int upper = ((int)ceil(pointer)) % TABLESIZE;

    // float sine = sineWavetable[lower] * err + sineWavetable[upper] * (1 - err);
    // float saw = sawtoothWavetable[lower] * err + sawtoothWavetable[upper] * (1 - err);

    float larynxPart = larynxWavetable[lower] * err + larynxWavetable[upper] * (1 - err);
    float sinePart = sineWavetable[lower] * err + sineWavetable[upper] * (1 - err);
    float sawtoothPart = sawtoothWavetable[lower] * err + sawtoothWavetable[upper] * (1 - err);
    float voice = sineRatio * sinePart + sawtoothRatio * sawtoothPart + larynxRatio * larynxPart;

    float noise = random(-32768, 32767) / 32768.0f;
    // noise = noise / 2.0f;

    // ************************************************
    // ****************** THE WIRING ******************
    // ************************************************

    for (int i = 0; i < N_PUSH_SWITCHES; i++) {
      if (switchValue[i]) {
        switchGain[i] += attackStep;
        if (switchGain[i] > 1) switchGain[i] = 1;
      } else {
        switchGain[i] -= decayStep;
        if (switchGain[i] < 0) switchGain[i] = 0;
      }
    }

    voice = switchGain[SWITCH_VOICED] * voice;

    float aspiration = switchGain[SWITCH_ASPIRATED] * noise;

    float current = (initialGain * (voice + aspiration)) * SIGNAL_GAIN;

    if (switchValue[TOGGLE_SING]) {
      float sing1Val = SING1_GAIN * sing1.tick(current);
      float sing2Val  = SING2_GAIN * sing2.tick(current);
      current = softClip((current + sing1Val + sing2Val) / 3.0f);
    }

    if (switchValue[TOGGLE_SHOUT]) {
      current = softClip(current * (1.0f - growl * stretchedNoise())); // amplitude mod
    }
    //float fTiltLow_in = (voice + aspiration) / 2.0f;
    //  float fTiltHigh_in = (voice + aspiration) / 2.0f;

    float s1 = sf1.tick(pink(noise)) * SF1_GAIN * switchGain[SWITCH_SF1];
    float s2 = sf2.tick(noise) * SF2_GAIN * switchGain[SWITCH_SF2];
    float s3 = sf3.tick(noise - pink(noise)) * SF3_GAIN * switchGain[SWITCH_SF3];

    float sibilants = softClip((s1 + s2 + s3) / 3.0f);

    current = current + sibilants;

    // pharynx/mouth is serial
    current = softClip(F1_GAIN * f1.tick(current) + F1PLUS_GAIN * f1Plus.tick(current));
    current = softClip(F2_GAIN * f2.tick(current) ); // + F2PLUS_GAIN * f2Plus.tick(current)

    if (switchValue[SWITCH_NASAL]) {
      current = NASAL_LP_GAIN * nasalLP.tick(current)
                + NASAL_FIXEDBP_GAIN * nasalFixedBP.tick(current)
                + NASAL_FIXEDNOTCH_GAIN * nasalFixedNotch.tick(current);
      current = softClip(current / 3.0f);
    }
    current = F3_GAIN * f3.tick(current);

    float  valL = softClip(current);
    float  valR = stretchedNoise();

    //softClip(softClip(current)); // voice pink(noise)

    dac.writeSample(valL, valR);
    // dac.writeSample(sine, saw);

    // ****************** END WIRING ******************

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 64 == 0) vTaskDelay(1); // was 64, 1
  }
}
// END OUTPUT THREAD

/* WEB SERVER THREAD */

void startWebServer()
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Configures static IP address
  if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to : ");
  Serial.println(WiFi.localIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    // request->send(200, "text/plain", "Hello from Chatterbox!");
    request->send(SPIFFS, "/index.html", String(), false, pageProcessor);
  });

  server.on("/chatterbox.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/chatterbox.html", String(), false, pageProcessor);
  });
  server.on("/chatterbox.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/chatterbox.css", String(), false, pageProcessor);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/favicon.ico", String(), false, pageProcessor);
  });
}
/* END WEB SERVER */

String pageProcessor(const String& var) {
  char snum[5];

  if (var == "pitch") {
    itoa((int)pitch, snum, 10);
    return snum;
  }
  if (var == "larynx") {
    itoa(larynx, snum, 10);
    return snum;
  }
  if (var == "f1f") {
    itoa(f1f, snum, 10);
    return snum;
  }
  if (var == "f2f") {
    itoa(f2f, snum, 10);
    return snum;
  }
  if (var == "f3f") {
    itoa(f3f, snum, 10);
    return snum;
  }
  if (var == "f3q") {
    itoa(f3q, snum, 10);
    return snum;
  }

  for (char i = 0; i < N_SWITCHES; i++) {
    if (var.compareTo(switchID[i]) == 0) {
      if (switchValue[i]) {
        return "on";
      } else {
        return "off";
      }
    }

  }

  /*
    if (var == "Q") return "QWERT";
    if (var == "RR") return F("RRRRR");
    int x = 1024;
    char snum[5];
    itoa(x, snum, 10);
    if (var == "X") return snum;
    float f = 1.123456789;
    char c[50]; //size of the number
    sprintf(c, "%g", f);
    if (var == "Y") return c;
  */
  return String();
}