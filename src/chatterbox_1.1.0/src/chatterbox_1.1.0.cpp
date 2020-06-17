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
#include "i2sdac.h"     // see src/lib - based on https://github.com/wjslager/esp32-dac

#include <NoiseMaker.h>
#include <Shapers.h>
#include <Node.h>
#include <Control.h>
#include <Switch.h>
#include <Pot.h>
#include <Parameters.h>

#include <Switches.h>

#include <ProcessorCreator.h>
#include <Processor.h>
#include <Softclip.h>

#include <SVF.h>

#include <WebConnector.h>

#include <dispatcher.hpp>
#include <SerialMonitor.h>

#define SAMPLERATE 22050
// 44100

// I2C DAC interface
#define GPIO_DAC_DATAPORT 0
#define GPIO_DAC_BCLK 26
#define GPIO_DAC_WSEL 25
#define GPIO_DAC_DOUT 27

#define ADC_TOP 4096

#define TABLESIZE 2048 // 1024

#define ADC_SAMPLES 32 // pot reading takes mean over this number of values

#define POT_ID_F1F "f1f"
#define POT_ID_NASAL "nasal"
#define POT_ID_F2F "f2f"
#define POT_ID_F3F "f3f"
#define POT_ID_F3Q "f3q"
#define POT_ID_LARYNX "larynx"
#define POT_ID_PITCH "pitch"
#define POT_ID_GROWL "growl"

// Mixing
#define SIGNAL_GAIN 0.5f
#define F1_GAIN 1.0f
#define F2_GAIN 1.0f
#define F3_GAIN 0.5f
#define SF1_GAIN 0.4f
#define SF2_GAIN 0.8f
#define SF3_GAIN 1.0f
#define VOICED_GAIN_DEFAULT 1.0f
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

#define CREAK_SINE_RATIO 0.4f     // .4
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

#define F1F_LOW 150 // formant filter 1 centre frequency lowest value
#define F1F_HIGH 1400

#define NASAL_LOW 1000 // nasal filter 1 centre frequency lowest value
#define NASAL_HIGH 3000

#define F2F_LOW 500
#define F2F_HIGH 5000

#define F3F_LOW 50 // F3 is auxiliary filter, may or may not be a formant
#define F3F_HIGH 7000
#define F3Q_MIN 1.0f
#define F3Q_MAX 10.0f

#define GROWL_MIN 0.0f
#define GROWL_MAX 2.0f

// Fixed parameter values
#define F1Q 5.0f
#define F2Q 8.0f

// for stressed/de-stressed
#define F1_LOWQ 2.0f
#define F2_LOWQ 3.0f

#define F1_NASALQ 2.0f // notch
#define F2_NASALQ 8.0f // bandpass

#define TILT_LOW_Q 5.0f
#define TILT_HIGH_Q 5.0f

#define ATTACK_TIME 0.04f     // switch envelope
#define DECAY_TIME 0.04f      //
#define ATTACK_TIME_SING 0.2f // switch envelope
#define DECAY_TIME_SING 0.2f  //

#define SF1F 2100 // Sibilance filter 1 centre frequency
#define SF2F 3700
#define SF3F 5600

#define NASAL_LPF 3500
#define NASAL_FIXEDBPF 250
#define NASAL_FIXEDNOTCHF 1000
#define SING1F 3000
#define SING2F 8000

#define SF1Q 10.0f
#define SF2Q 12.0f
#define SF3Q 14.0f
#define NASAL_LPQ 0.7f
#define NASAL_FIXEDBPQ 5.0f
#define NASAL_FIXEDNOTCHQ 5.0f
#define SING1Q 5.0f
#define SING2Q 5.0f

/* KEY COMPONENTS */
I2sDAC dac;
// TaskHandle_t AudioTask;

Dispatcher<EventType, String, float> controlDispatcher;
SerialMonitor serialMonitor;
WebConnector webConnector = WebConnector();

class ChatterboxOutput
{
public:
    ChatterboxOutput();
    // void run();
//  static TaskHandle_t AudioTask;
static void run();

  static void OutputDAC(void *pvParameter);
};

