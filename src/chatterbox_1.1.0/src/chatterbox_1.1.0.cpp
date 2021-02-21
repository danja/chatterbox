/**********************************************************************************/
/*    Chatterbox                                                                  */
/*                                                                                */
/*    a voice-like sound generator                                                */
/*                                                                                */
/*    see http://github.com/danja/chatterbox                                      */
/*                                                                                */
/*    unless otherwise stated: MIT license, attribution appreciated               */
/*                                                                                */
/*    Danny Ayers 2020-2021 | danny.ayers@gmail.com | @danja | https://hyperdata.it    */
/**********************************************************************************/

#define HEADLESS false

#define CHECK_STACK false // will make noisy
#define INPUT_TASK_STACK_SIZE 72000
#define OUTPUT_TASK_STACK_SIZE 3072

#define SAMPLERATE 22000 // 16000 // want at least 22000

#include <Arduino.h>
#include <driver/adc.h> // depends on Espressif ESP32 libs
#include <I2SDAC.h>     // see src/lib - based on https://github.com/wjslager/esp32-dac

// #include "Plotter.h"

#include <Manual.h>

#include <Wavetable.h>
#include <SineWavetable.h>
#include <SawtoothWavetable.h>
#include <LarynxWavetable.h>

#include <NoiseMaker.h>
#include <Shapers.h>
#include <Node.h>
#include <Control.h>
#include <Switch.h>
#include <Pot.h>
#include <Pots.h>
// #include <Parameters.h>
#include <Switches.h>

#include <Wavetable.h>

#include <ProcessorCreator.h>
#include <Processor.h>
#include <Softclip.h>

// #include <SineWavetable.h>
// #include <SawtoothWavetable.h>
#include <SVF.h>
#include <Patchbay.h>

#include <dispatcher.hpp>
#include <SerialMonitor.h>
#include <MIDIConnector.h>
#include <WebConnector.h>

// I2C DAC interface
#define GPIO_DAC_DATAPORT 0
#define GPIO_DAC_BCLK 26
#define GPIO_DAC_WSEL 25
#define GPIO_DAC_DOUT 27

#define ADC_TOP 4096

#define ADC_SAMPLES 32 // pot reading takes mean over this number of values

// Mixing
#define SIGNAL_GAIN 0.4f

#define VOICED_GAIN_DEFAULT 1.5f
#define VOICED_GAIN_DESTRESSED 0.5f
#define VOICED_GAIN_STRESSED 2.0f

#define F1_GAIN 0.5f
#define F2_GAIN 0.5f
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

#define CALIBRATE false

float samplerate = (float)SAMPLERATE;

float stepScale = tablesize / samplerate;

I2sDAC dac;

static Manual manual;
static Patchbay patchbay;

TaskHandle_t controlInputHandle = NULL;
TaskHandle_t outputDACHandle = NULL;

Dispatcher<EventType, String, float> controlDispatcher;
SerialMonitor serialMonitor;
WebConnector webConnector; ///
MIDIConnector midiConnector;

LarynxWavetable larynxWavetable = LarynxWavetable();
SineWavetable sineTable;
SawtoothWavetable sawtoothTable;
SineWavetable calibrateTable;

// MIDI
/*
using Transport = MIDI_NAMESPACE::SerialMIDI<HardwareSerial>;
int rxPin = 21;                                    //  1; //
int txPin = 22;                                    // 3;
HardwareSerial hardwareSerial = HardwareSerial(1); // UART_NUM_1
Transport serialMIDI(hardwareSerial);
MIDI_NAMESPACE::MidiInterface<Transport> MIDI((Transport &)serialMIDI);
*/

/* calibrate */
float sweepIncrement = 4.0f;
float calibrateTestIncrement = 200.0f;

float calibratePitch = PITCH_MIN;
float calibrateStep;

float calibrateTestMin = F1F_LOW;
float calibrateTestMax = F1F_HIGH;

float calibrateTestValue = calibrateTestMin;

float lowTrip = -1.0f;
float highTrip = 1.0f;

class ChatterboxOutput
{
public:
    ChatterboxOutput();
    static void OutputDAC(void *pvParameter);
};

