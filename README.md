# Chatterbox
*A manually controlled speech-like sound generator*

Based on ESP32 with external UDA1334A DAC. Work in progress.

**Prototype hardware demo (YouTube video) :**

[![Bleepy demo video](https://img.youtube.com/vi/DT7YRqZf7U4/0.jpg)](https://www.youtube.com/watch?v=DT7YRqZf7U4)

## Introduction

Not long ago I rediscovered an electronics magazine article I first saw whilst still at school, around 1979 : [Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/Chatterbox-1976.pdf). It was a design for a simple speech synthesizer, controlled by means of a joystick, switches etc. I wanted to build one then; now ever more so. 

![Original Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/original-chatterbox.png "Original Chatterbox")

The (mighty elegant) implementation then was with a handful of ICs, now I'd like to try something similar based around microcontroller(s). Also ties in nicely with other projects I have on the go.

The rationale for having such a machine is well made in the magazine article. But given a microcontroller, it should be relatively straightforward to add features such as MIDI I/O interface to make it a modern musical instrument, Web interface for the IoT, whatever. Should be fun.

* [Design Notes](https://github.com/danja/chatterbox/blob/master/design.md) 
* [Quasi-blog Notes](https://github.com/danja/chatterbox/blob/master/notes.md)
* [ToDo list](https://github.com/danja/chatterbox/blob/master/todo.md)

## Status

**2020-03-05** : All available pots set to filter controls, using a state variable. Between stages have added soft clipping (tanh), works a treat! 

**2020-03-03** : **Milestone!** first formant filter essentially working.

**2020-02-02** : I've got pot 5 controlling the pitch of a wavetable-generated larynx-like tone. Pot 4 controls the waveform. (Switch 0 is hooked up to change between test waveforms).
First attempts at a filter, cutoff freq on pot 3. Something very wrong - it's industrial noises in a box. Well, pulsed farty noises...

**2020-02-29** : I now have the main controls hooked into the code : joystick, 4x potentiometers, 5x push-button switches. Of these only 2 pots and 1 switch are actually doing anything. 

2020-02-26 : First prototype case with controls put together ([Front panel v1.0](https://github.com/danja/chatterbox/blob/master/media/front-panel_v1.png "Front panel v1.0")). Digging into software.

![Chatterbox prototype hardware v 1.0](https://github.com/danja/chatterbox/blob/master/media/chatterbox_1.0.jpg "prototype hardware v 1.0")

**2020-02-21** : First sounds from core hard/software (stereo 7kHz sine from wavetable @ 44.1kHz, 16 bit). This repository created.

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

----

**DAC Wiring**

ESP32	DAC

D26     BLCK

D27     DIN

D25     WSEL

100R in between

----

## Source Material
*Some have local copies for reference, see /reference*

links todo. https://github.com/chdh/klatt-syn has most others

**Key Sources**
* [Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/Chatterbox-1976.pdf)  [Witten & Madams, 1978]
* Digital Filters For Music Synthesis [Nielsen, 2000]

* [Faust](https://faust.grame.fr/)
* [KlattSyn](http://www.source-code.biz/klattSyn/)

**ESP32**
* [ESP32 Projects & Tutorials](https://randomnerdtutorials.com/projects-esp32/)
* [ESP32 Peripherals](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
* [ESP32 Pinout Reference: Which GPIO pins should you use?](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
* [The best input and output pins on the NodeMCU ESP32 and ESP8266 (YouTube)](https://www.youtube.com/watch?v=c0tMGlJVmkw)
* [ESP32 ADC – Read Analog Values with Arduino IDE](https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/)
* [Using the ESP32 ADC and some of its more advanced functions (YouTube)](https://www.youtube.com/watch?v=RlKMJknsNpo)
* [esp32-dac](https://github.com/wjslager/esp32-dac)
* [ESP32-ADC-Accuracy-Improvement-function](https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function)
* [Polynomial Regression Data Fit](https://arachnoid.com/polysolve/)

* [ESP-IDF Hello World](https://exploreembedded.com/wiki/Hello_World_with_ESP32_Explained)
* [ESP-IDF ADC API](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/adc.html)
* [ESP-DSP](https://github.com/espressif/esp-dsp)
* [ESP32 Parallel Programming](https://home.roboticlab.eu/en/iot-open/getting_familiar_with_your_hardware_rtu_itmo_sut/esp/esp_parallel_programming)

**Wavetables etc.**
* [the_synth](https://github.com/dzlonline/the_synth)
* [Alias-Free Digital Synthesis of Classic Analog Waveforms](https://ccrma.stanford.edu/~stilti/papers/blit.pdf)
* [Table Lookup Oscillators Using Generic Integrated Wavetables](http://mtg.upf.edu/node/485)

**Filters**
* [musicdsp.org](https://www.musicdsp.org/)
* [KVR Forum](https://www.kvraudio.com/forum/)
* [Biquads](https://www.earlevel.com/main/2003/02/28/biquads/)
* [The digital state variable filter](https://www.earlevel.com/main/2003/03/02/the-digital-state-variable-filter/)
* [A Collection of Useful C++ Classes for Digital Signal Processing](https://github.com/vinniefalco/DSPFilters)

**Fixed-point maths**
* [Fixed Point Math Library for C](https://sourceforge.net/p/fixedptc/)
* [related thread on StackOverflow](https://stackoverflow.com/questions/10067510/fixed-point-arithmetic-in-c-programming)

https://www.switchdoc.com/2018/04/esp32-tutorial-debouncing-a-button-press-using-interrupts/