ChatterboxOutput chatterboxOutput;

ChatterboxOutput::ChatterboxOutput(){

}

void ChatterboxOutput::run(){
   // Highest priority audio thread
   TaskHandle_t AudioTask;
   
  xTaskCreatePinnedToCore(
      OutputDAC,
      "audio",
      2048, // was 4096
      NULL,
      1,
      &AudioTask,
      0);
}
// void OutputDAC(void *pvParameter)

//////////////////////////////////////////////
//// FORWARD DECLARATIONS ////////////////////
void initLarynxWavetable();
void initFixedWavetables();
void OutputDAC(void *pvParameter);
void ControlInput(void *pvParameter);

// void SerialThread(void *pvParameter);

void togglePushSwitch(int i);
void pushSwitchChange(int i);

void pushToWebSocket();
void convertAndPush(String id, int value);

void setup();

void initInputs();

/*********************/
/*** INPUT RELATED ***/
/*********************/

Pot pots[N_POTS_VIRTUAL]; // TODO refactor to use std:array?
Switch switches[N_SWITCHES];

Switches svitches;

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

void loop(){
    // do nothing
    // vTaskDelay(1);
};

/* *** START SETUP() *** */
void setup()
{
  Serial.begin(serial_rate);

  delay(2000); // let it connect

  Serial.println("\n*** Starting Chatterbox ***\n");

 
 serialMonitor.registerCallback(controlDispatcher);
 webConnector.registerCallback(controlDispatcher);
   // Dispatcher<EventType, String, float> inputDispatcher;
  // serialMonitor.registerCallback(inputDispatcher);

  // dispatcher.broadcast(VALUE_CHANGE, "dummy", 1.23f);
  /* this makes things very noisy
   auto callbackid1 = dispatcher.addCallback([](EventType type, String name, float value) {
                          Serial.println("in chatterbox.cpp");
                          String message = name + ":" + value;
                          Serial.println(message);
                          });

  */

  dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT);

  switches[SWITCH_SF1] = Switch("sf1", 12, PUSH);             //  S1
  switches[SWITCH_SF2] = Switch("sf2", 19, PUSH);             // S2
  switches[SWITCH_SF3] = Switch("sf3", 5, PUSH);              // S3
  switches[SWITCH_VOICED] = Switch("voiced", 14, PUSH);       // S7
  switches[SWITCH_ASPIRATED] = Switch("aspirated", 17, PUSH); // S5
  switches[SWITCH_NASAL] = Switch("nasal", 23, PUSH);         // S6
  switches[SWITCH_DESTRESS] = Switch("destressed", 18, PUSH); // S4
  switches[SWITCH_STRESS] = Switch("stressed", 13, PUSH);     // S0

  switches[TOGGLE_HOLD] = Switch("hold", 16, TOGGLE);   // T0
  switches[TOGGLE_CREAK] = Switch("creak", 4, TOGGLE);  // T1
  switches[TOGGLE_SING] = Switch("sing", 2, TOGGLE);    // T2
  switches[TOGGLE_SHOUT] = Switch("shout", 15, TOGGLE); // T3

  initLarynxWavetable();
  initFixedWavetables();

  // Serial.println("portTICK_RATE_MS = " + portTICK_RATE_MS);

  // Try to start the DAC
  if (dac.begin())
  {
    // Serial.println("DAC init success");
  }
  else
  {
    // Serial.println("DAC init fail");
  }

ChatterboxOutput chatterboxOutput; 
chatterboxOutput.run();

  // Lower priority input thread
  xTaskCreatePinnedToCore(
      ControlInput,
      "ControlInput",
      2048,
      NULL,
      2, // priority
      NULL,
      1); // core

  // Low priority serial out thread


  webConnector.startWebServer();
}
/*** END SETUP() ***/







/* INITIALIZE LARYNX WAVETABLE */
int larynx = TABLESIZE / 2; // point in wavetable corresponding to closed larynx