ChatterboxOutput::ChatterboxOutput()
{
    xTaskCreatePinnedToCore(
        OutputDAC,
        "audio",
        OUTPUT_TASK_STACK_SIZE, // was 2048, 4096
        NULL,
        10,               // 1 | portPRIVILEGE_BIT,
        &outputDACHandle, // was &AudioTask,
        0);
}

class ChatterboxInput
{
public:
    ChatterboxInput();
    static void ControlInput(void *pvParameter);
};

ChatterboxInput::ChatterboxInput()
{
    xTaskCreatePinnedToCore(
        ControlInput,
        "ControlInput",
        INPUT_TASK_STACK_SIZE, // stack size, was 4096, 64000 // high 97004
        NULL,
        2, // priority
        &controlInputHandle,
        1); // core
}







static Softclip softClip;



//// FORWARD DECLARATIONS ////////////////////
void ControlInput(void *pvParameter);

void togglePushSwitch(int i);
void pushSwitchChange(int i);

void pushToWebSocket();
void convertAndPush(String id, int value);

void setup();
// void initInputs();

/*********************/
/*** INPUT RELATED ***/
/*********************/



/*
Switches switches;
pots pots;
*/

float tableStep = 1;

int bufferIndex = 0;

// Plotted variables must be declared as globals
// double xPlot;
// double yPlot;

// Plotter plotter;

void loop(){};

/* *** START SETUP() *** */
void setup()
{
    Serial.begin(serial_rate);

    delay(3000); // let it connect

    Serial.println("\n*** Starting Chatterbox ***\n");

    // MIDI.begin(1); // MIDI_CHANNEL_OMNI Listen to all incoming messages
    // hardwareSerial.begin(31250, SERIAL_8N1, rxPin, txPin, false, 100);

    // plotter.Begin();
    // plotter.AddTimeGraph("Time graph w/ 100 points", 100, "x label", xPlot);

    serialMonitor.registerCallback(controlDispatcher);
    webConnector.registerCallback(controlDispatcher);
    midiConnector.registerCallback(controlDispatcher);

    patchbay.registerCallback(midiConnector.midiDispatcher);

    // Serial.println("portTICK_RATE_MS = " + portTICK_RATE_MS);

    if (dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT))
    {
        Serial.println("DAC init success");
    }
    else
    {
        Serial.println("DAC init fail");
    }

    sineTable.init();
    sawtoothTable.init();
    calibrateTable.init();

    static ChatterboxOutput chatterboxOutput;
    static ChatterboxInput chatterboxInput;

    // manual.initInputs();

    webConnector.startWebServer();
    midiConnector.start();

    
    if(CHECK_STACK){
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
    }
}
/*** END SETUP() ***/

/* INITIALIZE patchbay.larynxSplit WAVETABLE */
// int patchbay.larynxSplit = TABLESIZE / 2; // point in wavetable corresponding to closed patchbay.larynxSplit

/* INITIALISE INPUTS */
/*
void initInputs()
{
    // init ADC inputs (pots)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

    pots.init();
    switches.init();
}
*/
/* END INITIALISE INPUTS */

//   Biquad(int type, float Fc, float Q, float peakGainDB);
//Biquad *n1 = new Biquad(HIGHSHELF, 1000.0f / samplerate, F1_NASALQ, F1_NASAL_GAIN);

void calibrateIncrement()
{

    calibratePitch += sweepIncrement;

    if (calibratePitch >= PITCH_MAX)
    {
        calibratePitch = PITCH_MIN;
        calibrateTestValue += calibrateTestIncrement;
        if (calibrateTestValue >= calibrateTestMax)
        {
            calibrateTestValue = calibrateTestMin;
        }
        Serial.print("calibrateTestValue = ");
        Serial.println(calibrateTestValue);
        // HARD WIRED
        patchbay.f2f = calibrateTestValue;
        patchbay.svf2.initParameters(calibrateTestValue, F2Q, "bandPass", samplerate, F2_GAIN);
    }
    // sweepIncrement = calibratePitch * tablesize / samplerate;
    calibrateStep = calibratePitch * stepScale;
}

int dt = 0;

