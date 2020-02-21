# Chatterbox
*A manually controlled speech-like sound generator*

## Pre-Intro

Go to [KlattSyn](http://www.source-code.biz/klattSyn/)

Imagine that with a joystick & knobs.

## Introduction

Not long ago I rediscovered an electronics magazine article I first saw whilst still at school, around 1979 : Chatterbox. It was a design for a simple speech synthesizer, controlled by means of a joystick, switches etc. I wanted to build one then; now ever more so. 


![Original Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/original-chatterbox.png "Original Chatterbox")

The implementation then was with a handful of ICs, now I'd like to try something similar based around microcontroller(s). Also ties in nicely with other projects I have on the go.

The rational e for having such a machine is well made in the magazine article. But given a microcontroller, it should be relatively straightforward to add features such as MIDI I/O interface to make it a modern musical instrument, Web interface for the IoT, whatever. Should be fun.

On looking around this domain a little, some key material on numerically modelling the physics of human speech can be found in the work of Klatt (links below).

## Progress

**2020-02-21** First sounds from core hard/software (stereo 7kHz sine from wavetable @ 44.1kHz, 16 bit). This repository created.

## Proposed Implementation    

The human interface is probably the most appealing part of the original Chatterbox. So joystick, bunch of switches, a few pots. Use an inexpensive microcontroller to do the work.

* Hardware : currently I'm looking at an ESP32 module, plus an external UDA1334A DAC module (using I2S). Old-school analog joystick, pots & switches. 

* Software : Initial larynx/glottal sounds, wavetable is fast & easy. Formants (essentially tuned filters mimicing the tubes around throat, mouth, nose), I'm quite entranced by the Swiss Army Knife DSP design. Make a little lib based on that, plug together. 

## Considerations
The 12-bit ADCs of the ESP32 should be adequate for analog input from joystick & a few pots. Hopefully, given an external DAC, 16 bit output signal shouldn't be too demanding. 

A potential issue is performance on the ESP32. The 7kHz @ 44.1kHz sine I got out of the box was generated using floats with minimal arithmetic. The original Chatterbox employed 5 formant filters, the computational demand of those (plus sound generation, interface recognition) might get heavy. 

* Plan B is to use fixed-point arithmetic for the DSP calculations. Potentially orders of magnitude speed increase in speed of calculations, at the cost of spending lots of time figuring it all out.

* Plan C is to use an Arduino Due, considerably faster than the ESP32 but with expense, leaning to the point of might-as-well-use-a-Raspberry Pi.

* Plan D is to drop all this, use a mobile phone, laptop. And/or, game handset.

## Source Material
*Local copies for reference, /reference*

links todo. https://github.com/chdh/klatt-syn has most others

* Chatterbox [Witten & Madams, 1978]
* Digital Filters For Music Synthesis [Nielsen, 2000]




