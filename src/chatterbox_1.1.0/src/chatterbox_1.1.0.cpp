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
#include <I2SDAC.h>     // see src/lib - based on https://github.com/wjslager/esp32-dac

#include <NoiseMaker.h>
#include <Shapers.h>
#include <Node.h>
#include <Control.h>
#include <Switch.h>
#include <Pot.h>
#include <Pots.h>
#include <Parameters.h>
#include <Switches.h>

#include <Wavetable.h>

#include <ProcessorCreator.h>
#include <Processor.h>
#include <Softclip.h>

// #include <SineWavetable.h>
// #include <SawtoothWavetable.h>
#include <SVF.h>
#include <Patchbay.h>

#include <WebConnector.h>

#include <dispatcher.hpp>
#include <SerialMonitor.h>

#define SAMPLERATE 22050
//22050
// 44100

// I2C DAC interface
#define GPIO_DAC_DATAPORT 0
#define GPIO_DAC_BCLK 26
#define GPIO_DAC_WSEL 25
#define GPIO_DAC_DOUT 27

#define ADC_TOP 4096

#define ADC_SAMPLES 32 // pot reading takes mean over this number of values

// Mixing
#define SIGNAL_GAIN 0.4f

#define VOICED_GAIN_DEFAULT 1.0f
#define VOICED_GAIN_DESTRESSED 0.7f
#define VOICED_GAIN_STRESSED 1.5f

#define F1_GAIN 1.0f
#define F2_GAIN 1.0f
#define F3_GAIN 0.5f
#define SF1_GAIN 0.4f
#define SF2_GAIN 0.8f
#define SF3_GAIN 1.0f

#define SING1_GAIN 0.7f
#define SING2_GAIN 0.5f

#define NASAL_LP_GAIN 0.5f
#define NASAL_FIXEDBP_GAIN 0.6f
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

//////////////////////////////////////
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
//////////////////////////

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

float larynxWavetable[TABLESIZE];

float sawtoothWavetable[TABLESIZE];
float sineWavetable[TABLESIZE];

TaskHandle_t controlInputHandle = NULL;
TaskHandle_t outputDACHandle = NULL;

Dispatcher<EventType, String, float> controlDispatcher;
SerialMonitor serialMonitor;
WebConnector webConnector = WebConnector();

class ChatterboxOutput
{
public:
  ChatterboxOutput();

  static void run();

  static void OutputDAC(void *pvParameter);
};

ChatterboxOutput chatterboxOutput;

ChatterboxOutput::ChatterboxOutput()
{
}