/************************/
/* *** INPUT THREAD *** */
/************************/
void ChatterboxInput::ControlInput(void *pvParameter)
{
    manual.initInputs();

    patchbay.larynxRatio = DEFAULT_LARYNX_RATIO;
    patchbay.sineRatio = DEFAULT_SINE_RATIO;
    patchbay.sawtoothRatio = DEFAULT_SAWTOOTH_RATIO;

    patchbay.attackTime = ATTACK_TIME; // 10mS
    patchbay.attackStep = 1.0f / (patchbay.attackTime * samplerate);

    patchbay.decayTime = DECAY_TIME; // 10mS
    patchbay.decayStep = 1.0f / (patchbay.decayTime * samplerate);

    while (1)
    {
        midiConnector.read();
        // vTaskDelay(1);
        //  Serial.println(MIDI.read(),BIN);
        // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000

        bool potChanged = false;

        // take mean of ADC readings, they are prone to noise
        for (int pot = 0; pot < N_POTS_ACTUAL; pot++)
        {
            vTaskDelay(1);
            int sum = 0;
            for (int j = 0; j < ADC_SAMPLES; j++)
            {
                sum += analogRead(manual.pots.getPot(pot).channel()); // get value from pot / ADC
            }
            manual.pots.getPot(pot).raw(sum / ADC_SAMPLES);

            if (abs(manual.pots.getPot(pot).previous() - manual.pots.getPot(pot).raw()) > 32)
            { // TODO refactor raw()
                potChanged = true;
                controlDispatcher.broadcast(VALUE_CHANGE, manual.pots.getPot(pot).id(), manual.pots.getPot(pot).value());
                manual.pots.getPot(pot).previous(manual.pots.getPot(pot).raw());
            }
        }

        manual.pots.getPot(POT_GROWL).raw(manual.pots.getPot(POT_P4).raw()); // same as patchbay.larynxSplit TODO refactor

        if (manual.switches.getSwitch(TOGGLE_CREAK).on())
        {
            manual.pots.getPot(POT_P4).id(POT_ID_GROWL);
        }
        else
        {
            manual.pots.getPot(POT_P4).id(POT_ID_LARYNX);
        }

        if (potChanged)
        {
            // pitch control
            //  MIDI.sendNoteOff(freqToMIDINote(patchbay.pitch), 127, 1); // temp test
            patchbay.pitch = manual.pots.getPot(POT_P5).value();
            //  MIDI.sendNoteOn(freqToMIDINote(patchbay.pitch), 127, 1);
        }

        // for midi
        if (patchbay.larynxSplit != patchbay.larynxSplitPrevious)
        {
            // initLarynxWavetable();

            // *****************
            // Serial.print("LarynxWavetable::init HWM = ");
            // Serial.println(uxTaskGetStackHighWaterMark(NULL));
            larynxWavetable.init(patchbay);

            larynxWavetable.init(patchbay);
            patchbay.larynxSplitPrevious = patchbay.larynxSplit;
            // vTaskDelay(1);
        }

        if (abs(patchbay.larynxSplit - manual.pots.getPot(POT_P4).value()) > 8)
        {
            patchbay.larynxSplit = manual.pots.getPot(POT_P4).value();
            larynxWavetable.init(patchbay);
        }

        patchbay.growl = manual.pots.getPot(POT_GROWL).value();

        tableStep = patchbay.pitch * stepScale; // tableStep aka delta

        patchbay.f1f = manual.pots.getPot(POT_P0).value();
        patchbay.f2f = manual.pots.getPot(POT_P1).value();
        patchbay.f3f = manual.pots.getPot(POT_P2).value();
        patchbay.f3q = manual.pots.getPot(POT_P3).value();

        if (manual.switches.getSwitch(SWITCH_NASAL).on())
        {
            manual.pots.getPot(POT_P0).id(POT_ID_NASAL);
            manual.pots.getPot(POT_P0).range(ADC_TOP, NASAL_LOW, NASAL_HIGH);

            patchbay.svf1.initParameters(patchbay.f1f, F1_NASALQ, "notch", samplerate, F1_GAIN); // NOTE not individual gain
            patchbay.svf2.initParameters(patchbay.f2f, F2_NASALQ, "bandPass", samplerate, F2_GAIN);
        }
        else
        {
            manual.pots.getPot(POT_P0).id(POT_ID_F1F);
            manual.pots.getPot(POT_P0).range(ADC_TOP, F1F_LOW, F1F_HIGH);

            patchbay.svf1.initParameters(patchbay.f1f, F1Q, "bandPass", samplerate, F1_GAIN);
            patchbay.svf2.initParameters(patchbay.f2f, F2Q, "bandPass", samplerate, F2_GAIN);
        }

        patchbay.svf3.initParameters(patchbay.f3f, patchbay.f3q, "lowPass", samplerate, F3_GAIN);

        /********************/
        /*** SWITCH INPUT ***/
        /********************/
        bool toggleChange = false;

        for (int i = 0; i < N_SWITCHES; i++)
        {
            bool switchVal = digitalRead(manual.switches.getSwitch(i).channel());
            manual.switches.getSwitch(i).on(switchVal); // TODO refactor - how?

            if (manual.switches.getSwitch(i).on() != manual.switches.getSwitch(i).previous())
            {
                controlDispatcher.broadcast(VALUE_CHANGE, manual.switches.getSwitch(i).id(), manual.switches.getSwitch(i).on());
                if (manual.switches.getSwitch(i).type() == TOGGLE)
                    toggleChange = true;

                manual.switches.getSwitch(i).previous(manual.switches.getSwitch(i).on());

                togglePushSwitch(i); // for HOLD toggle
            }

            if (manual.switches.getSwitch(i).type() == PUSH)
            {
                manual.switches.getSwitch(i).on(
                    manual.switches.getSwitch(i).on() || (manual.switches.getSwitch(i).hold() && manual.switches.getSwitch(TOGGLE_HOLD).on()));

                if (manual.switches.getSwitch(TOGGLE_HOLD).on())
                { // override envelope
                    if (manual.switches.getSwitch(i).on())
                    {
                        manual.switches.getSwitch(i).gain(1);
                    }
                    else
                    {
                        manual.switches.getSwitch(i).gain(0);
                    }
                }
            }
        }

        if (manual.switches.getSwitch(SWITCH_DESTRESS).gain())
        {
            patchbay.emphasisGain = VOICED_GAIN_DESTRESSED;

            patchbay.svf1.initParameters(patchbay.f1f, F1_LOWQ, "lowPass", samplerate, F1_GAIN);
            patchbay.svf2.initParameters(patchbay.f2f, F2_LOWQ, "bandPass", samplerate, F2_GAIN);
        }
        else
        {
            patchbay.emphasisGain = VOICED_GAIN_DEFAULT;

            patchbay.svf1.initParameters(patchbay.f1f, F1Q, "bandPass", samplerate, F1_GAIN);
            patchbay.svf2.initParameters(patchbay.f2f, F2Q, "bandPass", samplerate, F2_GAIN);
        }
        if (manual.switches.getSwitch(SWITCH_STRESS).on())
        {
            patchbay.emphasisGain = VOICED_GAIN_STRESSED;
        }
        else
        { // TODO tidy logic
            patchbay.emphasisGain = VOICED_GAIN_DEFAULT;
        }

        // Toggle switches
        if (toggleChange)
        { // feels like this would be better as a switch/case

            patchbay.larynxRatio = DEFAULT_LARYNX_RATIO;
            patchbay.sineRatio = DEFAULT_SINE_RATIO;
            patchbay.sawtoothRatio = DEFAULT_SAWTOOTH_RATIO;

            if (manual.switches.getSwitch(TOGGLE_CREAK).on())
            {
                patchbay.larynxRatio = CREAK_LARYNX_RATIO;
                patchbay.sineRatio = CREAK_SINE_RATIO;
                patchbay.sawtoothRatio = CREAK_SAWTOOTH_RATIO;
            }

            if (manual.switches.getSwitch(TOGGLE_SING).on())
            {
                patchbay.larynxRatio = SING_LARYNX_RATIO;
                patchbay.sineRatio = SING_SINE_RATIO;
                patchbay.sawtoothRatio = SING_SAWTOOTH_RATIO;

                patchbay.attackTime = ATTACK_TIME_SING;
                patchbay.decayTime = DECAY_TIME_SING;
                patchbay.attackStep = 1.0f / (patchbay.attackTime * samplerate);
                patchbay.decayStep = 1.0f / (patchbay.decayTime * samplerate);
            }
            else
            {
                patchbay.attackTime = ATTACK_TIME;
                patchbay.decayTime = DECAY_TIME;
                patchbay.attackStep = 1.0f / (patchbay.attackTime * samplerate);
                patchbay.decayStep = 1.0f / (patchbay.decayTime * samplerate);
            }

            if (manual.switches.getSwitch(TOGGLE_SHOUT).on())
            {
                patchbay.larynxRatio = SHOUT_LARYNX_RATIO;
                patchbay.sineRatio = SHOUT_SINE_RATIO;
                patchbay.sawtoothRatio = SHOUT_SAWTOOTH_RATIO;
            }
        }
    }
}
/* END INPUT THREAD */