void initLarynxWavetable()
{

  for (unsigned int i = 0; i < larynx / 2; i++)
  { // up slope+

    larynxWavetable[i] = 4.0f * (float)i / (float)larynx - 1.0f;
  }
  for (unsigned int i = larynx / 2; i < larynx; i++)
  { // down slope
    larynxWavetable[i] = 5.0f - 4.0f * (float)i / (float)larynx;
  }
  for (unsigned int i = larynx; i < TABLESIZE; i++)
  { // flat section __
    larynxWavetable[i] = -0.99f;
  }
}
/* END INITIALIZE WAVETABLE */

/* INITIALISE STANDARD WAVETABLES */
void initFixedWavetables()
{
  float sawScale = 2.0f / (float)TABLESIZE;
  float sinScale = 2.0f * PI / (float)TABLESIZE;

  for (unsigned int i = 0; i < TABLESIZE; i++)
  { // flat section __
    sawtoothWavetable[i] = ((float)i * sawScale - 1);
    sineWavetable[i] = sin((float)i * sinScale);
  }
}

/* INITIALISE INPUTS */
void initInputs()
{
  // init ADC inputs (pots)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  pots[POT_P0] = Pot("f1f", 36);
  pots[POT_P1] = Pot("f2f", 39);
  pots[POT_P2] = Pot("f3f", 32);
  pots[POT_P3] = Pot("f3q", 33);
  pots[POT_P4] = Pot("larynx", 34);
  pots[POT_P5] = Pot("pitch", 35);

  for (int i = 0; i < N_POTS_ACTUAL; i++)
  {
    adcAttachPin(pots[i].channel());
  }

  for (int i = 0; i < N_SWITCHES; i++)
  {
    pinMode(switches[i].channel(), INPUT);
    pinMode(switches[i].channel(), INPUT_PULLDOWN);
  }

  pots[POT_P0].range(ADC_TOP, F1F_LOW, F1F_HIGH);
  pots[POT_P1].range(ADC_TOP, F2F_LOW, F2F_HIGH);
  pots[POT_P2].range(ADC_TOP, F3F_LOW, F3F_HIGH);
  pots[POT_P3].range(ADC_TOP, F3Q_MIN, F3Q_MAX);
  pots[POT_P4].range(ADC_TOP, tablesize * LARYNX_MIN / 100.0f, tablesize * LARYNX_MAX / 100.0f);
  pots[POT_P5].range(ADC_TOP, PITCH_MIN, PITCH_MAX);

  pots[POT_GROWL].range(ADC_TOP, GROWL_MAX, GROWL_MIN);
}
/* END INITIALISE INPUTS */

// *** Initialise filters ***

// Variable formant filters
SVF svf1;
SVF svf2;
SVF svf3;

//   Biquad(int type, float Fc, float Q, float peakGainDB);
//Biquad *n1 = new Biquad(HIGHSHELF, 1000.0f / samplerate, F1_NASALQ, F1_NASAL_GAIN);

// fixed sibilant/fricative filters
SVF fricative1;
SVF fricative2;
SVF fricative3;

SVF nasalLP;
SVF nasalFixedBP;
SVF nasalFixedNotch;

SVF sing1;
SVF sing2;

int dt = 0;

float pitch;
// larynx is initialized at wavetable

float f1f;
float f2f;
float f3f;
float f3q;

float growl;

float emphasisGain = 1.0f;

