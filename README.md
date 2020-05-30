# Chatterbox
*A (sometimes) manually controlled speech-like sound generator*

Based on ESP32 with external UDA1334A DAC. Very much **active** work in progress, April 2020. It is basically working, but changing day-to-day. I'm updating these docs with every development.

![New Front Panel](https://github.com/danja/chatterbox/blob/master/media/front-panel_2020-05-30.jpeg)

**Slightly older prototype demo/intro (YouTube video) :**

[![Bleepy demo video](https://img.youtube.com/vi/DT7YRqZf7U4/0.jpg)](https://youtu.be/ta3ENgdaIzM)

## Introduction

Not long ago I rediscovered an electronics magazine article I first saw whilst still at school, around 1979 : [Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/Chatterbox-1976.pdf). It was a design for a simple speech synthesizer, controlled by means of a joystick, switches etc. I wanted to build one then; now ever more so. 

![Original Chatterbox](https://github.com/danja/chatterbox/blob/master/reference/original-chatterbox.png "Original Chatterbox")

The (mighty elegant) implementation then was with a handful of op amps & CMOS logic ICs, now I'd like to try something similar based around microcontroller(s). Also ties in nicely with other projects I have on the go.

The human interface is a very appealing part of the original Chatterbox. So joystick, bunch of switches, a few pots. Use an inexpensive microcontroller to do the work. But given such a setup, it should be straightforward to add other interfaces, notably Web (via WebSockets) and MIDI.

* [Source Code](https://github.com/danja/chatterbox/tree/master/src)

## Documentation
**Work-in-progress**
* [Quasi-blog Notes](https://github.com/danja/chatterbox/blob/master/notes.md)
* [Manual](https://github.com/danja/chatterbox/blob/master/manual/index.md) 
* [Links](https://github.com/danja/chatterbox/blob/master/links.md)
* [Local References](https://github.com/danja/chatterbox/tree/master/reference)

## Status

The core hardware & software is currently working, but changing on a daily basis. Check [Quasi-blog Notes](https://github.com/danja/chatterbox/blob/master/notes.md).

**2020-05-30** *version chatterbox_1.1.0*

Spent a *lot* of time transferring everything into a permanent pretty box (pictured above). Round in circles a bit with software, but overall have made some progress in refactoring. Soon should be able to get back to the fun side of the coding.

**2020-04-15** *version chatterbox_1.1.0*

I'd got part way through implementing the Web interface and the code was getting way too messy to work with. So I'm currently refactoring everything into more manageable C++ classes. To help with this I've moved from the Android IDE to [PlatformIO](https://platformio.org/) (on VSCode). *I should have done this weeks ago!*

Also, although this stuff is all here on GitHub, I haven't really thought in terms of releases yet. So far the code versioning has been ad hoc, really just for backup purposes. But this PlatformIO environment is *so much better*, once I've done the refactoring and cleaned up minor bugs I can easily shift to doing proper releases (with [Semantic Versioning](https://semver.org/)).

**2020-04-03** *version chatterbox_1.0.18* many hours of work later. Added basic WebSocket comms, started a browser-based UI. Added a bunch more switches to the hardware, now 4 toggle, 8 push, maxing out the ESP32 I/O except for a UART I'm reserving for MIDI and also there's a free I2C I/O which may go to a display, memory card, other physical I/O, who knows..?

**2020-03-17** *version chatterbox_1.0.10* work on 

**2020-03-11** : mini-milestone! Added a minimal Web server.

**2020-03-10** : **Milestone!**  All core functionality basically working, see video above. *version chatterbox_1.0.7*

**2020-03-05** : All available pots set to filter controls, using a state variable. Between stages have added soft clipping (tanh), works a treat! 

**2020-03-03** : **Milestone!** first formant filter essentially working.

**2020-02-02** : I've got pot 5 controlling the pitch of a wavetable-generated larynx-like tone. Pot 4 controls the waveform. (Switch 0 is hooked up to change between test waveforms).
First attempts at a filter, cutoff freq on pot 3. Something very wrong - it's industrial noises in a box. Well, pulsed farty noises...

**2020-02-29** : I now have the main controls hooked into the code : joystick, 4x potentiometers, 5x push-button switches. Of these only 2 pots and 1 switch are actually doing anything. 

**2020-02-26** : First prototype case with controls put together ([Front panel v1.0](https://github.com/danja/chatterbox/blob/master/media/front-panel_v1.png "Front panel v1.0")). Digging into software.

![Chatterbox prototype hardware v 1.0](https://github.com/danja/chatterbox/blob/master/media/chatterbox_1.0.jpg "prototype hardware v 1.0")

**2020-02-21** : First sounds from core hard/software (stereo 7kHz sine from wavetable @ 44.1kHz, 16 bit). This repository created.



