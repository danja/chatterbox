#ifndef ChatterboxOutput_h
#define ChatterboxOutput_h

#include <Arduino.h>
#include "i2sdac.h"     // see src/lib - based on https://github.com/wjslager/esp32-dac

#include <SVF.h>
#include <NoiseMaker.h>
#include <ProcessorCreator.h>
#include <Switch.h>

#define SAMPLERATE 22050

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

#define TABLESIZE 2048 // 1024 is also in Wavetable.h

#define DEFAULT_SINE_RATIO 0.4f // .4
#define DEFAULT_SAWTOOTH_RATIO 0.0f
#define DEFAULT_LARYNX_RATIO 0.6f // .6


#define ATTACK_TIME 0.04f     // switch envelope
#define DECAY_TIME 0.04f      //
#define ATTACK_TIME_SING 0.2f // switch envelope
#define DECAY_TIME_SING 0.2f  //

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

// fixed sibilant/fricative filters
SVF fricative1;
SVF fricative2;
SVF fricative3;

SVF nasalLP;
SVF nasalFixedBP;
SVF nasalFixedNotch;

SVF sing1;
SVF sing2;

// Variable formant filters
SVF svf1;
SVF svf2;
SVF svf3;

 I2sDAC dac;


float emphasisGain = 1.0f;

float growl;

float samplerate = (float)SAMPLERATE;

float tablesize = (float)TABLESIZE;
float tableStep = 1;

Switch switches[N_SWITCHES];

float larynxRatio = DEFAULT_LARYNX_RATIO;
float sineRatio = DEFAULT_SINE_RATIO;
float sawtoothRatio = DEFAULT_SAWTOOTH_RATIO;

float larynxWavetable[TABLESIZE];
float sawtoothWavetable[TABLESIZE];
float sineWavetable[TABLESIZE];


float attackTime = ATTACK_TIME; // 10mS
float attackStep = 1.0f / (attackTime * samplerate);

float decayTime = DECAY_TIME; // 10mS
float decayStep = 1.0f / (decayTime * samplerate);

float minValue = 0;
float maxValue = 0;

class Processors{

public:
Processors();
// static void OutputDAC(void *pvParameter);

};


#endif