/************************/
/* *** INPUT THREAD *** */
/************************/
void ControlInput(void *pvParameter)
{
  initInputs();

  // Dispatcher<EventType, String, float> inputDispatcher;
  // serialMonitor.registerCallback(inputDispatcher);

  // inputDispatcher.broadcast(VALUE_CHANGE, "dummy", 1.23f);

  while (1)
  {
    // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000
    vTaskDelay(2);

    bool potChanged = false;

    // take mean of ADC readings, they are prone to noise
    for (int pot = 0; pot < N_POTS_ACTUAL; pot++)
    {
      int sum = 0;
      for (int j = 0; j < ADC_SAMPLES; j++)
      {
        sum += analogRead(pots[pot].channel()); // get value from pot / ADC
      }
      pots[pot].raw(sum / ADC_SAMPLES);

      if (abs(pots[pot].previous() - pots[pot].raw()) > 32)
      { // TODO refactor raw()
        potChanged = true;
        controlDispatcher.broadcast(VALUE_CHANGE, pots[pot].id(), pots[pot].value());
        pots[pot].previous(pots[pot].raw());
      }
    }

    pots[POT_GROWL].raw(pots[POT_P4].raw()); // same as larynx TODO refactor

    if (switches[TOGGLE_CREAK].on())
    {
      pots[POT_P4].id(POT_ID_GROWL);
    }
    else
    {
      pots[POT_P4].id(POT_ID_LARYNX);
    }

    if (potChanged)
    {
      // pitch control
      pitch = pots[POT_P5].value();
    }

    growl = pots[POT_GROWL].value();

    tableStep = pitch * tablesize / samplerate; // tableStep aka delta

    if (abs(larynx - pots[POT_P4].value()) > 8)
    {
      larynx = pots[POT_P4].value();
      initLarynxWavetable();
    }

    f1f = pots[POT_P0].value();
    f2f = pots[POT_P1].value();
    f3f = pots[POT_P2].value();
    f3q = pots[POT_P3].value();

    if (switches[SWITCH_NASAL].on())
    {
      pots[POT_P0].id(POT_ID_NASAL);
      pots[POT_P0].range(ADC_TOP, NASAL_LOW, NASAL_HIGH);

      svf1.initParameters(f1f, F1_NASALQ, "notch", samplerate);
      svf2.initParameters(f2f, F2_NASALQ, "bandPass", samplerate);
    }
    else
    {
      pots[POT_P0].id(POT_ID_F1F);
      pots[POT_P0].range(ADC_TOP, F1F_LOW, F1F_HIGH);

      svf1.initParameters(f1f, F1Q, "bandPass", samplerate);
      svf2.initParameters(f2f, F2Q, "bandPass", samplerate);
    }

    svf3.initParameters(f3f, f3q, "lowPass", samplerate);

    /********************/
    /*** SWITCH INPUT ***/
    /********************/
    bool toggleChange = false;

    for (int i = 0; i < N_SWITCHES; i++)
    {                                                     
      // switch envelope generator TODO only needed for push switches
      switches[i].on(digitalRead(switches[i].channel())); // TODO refactor - how?

      if (switches[i].on() != switches[i].previous())
      {
        controlDispatcher.broadcast(VALUE_CHANGE, switches[i].id(), switches[i].on());
        if (switches[i].type() == TOGGLE)
          toggleChange = true;

        switches[i].previous(switches[i].on());

        togglePushSwitch(i); // for HOLD toggle
        // pushSwitchChange(i);
      }

      if (switches[i].type() == PUSH)
      {
        switches[i].on(
            switches[i].on() || (switches[i].hold() && switches[TOGGLE_HOLD].on()));

        if (switches[TOGGLE_HOLD].on())

        { // override envelope
          if (switches[i].on())
          {
            switches[i].gain(1);
          }
          else
          {
            switches[i].gain(0);
          }
        }
      }
    }

    if (switches[SWITCH_DESTRESS].gain())
    {
      emphasisGain = VOICED_GAIN_DESTRESSED;

      svf1.initParameters(f1f, F1_LOWQ, "lowPass", samplerate);
      svf2.initParameters(f2f, F2_LOWQ, "bandPass", samplerate);
    }
    else
    {
      emphasisGain = VOICED_GAIN_DEFAULT;

      svf1.initParameters(f1f, F1Q, "bandPass", samplerate);
      svf2.initParameters(f2f, F2Q, "bandPass", samplerate);
    }
    if (switches[SWITCH_STRESS].on())
    {
      emphasisGain = VOICED_GAIN_STRESSED;
    }
    else
    { // TODO tidy logic
      emphasisGain = VOICED_GAIN_DEFAULT;
    }

    // Toggle switches
    if (toggleChange)
    { // feels like this would be better as a switch/case

      larynxRatio = DEFAULT_LARYNX_RATIO;
      sineRatio = DEFAULT_SINE_RATIO;
      sawtoothRatio = DEFAULT_SAWTOOTH_RATIO;

      if (switches[TOGGLE_CREAK].on())
      {
        larynxRatio = CREAK_LARYNX_RATIO;
        sineRatio = CREAK_SINE_RATIO;
        sawtoothRatio = CREAK_SAWTOOTH_RATIO;
      }

      if (switches[TOGGLE_SING].on())
      {
        larynxRatio = SING_LARYNX_RATIO;
        sineRatio = SING_SINE_RATIO;
        sawtoothRatio = SING_SAWTOOTH_RATIO;

        attackTime = ATTACK_TIME_SING;
        decayTime = DECAY_TIME_SING;
        attackStep = 1.0f / (attackTime * samplerate); 
        decayStep = 1.0f / (decayTime * samplerate);
      }
      else
      {
        attackTime = ATTACK_TIME;
        decayTime = DECAY_TIME;
        attackStep = 1.0f / (attackTime * samplerate);
        decayStep = 1.0f / (decayTime * samplerate);
      }

      if (switches[TOGGLE_SHOUT].on())
      {
        larynxRatio = SHOUT_LARYNX_RATIO;
        sineRatio = SHOUT_SINE_RATIO;
        sawtoothRatio = SHOUT_SAWTOOTH_RATIO;
      }
    }
    /* pushToWebSocket(); */
  }
}
/* END INPUT THREAD */

