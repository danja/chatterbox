# Chatterbox
*A manually controlled speech-like sound generator*

Based on ESP32 with external UDA1334A DAC. Very much **active** work in progress, April 2020. It is basically working, but changing day-to-day. I'm updating these docs with every development.

**Prototype demo/intro (YouTube video) :**

[![Bleepy demo video](https://img.youtube.com/vi/DT7YRqZf7U4/0.jpg)](https://youtu.be/ta3ENgdaIzM)

## Introduction

Not long ago I rediscovered an electronics magazine article I first saw whilst still at school, around 1979 : [Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/Chatterbox-1976.pdf). It was a design for a simple speech synthesizer, controlled by means of a joystick, switches etc. I wanted to build one then; now ever more so. 

![Original Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/original-chatterbox.png "Original Chatterbox")

The (mighty elegant) implementation then was with a handful of op amps & CMOS logic ICs, now I'd like to try something similar based around microcontroller(s). Also ties in nicely with other projects I have on the go.

The human interface is probably the most appealing part of the original Chatterbox. So joystick, bunch of switches, a few pots. Use an inexpensive microcontroller to do the work.

*The rationale for having such a machine is well made in the magazine article. But I like the notion of treating it a source of speech-like sounds, not a speech synth per se. Given a microcontroller, it should be relatively straightforward to add features such as MIDI I/O interface to make it a modern musical instrument, Web interface for the IoT, whatever. Should be fun.*

* [Source Code](https://github.com/danja/chatterbox/tree/master/src)

## Documentation
**Work-in-progress**
* [Quasi-blog Notes](https://github.com/danja/chatterbox/blob/master/notes.md)
* [Manual](https://github.com/danja/chatterbox/blob/master/manual/index.md) 
* [Links](https://github.com/danja/chatterbox/blob/master/links.md)
* [Local References](https://github.com/danja/chatterbox/tree/master/reference)

## Status

The core hardware & software is currently working, but changing on a daily basis. Check [Quasi-blog Notes](https://github.com/danja/chatterbox/blob/master/notes.md).

**2020-04-03** *version chatterbox_1.0.18* many hours of work later. Added basic WebSocket comms, started a browser-based UI. Added a bunch more switches to the hardware, now 4 toggle, 8 push, maxing out the ESP32 I/O except for a UART I'm reserving for MIDI and I2C which may go to a display, memory card, other physical I/O, who knows..?

**2020-03-17** *version chatterbox_1.0.10* work on 

**2020-03-11** : mini-milestone! Added a minimal Web server.

**2020-03-10** : **Milestone!**  All core functionality basically working, see video above. *version chatterbox_1.0.7*

**2020-03-05** : All available pots set to filter controls, using a state variable. Between stages have added soft clipping (tanh), works a treat! 

**2020-03-03** : **Milestone!** first formant filter essentially working.

**2020-02-02** : I've got pot 5 controlling the pitch of a wavetable-generated larynx-like tone. Pot 4 controls the waveform. (Switch 0 is hooked up to change between test waveforms).
First attempts at a filter, cutoff freq on pot 3. Something very wrong - it's industrial noises in a box. Well, pulsed farty noises...

**2020-02-29** : I now have the main controls hooked into the code : joystick, 4x potentiometers, 5x push-button switches. Of these only 2 pots and 1 switch are actually doing anything. 

2020-02-26 : First prototype case with controls put together ([Front panel v1.0](https://github.com/danja/chatterbox/blob/master/media/front-panel_v1.png "Front panel v1.0")). Digging into software.

![Chatterbox prototype hardware v 1.0](https://github.com/danja/chatterbox/blob/master/media/chatterbox_1.0.jpg "prototype hardware v 1.0")

**2020-02-21** : First sounds from core hard/software (stereo 7kHz sine from wavetable @ 44.1kHz, 16 bit). This repository created.