void ChatterboxOutput::run()
{
  xTaskCreatePinnedToCore(
      OutputDAC,
      "audio",
      2048, // was 2048, 4096
      NULL,
      1 | portPRIVILEGE_BIT,
      &outputDACHandle, // was &AudioTask,
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

Switches switches = Switches(); ///////////////////////////////////
Pots pots;

float larynxRatio = DEFAULT_LARYNX_RATIO;
float sineRatio = DEFAULT_SINE_RATIO;
float sawtoothRatio = DEFAULT_SAWTOOTH_RATIO;

float samplerate = (float)SAMPLERATE;

float attackTime = ATTACK_TIME; // 10mS
float attackStep = 1.0f / (attackTime * samplerate);

float decayTime = DECAY_TIME; // 10mS
float decayStep = 1.0f / (decayTime * samplerate);

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

  // dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT);

  initLarynxWavetable();
  initFixedWavetables();

  // Serial.println("portTICK_RATE_MS = " + portTICK_RATE_MS);

  // Try to start the DAC

  if (dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT))
  {
    Serial.println("DAC init success");
  }
  else
  {
    Serial.println("DAC init fail");
  }

  ChatterboxOutput chatterboxOutput;
  chatterboxOutput.run();

  // Lower priority input thread
  xTaskCreatePinnedToCore(
      ControlInput,
      "ControlInput",
      2048, // was 4096
      NULL,
      2, // priority
      &controlInputHandle,
      1); // core

  // Low priority serial out thread

  webConnector.startWebServer();

  /*
  Serial.print("esp_get_free_heap_size() = ");
  Serial.println(esp_get_free_heap_size(), DEC);

  Serial.print("esp_get_minimum_free_heap_size() = ");
  Serial.println(esp_get_minimum_free_heap_size(), DEC);

  Serial.print("uxTaskGetStackHighWaterMark(controlInputHandle) = ");
  Serial.println(uxTaskGetStackHighWaterMark(controlInputHandle), DEC);

  Serial.print("uxTaskGetStackHighWaterMark(outputDACHandle) = ");
  Serial.println(uxTaskGetStackHighWaterMark(outputDACHandle), DEC);

  Serial.print("uxTaskGetNumberOfTasks() = ");
  Serial.println(uxTaskGetNumberOfTasks(), DEC);
*/
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
  {
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

  pots.init();

  switches.init();
}
/* END INITIALISE INPUTS */

// *** Initialise filters ***

// Variable formant filters
SVF svf1;
SVF svf2;
SVF svf3;

Patchbay patchbay;


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
    // vTaskDelay(1); // 2

    // Serial.println(switches.getSwitch(0).on());

    bool potChanged = false;

    // take mean of ADC readings, they are prone to noise
    for (int pot = 0; pot < N_POTS_ACTUAL; pot++)
    {
      vTaskDelay(1);
      int sum = 0;
      for (int j = 0; j < ADC_SAMPLES; j++)
      {
        sum += analogRead(pots.getPot(pot).channel()); // get value from pot / ADC
      }
      pots.getPot(pot).raw(sum / ADC_SAMPLES);

      if (abs(pots.getPot(pot).previous() - pots.getPot(pot).raw()) > 32)
      { // TODO refactor raw()
        potChanged = true;
        controlDispatcher.broadcast(VALUE_CHANGE, pots.getPot(pot).id(), pots.getPot(pot).value());
        pots.getPot(pot).previous(pots.getPot(pot).raw());
      }
    }

    pots.getPot(POT_GROWL).raw(pots.getPot(POT_P4).raw()); // same as larynx TODO refactor

    if (switches.getSwitch(TOGGLE_CREAK).on())
    {
      pots.getPot(POT_P4).id(POT_ID_GROWL);
    }
    else
    {
      pots.getPot(POT_P4).id(POT_ID_LARYNX);
      // pots.getPot(POT_P4].id(POT_ID_LARYNX);
    }

    if (potChanged)
    {
      // pitch control
      // pitch = pots.getPot(POT_P5].value();
      pitch = pots.getPot(POT_P5).value();
    }

    growl = pots.getPot(POT_GROWL).value();
    //   growl = pots.getPot(POT_GROWL].value();

    tableStep = pitch * tablesize / samplerate; // tableStep aka delta

    //  if (abs(larynx - pots.getPot(POT_P4].value()) > 8)
    if (abs(larynx - pots.getPot(POT_P4).value()) > 8)
    {
      larynx = pots.getPot(POT_P4).value();
      // larynx = pots.getPot(POT_P4].value();
      initLarynxWavetable();
    }

    f1f = pots.getPot(POT_P0).value();
    f2f = pots.getPot(POT_P1).value();
    f3f = pots.getPot(POT_P2).value();
    f3q = pots.getPot(POT_P3).value();
    /*
f1f = pots.getPot(POT_P0].value();
    f2f = pots.getPot(POT_P1].value();
    f3f = pots.getPot(POT_P2].value();
    f3q = pots.getPot(POT_P3].value();
    */
    if (switches.getSwitch(SWITCH_NASAL).on())
    {
      pots.getPot(POT_P0).id(POT_ID_NASAL);
      pots.getPot(POT_P0).range(ADC_TOP, NASAL_LOW, NASAL_HIGH);

      svf1.initParameters(f1f, F1_NASALQ, "notch", samplerate,  F1_GAIN); // NOTE not individual gain
      svf2.initParameters(f2f, F2_NASALQ, "bandPass", samplerate,  F2_GAIN);
    }
    else
    {
      pots.getPot(POT_P0).id(POT_ID_F1F);
      pots.getPot(POT_P0).range(ADC_TOP, F1F_LOW, F1F_HIGH);

      svf1.initParameters(f1f, F1Q, "bandPass", samplerate, F1_GAIN);
      svf2.initParameters(f2f, F2Q, "bandPass", samplerate, F2_GAIN);
    }



    svf3.initParameters(f3f, f3q, "lowPass", samplerate, F3_GAIN);

    /********************/
    /*** SWITCH INPUT ***/
    /********************/
    bool toggleChange = false;

    for (int i = 0; i < N_SWITCHES; i++)
    {
      bool switchVal = digitalRead(switches.getSwitch(i).channel());
      switches.getSwitch(i).on(switchVal); // TODO refactor - how?

      if (switches.getSwitch(i).on() != switches.getSwitch(i).previous())
      {
        controlDispatcher.broadcast(VALUE_CHANGE, switches.getSwitch(i).id(), switches.getSwitch(i).on());
        if (switches.getSwitch(i).type() == TOGGLE)
          toggleChange = true;

        switches.getSwitch(i).previous(switches.getSwitch(i).on());

        togglePushSwitch(i); // for HOLD toggle
        // pushSwitchChange(i);
      }

      if (switches.getSwitch(i).type() == PUSH)
      {
        switches.getSwitch(i).on(
            switches.getSwitch(i).on() || (switches.getSwitch(i).hold() && switches.getSwitch(TOGGLE_HOLD).on()));

        if (switches.getSwitch(TOGGLE_HOLD).on())

        { // override envelope
          if (switches.getSwitch(i).on())
          {
            switches.getSwitch(i).gain(1);
          }
          else
          {
            switches.getSwitch(i).gain(0);
          }
        }
      }
    }

    if (switches.getSwitch(SWITCH_DESTRESS).gain())
    {
      emphasisGain = VOICED_GAIN_DESTRESSED;

      svf1.initParameters(f1f, F1_LOWQ, "lowPass", samplerate, F1_GAIN);
      svf2.initParameters(f2f, F2_LOWQ, "bandPass", samplerate, F2_GAIN);
    }
    else
    {
      emphasisGain = VOICED_GAIN_DEFAULT;

      svf1.initParameters(f1f, F1Q, "bandPass", samplerate, F1_GAIN);
      svf2.initParameters(f2f, F2Q, "bandPass", samplerate, F2_GAIN);
    }
    if (switches.getSwitch(SWITCH_STRESS).on())
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

      if (switches.getSwitch(TOGGLE_CREAK).on())
      {
        larynxRatio = CREAK_LARYNX_RATIO;
        sineRatio = CREAK_SINE_RATIO;
        sawtoothRatio = CREAK_SAWTOOTH_RATIO;
      }

      if (switches.getSwitch(TOGGLE_SING).on())
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

      if (switches.getSwitch(TOGGLE_SHOUT).on())
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

void togglePushSwitch(int i)
{
  if (switches.getSwitch(i).type() != PUSH)
    return;

  if (switches.getSwitch(TOGGLE_HOLD).on())
  {
    if (switches.getSwitch(i).on())
    {
      switches.getSwitch(i).hold(!switches.getSwitch(i).hold()); // toggle, rename to flip method?
    }
  }
}

float minValue = 0;
float maxValue = 0;

/*****************/
/* OUTPUT THREAD */
/*****************/
void ChatterboxOutput::OutputDAC(void *pvParameter)
{

  // static SineWavetable sinWavetable;
  // static SawtoothWavetable sawWavetable;

  unsigned int frameCount = 0;

  // Serial.print("Audio thread started at core: ");
  // Serial.println(xPortGetCoreID());

  fricative1.initParameters(SF1F, SF1Q, "highPass", samplerate, SF1_GAIN); // TODO make SF1F etc variable?
  fricative2.initParameters(SF2F, SF2Q, "highPass", samplerate, SF2_GAIN);
  fricative3.initParameters(SF3F, SF3Q, "highPass", samplerate, SF3_GAIN);

  nasalLP.initParameters(NASAL_LPF, NASAL_LPQ, "lowPass", samplerate, NASAL_LP_GAIN);
  nasalFixedBP.initParameters(NASAL_FIXEDBPF, NASAL_FIXEDBPQ, "bandPass", samplerate,NASAL_FIXEDBP_GAIN);
  nasalFixedNotch.initParameters(NASAL_FIXEDNOTCHF, NASAL_FIXEDNOTCHQ, "notch", samplerate,NASAL_FIXEDNOTCH_GAIN);

  sing1.initParameters(SING1F, SING1Q, "bandPass", samplerate, SING1_GAIN);
  sing2.initParameters(SING2F, SING2Q, "bandPass", samplerate, SING2_GAIN);

  int pointer = 0;

  NoiseMaker creakNoise = NoiseMaker();
  NoiseMaker shoutNoise = NoiseMaker();
  NoiseMaker sf1Noise = NoiseMaker();
  NoiseMaker sf3Noise = NoiseMaker();
  // Shapers shaper = Shapers();

  ProcessorCreator processorCreator;
  Processor softClip = processorCreator.create(ProcessorCreator::SOFTCLIP);

  /*
 Serial.println("*******");
for(int i=0; i<TABLESIZE;i=i+100){
    Serial.println(sineWavetable[i], DEC);
    Serial.println(sinWavetable.get(i), DEC);
  }
  delay(1000);
*/

patchbay.setModules(svf1);

  while (1)
  {
    // *** Read wavetable voice ***
    if (switches.getSwitch(TOGGLE_CREAK).on())
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
    // float sinePart = sinWavetable.get(lower) * err + sinWavetable.get(upper) * (1 - err);
    // float sawtoothPart = sawWavetable.get(lower) * err + sawWavetable.get(upper) * (1 - err);

    float sinePart = sineWavetable[lower] * err + sineWavetable[upper] * (1 - err);
    float sawtoothPart = sawtoothWavetable[lower] * err + sawtoothWavetable[upper] * (1 - err);

    // tableStep = pitch * tablesize / samplerate;

    // sinePart = sin(waveX);

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
    if (switches.getSwitch(SWITCH_STRESS).on() && switches.getSwitch(SWITCH_DESTRESS).on())
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
      if (switches.getSwitch(i).on())
      //if (switches.getSwitch(i).on())
      {
        switches.getSwitch(i).gain(switches.getSwitch(i).gain() + attackStep); // TODO refactor
      }
      else
      {
        switches.getSwitch(i).gain(switches.getSwitch(i).gain() - decayStep); // TODO refactor
      }
    }

    voice = (switches.getSwitch(SWITCH_VOICED).gain() + switches.getSwitch(SWITCH_NASAL).gain()) * voice / 2.0f;

    float aspiration = switches.getSwitch(SWITCH_ASPIRATED).gain() * noise/2.0f;

    float current = (emphasisGain * (voice + aspiration)) * SIGNAL_GAIN;

    if (switches.getSwitch(TOGGLE_SING).on())
    {
      float sing1Val = sing1.process(current); // SING1_GAIN * 
      float sing2Val = sing2.process(current); // SING2_GAIN * 

      current = (current + sing1Val + sing2Val) / 2.0f; // softClip.process(
    }

    if (switches.getSwitch(TOGGLE_SHOUT).on())
    {
      current = current * (1.0f - growl * shoutNoise.stretchedNoise()); // amplitude mod softClip.process(
    }

    float s1 = fricative1.process(sf1Noise.pink(noise)) * switches.getSwitch(SWITCH_SF1).gain(); // SF1_GAIN * 
    float s2 = 1.5f * fricative2.process(noise) * switches.getSwitch(SWITCH_SF2).gain(); // SF2_GAIN * 
    float s3 = 5.0f * fricative3.process(noise - sf3Noise.pink(noise)) * switches.getSwitch(SWITCH_SF3).gain(); // SF3_GAIN * 

    float sibilants = (s1 + s2 + s3) / 3.0f; // softClip.process(

    float mix1 = current + sibilants;

    // pharynx/mouth is serial
   // float mix2 = F1_GAIN * svf1.process(mix1); // softClip.process(
  //   float mix2 = patchbay.process(mix1); // softClip.process( F1_GAIN * 
  float mix2 = svf1.process(mix1);
    float mix3 = svf2.process(mix2 * 0.9f); // softClip.process( F2_GAIN * 
    float mix4 = mix3;

    if (switches.getSwitch(SWITCH_NASAL).on())
    {
      mix4 = nasalLP.process(mix3) + nasalFixedBP.process(mix3) + nasalFixedNotch.process(mix3);
// NASAL_LP_GAIN * NASAL_FIXEDBP_GAIN * NASAL_FIXEDNOTCH_GAIN * 
      mix4 = mix4 / 3.0f; // softClip.process(
    }
    float mix5 = svf3.process(mix4); // F3_GAIN * 

    // float valL = softClip.process(current);
    // float valR = creakNoise.stretchedNoise();

    // Outputs A, B
    dac.writeSample(sinePart, mix5 ); //mix5

    // ****************** END WIRING ******************

    // Pause thread after delivering 64 samples so that other threads can do stuff

    if (frameCount++ % 64 == 0)
      vTaskDelay(1); // was 64, 1
  }
}
// END OUTPUT THREAD