const int potResolution = 32; // TODO sort out noise!!!!

/*
void pushToWebSocket()
{
  for (int i = 0; i < N_POTS_ACTUAL; i++)
  {
    if (abs(pots[i].raw() - pots[i].previous()) >= potResolution)
    {
      webConnector.convertAndPush(pots[i].id(), pots[i].channel());
      webConnector.convertAndPush(pots[i].id(), pots[i].value());
    }
  }
}
*/

void togglePushSwitch(int i)
{
  if (switches[i].type() != PUSH)
    return;

  if (switches[TOGGLE_HOLD].on())
  {
    if (switches[i].on())
    {
      switches[i].hold(!switches[i].hold()); // toggle, rename to flip method?
    }
  }
}

/*
void pushSwitchChange(int i)
{
  webConnector.convertAndPush(switches[i].id(), switches[i].channel());
  if (switches[i].on())
  {
    webConnector.convertAndPush(switches[i].id(), 1);
  }
  else
  {
    webConnector.convertAndPush(switches[i].id(), 0);
  }
}
*/

float minValue = 0;
float maxValue = 0;

/*****************/
/* OUTPUT THREAD */
/*****************/
void ChatterboxOutput::OutputDAC(void *pvParameter)
{
  unsigned int frameCount = 0;

  // Serial.print("Audio thread started at core: ");
  // Serial.println(xPortGetCoreID());

  fricative1.initParameters(SF1F, SF1Q, "highPass", samplerate); // TODO make SF1F etc variable?
  fricative2.initParameters(SF2F, SF2Q, "highPass", samplerate);
  fricative3.initParameters(SF3F, SF3Q, "highPass", samplerate);

  nasalLP.initParameters(NASAL_LPF, NASAL_LPQ, "lowPass", samplerate);
  nasalFixedBP.initParameters(NASAL_FIXEDBPF, NASAL_FIXEDBPQ, "bandPass", samplerate);
  nasalFixedNotch.initParameters(NASAL_FIXEDNOTCHF, NASAL_FIXEDNOTCHQ, "notch", samplerate);

 sing1.initParameters(SING1F, SING1Q, "bandPass", samplerate);
  sing2.initParameters(SING2F, SING2Q, "bandPass", samplerate);

  int pointer = 0;

  NoiseMaker creakNoise = NoiseMaker();
  NoiseMaker shoutNoise = NoiseMaker();
  NoiseMaker sf1Noise = NoiseMaker();
  NoiseMaker sf3Noise = NoiseMaker();
  // Shapers shaper = Shapers();

  ProcessorCreator processorCreator;
  Processor softClip = processorCreator.create(ProcessorCreator::SOFTCLIP);

  while (1)
  {
    // *** Read wavetable voice ***
    if (switches[TOGGLE_CREAK].on())
    {
      pointer = pointer + tableStep * (1.0f - creakNoise.stretchedNoise() * growl);
      if (pointer < 0)
        pointer = 0;
    }
    else
    {
      pointer = pointer + tableStep;
    }

    if (pointer >= tablesize)
      pointer = pointer - tablesize;

    float err = 0;

    // interpolate between neighbouring values
    err = pointer - floor(pointer);

    int lower = (int)floor(pointer);
    int upper = ((int)ceil(pointer)) % TABLESIZE;

    float larynxPart = larynxWavetable[lower] * err + larynxWavetable[upper] * (1 - err);
    float sinePart = sineWavetable[lower] * err + sineWavetable[upper] * (1 - err);
    float sawtoothPart = sawtoothWavetable[lower] * err + sawtoothWavetable[upper] * (1 - err);
    float voice = sineRatio * sinePart + sawtoothRatio * sawtoothPart + larynxRatio * larynxPart;

    // CHECK THE MIN/MAX OF A VALUE
    float monitorValue = larynxPart;
    if (monitorValue < minValue)
    {
      minValue = monitorValue;
    }
    if (monitorValue > maxValue)
    {
      maxValue = monitorValue;
    }
    if (switches[SWITCH_STRESS].on() && switches[SWITCH_DESTRESS].on())
    {
      Serial.print("minValue = ");
      Serial.println(minValue, DEC);
      Serial.print("maxValue = ");
      Serial.println(maxValue, DEC);
    }

    float noise = random(-32768, 32767) / 32768.0f;

    // ************************************************
    // ****************** THE WIRING ******************
    // ************************************************

    for (int i = 0; i < N_PUSH_SWITCHES; i++)
    {
     if (switches[i].on())
     //if (svitches.getSwitch(i).on())
      {
        switches[i].gain(switches[i].gain() + attackStep); // TODO refactor
      }
      else
      {
        switches[i].gain(switches[i].gain() - decayStep); // TODO refactor
      }
    }

    voice = (switches[SWITCH_VOICED].gain() + switches[SWITCH_NASAL].gain()) * voice / 2.0f;

    float aspiration = switches[SWITCH_ASPIRATED].gain() * noise;

    float current = (emphasisGain * (voice + aspiration)) * SIGNAL_GAIN;

    if (switches[TOGGLE_SING].on())
    {
      float sing1Val = SING1_GAIN * sing1.process(current);
      float sing2Val = SING2_GAIN * sing2.process(current);
    
      current = softClip.process((current + sing1Val + sing2Val) / 3.0f);
    }

    if (switches[TOGGLE_SHOUT].on())
    {
      current = softClip.process(current * (1.0f - growl * shoutNoise.stretchedNoise())); // amplitude mod
    }

    float s1 = fricative1.process(sf1Noise.pink(noise)) * SF1_GAIN * switches[SWITCH_SF1].gain();
    float s2 = fricative2.process(noise) * SF2_GAIN * switches[SWITCH_SF2].gain();
    float s3 = 3 * fricative3.process(noise - sf3Noise.pink(noise)) * SF3_GAIN * switches[SWITCH_SF3].gain();

    float sibilants = softClip.process((s1 + s2 + s3) / 3.0f);

    float mix1 = current + sibilants;

    // pharynx/mouth is serial
    float mix2 = softClip.process(F1_GAIN * svf1.process(mix1));
    float mix3 = softClip.process(F2_GAIN * svf2.process(mix2));
    float mix4 = mix3;

    if (switches[SWITCH_NASAL].on())
    {
      mix4 = NASAL_LP_GAIN * nasalLP.process(mix3) + NASAL_FIXEDBP_GAIN * nasalFixedBP.process(mix3) + NASAL_FIXEDNOTCH_GAIN * nasalFixedNotch.process(mix3);

      mix4 = softClip.process(mix4 / 3.0f);
    }
    float mix5 = F3_GAIN * svf3.process(mix4);

    float valL = softClip.process(current);
    float valR = creakNoise.stretchedNoise();

    dac.writeSample(sinePart, mix5); //mix5

    // ****************** END WIRING ******************

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 64 == 0)
      vTaskDelay(1); // was 64, 1
  }
}
// END OUTPUT THREAD