void togglePushSwitch(int i)
{
    if (manual.switches.getSwitch(i).type() != PUSH)
        return;

    if (manual.switches.getSwitch(TOGGLE_HOLD).on())
    {
        if (manual.switches.getSwitch(i).on())
        {
            manual.switches.getSwitch(i).hold(!manual.switches.getSwitch(i).hold()); // toggle, rename to flip method?
        }
    }
}

float max(float a, float b, float c)
{
    float max = (a < b) ? b : a;
    return (max < c) ? c : max;
}

float minValue = 0;
float maxValue = 0;

// ProcessorCreator processorCreator;
//Softclip softClip;
// = processorCreator.create(ProcessorCreator::SOFTCLIP);

/*****************/
/* OUTPUT THREAD */
/*****************/
void ChatterboxOutput::OutputDAC(void *pvParameter)
{
    // Serial.print("Audio thread started at core: ");
    // Serial.println(xPortGetCoreID());
    // static SineWavetable sinWavetable;
    // static SawtoothWavetable sawWavetable;

    unsigned int frameCount = 0;

    patchbay.fricative1.initParameters(SF1F, SF1Q, "highPass", samplerate, SF1_GAIN); // TODO make SF1F etc variable?
    patchbay.fricative2.initParameters(SF2F, SF2Q, "highPass", samplerate, SF2_GAIN);
    patchbay.fricative3.initParameters(SF3F, SF3Q, "highPass", samplerate, SF3_GAIN);

    patchbay.nasalLP.initParameters(NASAL_LPF, NASAL_LPQ, "lowPass", samplerate, NASAL_LP_GAIN);
    patchbay.nasalFixedBP.initParameters(NASAL_FIXEDBPF, NASAL_FIXEDBPQ, "bandPass", samplerate, NASAL_FIXEDBP_GAIN);
    patchbay.nasalFixedNotch.initParameters(NASAL_FIXEDNOTCHF, NASAL_FIXEDNOTCHQ, "notch", samplerate, NASAL_FIXEDNOTCH_GAIN);

    patchbay.sing1.initParameters(SING1F, SING1Q, "bandPass", samplerate, SING1_GAIN);
    patchbay.sing2.initParameters(SING2F, SING2Q, "bandPass", samplerate, SING2_GAIN);

    //int pointer = 0;

    NoiseMaker creakNoise = NoiseMaker();
    NoiseMaker shoutNoise = NoiseMaker();
    NoiseMaker sf1Noise = NoiseMaker();
    NoiseMaker sf3Noise = NoiseMaker();

    // patchbay.setModules(svf1);

    while (1)
    {
        // *** Read wavetable voice ***
        // float readWavetable(int wavetable[], int size, float offset){
        float pointerOffset = 1.0f;
        if (manual.switches.getSwitch(TOGGLE_CREAK).on())
        {
            pointerOffset -= creakNoise.stretchedNoise() * patchbay.growl;
        }

        patchbay.larynxPart = larynxWavetable.get(tableStep * pointerOffset);

        // float sinePart = readWavetable(sineWavetable, TABLESIZE, pointerOffset);
        float sinePart = sineTable.get(tableStep * pointerOffset);
        /*
        Serial.print("pointerOffset : ");
        Serial.println(pointerOffset);
        Serial.print("sinPart : ");
        Serial.println(sinPart);
        */
        // float sawtoothPart = readWavetable(sawtoothWavetable, TABLESIZE, pointerOffset);
        float sawtoothPart = sawtoothTable.get(tableStep * pointerOffset);

        float calibratePart = calibrateTable.get(calibrateStep);

        float voice = patchbay.sineRatio * sinePart + patchbay.sawtoothRatio * sawtoothPart + patchbay.larynxRatio * patchbay.larynxPart;

        //
        if (CALIBRATE)
            voice = calibratePart;

        float noise = random(-32768, 32767) / 32768.0f;

        // ************************************************
        // ****************** THE WIRING ******************
        // ************************************************

        for (int i = 0; i < N_PUSH_SWITCHES; i++)
        {
            if (manual.switches.getSwitch(i).on())
            {
                manual.switches.getSwitch(i).gain(manual.switches.getSwitch(i).gain() + patchbay.attackStep); // TODO refactor
            }
            else
            {
                manual.switches.getSwitch(i).gain(manual.switches.getSwitch(i).gain() - patchbay.decayStep); // TODO refactor
            }
        }
        float vGain = manual.switches.getSwitch(SWITCH_VOICED).gain();
        float nGain = manual.switches.getSwitch(SWITCH_NASAL).gain();

        voice = max(vGain * patchbay.voicedGain, nGain) * voice;

        float aspiration = manual.switches.getSwitch(SWITCH_ASPIRATED).gain() * noise / 2.0f;

        float current = (patchbay.emphasisGain * (voice + aspiration)) * SIGNAL_GAIN;

        if (manual.switches.getSwitch(TOGGLE_SING).on())
        {
            float sing1Val = patchbay.sing1.process(current);
            float sing2Val = patchbay.sing2.process(current);

            current = (current + sing1Val + sing2Val) / 2.0f;
        }

        if (manual.switches.getSwitch(TOGGLE_SHOUT).on())
        {
            current = current * (1.0f - patchbay.growl * shoutNoise.stretchedNoise()); // amplitude mod
        }

        float s1 = patchbay.fricative1.process(sf1Noise.pink(noise)) * manual.switches.getSwitch(SWITCH_SF1).gain();                // SF1_GAIN *
        float s2 = 1.5f * patchbay.fricative2.process(noise) * manual.switches.getSwitch(SWITCH_SF2).gain();                        // SF2_GAIN *
        float s3 = 5.0f * patchbay.fricative3.process(noise - sf3Noise.pink(noise)) * manual.switches.getSwitch(SWITCH_SF3).gain(); // SF3_GAIN *

        float sibilants = (s1 + s2 + s3) / 3.0f;

        float mix1 = current + sibilants;

        // pharynx/mouth is serial
        // float mix2 = patchbay.process(mix1);
        float mix2 = patchbay.svf1.process(mix1);
        float mix3 = patchbay.svf2.process(mix2 * 0.9f);
        float mix4 = mix3;

        if (manual.switches.getSwitch(SWITCH_NASAL).on())
        {
            mix4 = patchbay.nasalLP.process(mix3) + patchbay.nasalFixedBP.process(mix3) + patchbay.nasalFixedNotch.process(mix3);
            mix4 = mix4 / 3.0f;
        }
        float mix5 = patchbay.svf3.process(mix4);

        float calibrateOutput = mix5;

        // xPlot = patchbay.larynxPart;

        // Outputs A, B
        dac.writeSample(calibratePart, calibrateOutput); //mix5 patchbay.larynxPart calibratePart

        // ****************** END WIRING ******************

        // CHECK THE MIN/MAX OF A VALUE
        float monitorValue = calibrateOutput;

        if (monitorValue > highTrip || monitorValue < lowTrip)
        {
            Serial.print("minValue = ");
            Serial.println(minValue, DEC);
            Serial.print("maxValue = ");
            Serial.println(maxValue, DEC);
        }

        // patchbay.larynxPart;
        if (monitorValue < minValue)
        {
            minValue = monitorValue;
        }
        if (monitorValue > maxValue)
        {
            maxValue = monitorValue;
        }
        if (manual.switches.getSwitch(SWITCH_STRESS).on() && manual.switches.getSwitch(SWITCH_DESTRESS).on())
        {
            Serial.print("minValue = ");
            Serial.println(minValue, DEC);
            Serial.print("maxValue = ");
            Serial.println(maxValue, DEC);
        }

        // Pause thread after delivering 128 samples so that other threads can do stuff
        if (frameCount++ % 128 == 0) // was 64
        {
            if (CALIBRATE)
            {
                calibrateIncrement();
            }
            vTaskDelay(1); // was 64, 1
            //  plotter.Plot();
        }
    }
}
// END OUTPUT THREAD
