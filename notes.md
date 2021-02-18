**2021-02-18** *version chatterbox_1.1.0*

Now I've got a JTAG interface board I want to get cracking on the *headless* version of Chatterbox, ie. no manual controls, just a MIDI in, audio out (and whatever WiFi/Bluetooth I get set up).

Decided the best approach would be to wrap the manual-control oriented bits into their own class ```Manual```.

So far I've put the ADC init and the pot & switch variables over there. Have to have a look how best to wire this with the input task (thread, is tied to one processor, the output task tied to the other). 

Once I'd got it running the signal was very noisy with crunchy artifacts. This has happened before, when the fix seemed to be tweaking the stack sizes for the processors.

I tried dropping the sample rate from 22kHz to 16kHz which got rid of the noise. That's not what I want, but I'll leave it for now, revisit performance once I've refactored a bit more.

Frankly I'm not sure what's causing the performance hit, the additional resources needed to pull this stuff out into a separate (static) class should be minimal. I suspect there's extra comms between the processes, so if I aim to try and get all the input- (/output-)related bits more closely associated with each other, that may help.

Current settings -

```
#define CHECK_STACK true // will make noisy
#define INPUT_TASK_STACK_SIZE 72000
#define OUTPUT_TASK_STACK_SIZE 3072

#define SAMPLERATE 16000 // want at least 22000
```

Which on running produces:
```
esp_get_free_heap_size() = 72372
esp_get_minimum_free_heap_size() = 72356
uxTaskGetStackHighWaterMark(controlInputHandle) = 69132
uxTaskGetStackHighWaterMark(outputDACHandle) = 1960
uxTaskGetNumberOfTasks() = 14
```

**2021-02-11**  *version chatterbox_1.1.0*

JTAG connections : 
```
GPIO14 => JTAG TMS
GPIO12 => JTAG TDI
GPIO13 => JTAG TCK
GPIO15 =>JTAG TDO
```

https://www.esp32.com/viewtopic.php?t=1887

https://hydrabus.com/wp-content/uploads/2016/10/esp32-devkitc_with_FTDI_C232HM_DDHSL-0_JTAG_Debug.jpg

**2021-02-10**  *version chatterbox_1.1.0*

I've make a start on the hardware for a version of Chatterbox without all the controls, [Headless Chatterbox](https://dannyayers.wordpress.com/2021/02/07/headless-chatterbox/).

Is a good opportunity to go back over notes here. 

I don't seem to have made any notes about the MIDI interface, grrr.

But I think I just followed the MIDI spec circuit and added a 2N7000-based level shifter (bidirectonal-mosfet-level-shifter.png). Which, looking back, doesn't appear necessary. The MIDI in goes to an opto-isolator. The UART is on the other side of it, I think this should work, just power the opto-isolator transistor from 3.3v.  

https://hackaday.com/2016/12/05/taking-it-to-another-level-making-3-3v-and-5v-logic-communicate-with-level-shifters/

**2021-02-07**  *version chatterbox_1.1.0*

Blimey, time flies.

strange git merginess, working across machines.

**2020-12-14** *version chatterbox_1.1.0*

I went back to the calibration stuff. I made a lot of silly mistakes thanks to getting variable names mixed up, but then got a sweep tone basically working. *But* it had a lot of crackly artifacts. These were also present on the normal channel, indicating the timing was getting messed up somewhere.
I briefly tried a timer interrupt - [this example](https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/) appeared to work ok, but then when I tried to apply it to the sweep tone, it repeatedly crashed.
Finally I modified the very last bit of code, doubling the number of ops on this thread and inserting the calibrateIncrement() there:
```
// Pause thread after delivering 128 samples so that other threads can do stuff
if (frameCount++ % 128 == 0) // was 64
{
   calibrateIncrement();
   vTaskDelay(1); // was 64, 1
            //  plotter.Plot();
}
```

This worked! 

Right now I've got another value incrementing with every sweep (calibrateTestValue) and have it hard-wired to set the pitch of the second SVF. Only the value gets overridden pretty immediately.

So I have to figure out how to integrate this stuff neatly. I guess using the event-handling bits (as used by MIDI etc) would make the most sense. But I've forgotten how I set that up. I guess next I'll have another look at the MIDI bits then come back to calibration.


**2020-12-01** *version chatterbox_1.1.0*

In the process of setting up the calibration stuff I discovered I'd left the bit of the refactoring I was on a few weeks ago unfinished. I hadn't moved the LarynxWavetable out to it's own class.
After doing that, I got a stack overflow on the ControlInput task. I spent ages looking for a recursive loop.
I had tried upping the available stack on that task x4, but that didn't help.
Finally, I tried monitoring stack level, and for that I upped the available stack to 64k.
It worked! And told me it was using 61k.

I don't immediately see how the other changes I made can have kicked up the stack so much, but now at least it's working and I've a better chance of fixing any mistake.

Hmm, the issue is somewhere around the calls to LarynxWavetable::init(patchbay) inside the ControlInput task (which are used to adjuct the larynx waveform). Leaving those out with low stack available caused the error. But now after commenting them out but leaving the availability high it's still saying it's using ~61k. I'm confused.

I think I'll leave that bit as-is for now, get on with calibration thing.

Serial.print("LarynxWavetable::init HWM = ");
Serial.println(uxTaskGetStackHighWaterMark(NULL));
vTaskDelay(10);

**2020-12-01** *version chatterbox_1.1.0*

Hah! While adding some sweep code for calibration/overflow testing, discovered I'd made a silly mistake in the implementation of wavetables. The pointer through the wavetable which should have been a float was an int. D'oh!

Intellisense broken...

**2020-11-27** *version chatterbox_1.1.0*

Took a lot of hunting but I was able to fix the include path problem easily by manually adding the missing bit to `platformio.ini` :
```
build_flags = -Ilib/Observer
```
I haven't checked the MIDI etc, but the core bits appear to be working, as well as I've had them.

Typical - just settled down at desk, postie visits, dog escapes...
A fence repair later.

So. Lots to do. I'm champing at the bit to try new things, but first I want to refactor the current setup a little more and make everything work a little better - ie. as good as possible without making any big changes to algorithms.

First priority, right now, on some settings the level *somewhere* is overflowing. I found a very good article about this kind of issue: https://www.dsprelated.com/showarticle/532/overflow-i-had-too-much-to-add-last-night

Aha! Having a good break was a good idea. I can see several places where I have statements of the form:
``` float sibilants = (s1 + s2 + s3) / 3.0f;```

Adding first then scaling to prevent overflow!

Oh wait...I'm using floats, aiming for the range -1...1. So adding first then scaling is fine. This should be a lot easier than the things in the link above that deal with integers.

Hmm, CALIBRATE!
Pseudocode:
```
gain = 0
overflow = false
while (!overflow){
	gain++
	filter.setGain(gain)
	for freq = min to max
		...
		y = filter(x)
		if y > 1 or y < -1 : overflow = true
}
return overflow

	

**2020-11-26** *version chatterbox_1.1.0*

Returning to the project after a break. 

I did some more refactoring which I haven't written up. I *think* the reason I was having difficulties pulling out classes previously was that there was one or more serial print statements I'd forgotten about, hidden from immediate view.

Now, when trying to compile again, I run into :
```
In file included from lib/Generator/LarynxWavetable.cpp:4:0:
lib/Processors/Patchbay.h:6:36: fatal error: EventReceiver.h: No such file or directory
```
This file is on the path.

I've a feeling I ran into this before but annoyingly can't see any note relating to it.

The compile (using `pio run -v`) does warn of circular dependencies, but I haven't (yet) seen anything obvious.

This was interesting:
[[
	I have found one way to get circular dependencies:

Generate a DOT file which describes a #include dependency directed graph using cinclude2dot.pl Perl script.

./cinclude2dot.pl --src path_to_include_dir graph.dot

Decompose directed graph into strongly connected components (circular dependencies):

sccmap -v graph.dot
]] https://stackoverflow.com/questions/9885656/how-can-i-find-circular-dependencies

workspace.code-workspace json
changed
"command": "msbuild",
"command": "build",


For C++ source files, IntelliSenseMode was changed from "clang-x64" to "gcc-x64" based on compiler args and probing compilerPath: "/home/danny/.platformio/packages/toolchain-xtensa32/bin/xtensa-esp32-elf-gcc"


**2020-08-06** *version chatterbox_1.1.0*

Last week, quite a major milestone, got MIDI out doing something as well. That's kinda the hardware done. 
But only this morning I swapped the internal amplifier for a PAM8403 module, is much better.

Also, quite a priority : I need to see if there's anything I can do in hardware to reduce noise from pots. I'll try this on a separate ESP32 module so I'm less likely to break anything.

NOTA BENE : I think sometimes when it's made a bad noise for no apparent reason, I think it could be some activity on the serial line that isn't visible. Try Upload without Monitor.

I've been moving variables out of global scope into a class called Patchbay, as public members.
I've moved the MIDI stuff to it's own class, MIDIConnector, which communicates with the main classes through dispatch/receive bits on Patchbay. Getting neater!


**2020-07-23**   *version chatterbox_1.1.0*

Ok, finally got MIDI input working using:

https://github.com/FortySevenEffects Arduino MIDI Library v5.0.2

with 

GPIO 21 [I2C SDA] - MIDI rx
GPIO 22 [I2C SCL] - MIDI tx 


https://en.wikipedia.org/wiki/MIDI_tuning_standard

**2020-07-20**   *version chatterbox_1.1.0*

I've been trying to get MIDI in & out working.

I had a little annoyance in that I'd left these free:

GPIO 21 [I2C SDA]  ...(((DISPLAY?)))
GPIO 3 [UART 0 RX] ...(((MIDI IN)))
GPIO 1 [UART 0 TX] ...(((MIDI OUT)))
GPIO 22 [I2C SCL] ...(((DISPLAY?)))

Not for the first time, I hadn't read enough manual. 
I tried some direct I/O using a UART lib...it didn't work.
Apparently UART0 is used by the ESP32 card's USB/serial so is effectively out of bounds. I haven't found a definitive statement but consensus seems to be that the corresponding pins are probly hardwire to the card's USB subsystem.
Boo!

But the ESP32 has 3 UARTs and they can be assigned to any pins via the internal multiplexer. UART2 is used (by flash?) but UART1 should be free.

Rather than spending more time on addressing UART directly, I decided to go straight to MIDI.

The prefered MIDI lib appears to be this one:

https://github.com/FortySevenEffects Arduino MIDI Library v5.0.2

I tried EspSoftwareSerial, but I got opaque errors. 

More success with HardwareSerial.

**2020-07-09**  *version chatterbox_1.1.0*

Few steps forward, not got around to noting down...

I've tweaked the larynx waveform generation algorithm to better approximate what is found in the literature.
The up-ramp is shorter for more pulsy waves, going to more balanced at triangle end.
Also running it through the softClip filter to round off the corners a bit.

![improved-larynx-1](https://github.com/danja/chatterbox/blob/master/media/improved-larynx-1.png)

![improved-larynx-2](https://github.com/danja/chatterbox/blob/master/media/improved-larynx-2.png)

Also moved the control input thread code into it's own class (though still in the chatterbox_1.1.0.cpp file).


**2020-07-04**  *version chatterbox_1.1.0*

The code was giving me a headache so yesterday had a go at improving the internal amplifier. What I had was one of these LM386 modules:

![LM386 Module](https://github.com/danja/chatterbox/blob/master/media/lm386-module.jpeg)

I'd mounted it upside-down on the main stripboard via SIL sockets, lower left here:

![Inside box](https://github.com/danja/chatterbox/blob/master/media/lm386-in-place.jpeg)

The loudspeaker I've got in there is 10cm, 4 ohm, rated at 2W. But the signal was distorting long before it got very loud. 
I think the LM386 is rated around 250mW. Rooting through my components, I've got some BC109s (NPN) and BC179s (PNP). Complementary, both in little TO-18 metal packages. Each rated 600mW. So I found a textbook amplifier, tweaked a little for the components I had:

![3-Transistor amplifier](https://github.com/danja/chatterbox/blob/master/media/simple-transistor-amp.jpeg)

I soldered this up on a bit of stripboard last night, just tried it out. Definite improvement, so I fitted it:

![Transistor amplifier](https://github.com/danja/chatterbox/blob/master/media/simple-transistor-amp-board.jpeg)

![New amplifier in box](https://github.com/danja/chatterbox/blob/master/media/simple-transistor-amp-in-place.jpeg)

Because all the connectors between the PCBs are made with plugs & sockets, all I had to do was drill 4 little holes in the box to mount, total time around 10 mins.

The level is still well below the volume the speaker can handle. (I did try tweaking the resistor values, but the current ones seem optimal). I would guess it's probably now getting to around 500mW.

It did occur to me that this might have been from overloading 1-transistor mixer I'd put in. It wasn't (because of the easy connectors, took literally 2 mins to check). 
This mixer is pretty much redundant, I guess I went for overkill on isolating the signal paths. But it's harmless so leaving in for now. This is the circuit:

![Mixer circuit](https://github.com/danja/chatterbox/blob/master/media/mixer-circuit.jpeg)

It's on the middle circuit board in the photo above.

I reckon there are 2 limiting factors on the amplifier: one is just the components I have in stock. The other is the power supply - I'm currently using an external standard lowish current 9v DC power adapter. There's room in the box for an internal PSU, and I have the parts, but am not really motivated to put one in right now. 
A better amplifier (module?) and *perhaps* a less noisy supply for the ESP32/DAC would be desirable, but not a priority. Back to the code...

**2020-07-02**  *version chatterbox_1.1.0*

After a lot of I integrated the earlevel.com wavetable code. Made a horrible noise, predictably - it's quite sophisticated. Pulled it out again. Useful exercise though, maybe.

Tried generating sine in real time, noisy.

**2020-06-30**  *version chatterbox_1.1.0*

Other little tasks while pondering wavetable issues.

Misc. links:

smoothing noisy pot signal :
https://stackoverflow.com/questions/10930732/c-efficiently-calculating-a-running-median

flash partitions:
https://github.com/espressif/arduino-esp32/issues/1476

https://stackoverflow.com/questions/17775066/c-c-performance-of-static-arrays-vs-dynamic-arrays

VIRTUAL ANALOG OSCILLATORS
https://ccrma.stanford.edu/~juhan/vas.html

THREEDEE : REAL-TIME 3D SOUND PROCESSOR
https://ccrma.stanford.edu/~juhan/threedee.html

https://www.earlevel.com/main/category/digital-audio/oscillators/wavetable-oscillators/

cheapish ADCs
https://www.audiophonics.fr/en/devices-hifi-audio-adc-c-6407.html

http://www.icstation.com/pcm1802-24bit-audio-stereo-converter-decoder-amplifier-player-board-p-13557.html

https://www.aliexpress.com/w/wholesale-pcm1802.html

debugging
https://docs.platformio.org/en/latest/tutorials/espressif32/arduino_debugging_unit_testing.html

To Hell With Setters and Getters
https://medium.com/@Jernfrost/to-hell-with-setters-and-getters-7814e7b2f949

muzines.co.uk - electronic music magazines


**2020-06-27**  *version chatterbox_1.1.0*

Another little hitch in the Great Refactoring. My ignorance of C++ is the common issue.

I'm trying to move the wavetables (currently 3: larynx, sine & sawtooth) out to objects but keep running into a problem where the output develops a glitchiness. 
I think yesterday's experiments showed that it wasn't gross memory usage that was causing the problem and that the values were getting their right values.

TODO: I should have a look at calculating the values in real time rather than using table lookup, reading about it suggests that this may be more efficient, at least for the simple functions. But I do want the option to use wavetables (definitely in a later sound machine with granular synth features).

* moving things to static
* using std:array
* playing around with getting values vs. references

* moving object construction to the output thread
got continuous reboots - d'oh! - the memory allocation for output thread presumably too low. Upped from 2048 to 8192


https://github.com/espressif/arduino-esp32/issues/1476

**2020-06-26**  *version chatterbox_1.1.0*

Sawtooth & sin, arrays and objects, tablesize 2048, noisy
esp_get_free_heap_size() = 132072
esp_get_minimum_free_heap_size() = 132064
uxTaskGetStackHighWaterMark(controlInputHandle) = 1048
uxTaskGetStackHighWaterMark(outputDACHandle) = 876
uxTaskGetNumberOfTasks() = 14

sin object, arrays, tablesize 2048 noisy
esp_get_free_heap_size() = 140296
esp_get_minimum_free_heap_size() = 140292
uxTaskGetStackHighWaterMark(controlInputHandle) = 1032
uxTaskGetStackHighWaterMark(outputDACHandle) = 816
uxTaskGetNumberOfTasks() = 14

sin object, saw array, tablesize 2048 noisy
esp_get_free_heap_size() = 148648
esp_get_minimum_free_heap_size() = 148640
uxTaskGetStackHighWaterMark(controlInputHandle) = 1048
uxTaskGetStackHighWaterMark(outputDACHandle) = 876
uxTaskGetNumberOfTasks() = 14

sin & saw objects, no arrays
esp_get_free_heap_size() = 148620
esp_get_minimum_free_heap_size() = 148616
uxTaskGetStackHighWaterMark(controlInputHandle) = 1048
uxTaskGetStackHighWaterMark(outputDACHandle) = 876
uxTaskGetNumberOfTasks() = 14

esp_get_free_heap_size() = 148480
esp_get_minimum_free_heap_size() = 148472
uxTaskGetStackHighWaterMark(controlInputHandle) = 1048
uxTaskGetStackHighWaterMark(outputDACHandle) = 876
uxTaskGetNumberOfTasks() = 14

tablesize = 1024, objects only, noisy
esp_get_free_heap_size() = 160772
esp_get_minimum_free_heap_size() = 160764
uxTaskGetStackHighWaterMark(controlInputHandle) = 1048
uxTaskGetStackHighWaterMark(outputDACHandle) = 876
uxTaskGetNumberOfTasks() = 14

tablesize = 512, objects only, noisy
esp_get_free_heap_size() = 166896
esp_get_minimum_free_heap_size() = 166892
uxTaskGetStackHighWaterMark(controlInputHandle) = 1048
uxTaskGetStackHighWaterMark(outputDACHandle) = 876
uxTaskGetNumberOfTasks() = 14

tablesize = 512, arrays only, clean
esp_get_free_heap_size() = 166964
esp_get_minimum_free_heap_size() = 166960
uxTaskGetStackHighWaterMark(controlInputHandle) = 1048
uxTaskGetStackHighWaterMark(outputDACHandle) = 876
uxTaskGetNumberOfTasks() = 14

**2020-06-19** *version chatterbox_1.1.0*

Thing got into too much of a tangle after splitting of the output thread. After quite a frustrating week, did a reset back to back before then.
The last couple of days spent splitting off just the switches into a separate class (Switches) that wraps a std:array containing the switch objects. I got very stuck again, despite it seeming a trivial refactoring. This time it was just down to my ignorance of C++, I needed the & here:

Switch& getSwitch(int switchN);

So, that's back to working (badly) as it was before. But next I should be able to pull out the pots in the same fashion, keep splitting things up but piano piano this time.

I didn't finish the sentence the last time. The other algorithm that looks very interesting is one from a short-lived Yahama box (SPDT or something), called formant synthesis. It is and isn't formant synthesis. A bank of (8?) 'operators' that are each an oscillator & a bp filter, plus a bank with noise + bp. Each bit has envelope control. They are mixed in parallel. The reviews say it produces voice/natural instrument kind of voices. No idea how I'll map to the controls, may use a pair of push switches to effect a mode change. 

Also read up a fair bit more or granular synthesis, I really want to play with that. Occurred to me that applying a sorting algorith to granules might be interesting. But save that for a later project (I have an Arduino Due waiting in the wings).

**2020-06-08** *version chatterbox_1.1.0*

I was going softly softly through refactoring but then yesterday got a bit impatient, split off the whole of the output thread (OutputDac) into a separate class/file. Which split off lots of the variables etc. I did this back to front! Should have pulled out the variables first. But now I've started, continuing this way, at the moment trasferring the variable definitions into global namespace in ChatterboxOutput.h.

TODO : the **mel scale** looks like it could be a good idea for mapping between physical values and frequencies, it aims to match perceived variations in pitch. Is a fairly straightforward log mapping.

TODO : another thing for later. While reading up on FM synths I've come across several different algorithms that might be good for channel 2.
There's the operator-based thing of DX7, plus one 

**2020-06-02** *version chatterbox_1.1.0*

I'd already printed out Klatt 1979 but clearly hadn't read too closely. It does quite clearly give the algorithm for the formant filters (resonators). After skimming the [source of synthv1](https://github.com/rncbc/synthv1/blob/master/src/synthv1_formant.cpp), there it appears the Klatt algorithm has been used with values taken from [CSound Formant Values](http://www.csounds.com/manual/html/MiscFormants.html). 
Synthv1 features 5 resonators preset with 5 vowel sounds for 5 different vocal ranges. Each resonator has 3 parameters (frequency, amplitude, bandwidth). What's not obvious is how these are varied in use, but I've got the plugin installed on music room computer, must have a play later.

Chatterbox is limited to 6 analog values, so clearly I'll have to fudge things somehow. I'm still liking the idea of incorporating comb filter(s) somehow to get a better effect with less processing/control. 
Have a pot for pitch and larynx shapes still makes sense, so *perhaps* if I attach 2 resonators to the joysticks, corresponding to the first 2 formants, with a comb filter in parallel on each, tracking freq. Following these with a crude *tilt* filter might get me a lot of the way, and isn't all that far from what I have already.
So this calls for:

* implementing Klatt resonators
* implementing comb filters
* implementing tilt filter (the variable freq & Q lowpass SVF I already have might be suitable) 

But before getting into this more deeply, there's still refactoring to do...

**2020-06-01** *version chatterbox_1.1.0*

Wrapped the SVF into Processor and in process of swapping references.

Fixed a silly sums problem with the amplitude of larynx wave. (Put in a little max/min value monitor, shows values when stressed+destressed buttons pressed together).

There is a little bit of crosstalk in mixer electronics, putting a sawtooth to one input, sine to the other, the sine carries a little glitch at the transition in the sawtooth.
TODO check decoupling caps

TODO The X axis of joystick is wired back to front.

Pulled out the 3rd harmonic formant filters for now (I *think* it sounded better with them).

TODO Want to have a look at :

// synthv1_formant.h - formant parallel filter after Dennis H. Klatt's
//                     Software for a cascade/parallel formant synthesizer
//                     1979 MIT; 1980 Acoustical Society of America.



**2020-05-31** *version chatterbox_1.1.0*

So... I've got the skeleton of messaging worked out, to pass name:value pairs between 'modules'. 

I first need to wrap the generators, filters etc. in Processor to be able to consistently set parameters. 

Ultimately:

Chatterbox.cpp as top level

Controls.cpp? as input thread
Output.cpp? as DAC thread  
Patchbay.cpp? as the wiring 


**2020-05-30** *version chatterbox_1.1.0*

So, proper hardware nearing completion:

![New Front Panel](https://github.com/danja/chatterbox/blob/master/media/front-panel_2020-05-30.jpeg)

The only piece currently missing is the MIDI interface. This *should* be trivial: and optoisolator and a transistor (to level-shift from 3.3v to 5v) attached to the UART I've left free on the ESP32. But software for that will have to wait a while, still loads of work on the core code needed.

These are the internals:

![Internals](https://github.com/danja/chatterbox/blob/master/media/internals_2020-05-30.jpeg)

I first moved all the controls onto the new front panel, then connected these up to the breadboard circuit. Once this was working, I built a totally ESP32 setup on a bit of stripboard. A complication here was that I couldn't find an identical ESP32 module to the one I'd used before, instead tried a different one that appeared to expose all the same functionality, with a slightly different layout. Luckily, it worked!

(I've got it pushing name-value pairs from the control parameters to ESP32 serial interface so is easy to check).

The stripboard was perfect for the wiring from ports on the ESP32 to SIL connectors, into which I plugged the existing connectors from the controls. 
Rather than soldering, for the DAC, PSU and amplifier I used SIL connectors plus breadboard-style hookup wired. 

The power supply I used is a cheap linear regulator module, 9v in (fed from an external PSU), 3.3v out.

One thing I hadn't really thought through was that the DAC has 2 outputs, both of which I wanted somehow going to the loudspeaker and also exposed to the outside world. I'd initially thought of Volume and Mix controls, but while considering how to wire these up I decided it would be easier and probably more versatile to use separate Level A and Level B controls. After potentiometers, these can each go directly go to jack sockets (which I have ready but not yet wired up) as well as to the inputs of a one-transistor mixer. The output of the mixer goes to a little LM386-based amplifier module and from there to the speaker. (I hacked the mixer together on breadboard before transferring to a scrap of stripboard).

The volume out of the internal speaker isn't great before the amplifier distorts, but it's certainly adequate for now. Things are very modular inside so it'll be easy enough to swap in someone more hefty later, if I can be bothered. (The speaker I think is 4W, way more that the LM386 can provide). An internal mains supply would be nice to have too, and there's plenty of space in the box.

I've made it so the USB connector is easily accessible, though have also been looking at ESP32 Over The Air (OTA) updates, ie. WiFi.

The new front panel is an A4 piece of laminated aluminium/plastic intended for signage. It was cheaper than the sheet aluminium I could find on Amazon and turned out to be ideal. I've inkjet-printed labels onto a transparent A4 sheet. This is less than perfect, a little bit floppy (spraymount might have been an idea?), but fine for a prototype.

The box is a wooden one, sold on Amazon for craft purposes. I made a simple frame inside on which to mount the front panel. I was tempted to finish the box with decoupage as intended, but decided spraying it black would be quicker.


Back to the coding.


**2020-05-16**  *version chatterbox_1.1.0*

I got a bit stuck trying to make a nice (template-based) model for Processor with parameters. Had a break from coding, did a load more on the hardware. Front & back panels prepared.
Just implemented the ugly processor.

Hmm, wonder how the same kind of thing has been implemented elsewhere, eg. LV2 plugins...

**2020-05-02**  *version chatterbox_1.1.0*

Still refactoring. Trying to pull out the Web control bits, but need to allow access to values such as pitch, f1f etc. As a stepping stone, pulling out all those values into a Parameters object with getters & setters. Is ugly (and should probably be static) but will hopefully allow me to find a better solution...

Also discovered that there may be a workaround for the no-ADC2 when WiFi is enabled issue. But I think I've now used all the available pins elsewhere...
https://github.com/espressif/arduino-esp32/issues/102#issuecomment-593650746


**2020-04-23** *version chatterbox_1.1.0*

Refactored switches and pots to objects. Code beginning to look a lot cleaner.

I stumbled across [synthv1](https://synthv1.sourceforge.io/) while playing in music room. Old school mono synth in DSP, standalone/LV2 plugin, with an interesting formant filter -

// [synthv1_formant](https://github.com/rncbc/synthv1/blob/master/src/synthv1_formant.h) - formant parallel filter after Dennis H. Klatt's
//                   Software for a cascade/parallel formant synthesizer
//                   1979 MIT; 1980 Acoustical Society of America.

I believe it's implemented as 5x 2-pole filters. The result looks very like a comb.

Which leads me to wonder if a comb filter could be used here. They're trivial to implement, the main questions being how to do the overall shape and set up for the appropriate set of harmonics (odd? I need to re-read notes).

**2020-04-14** *version chatterbox_1.1.0*

Refactoring resuming.

First thing, get it working as-is.

* fix warnings
* add forward references

**SAVE REALLY OFTEN! - IDEALLY FUNCTIONAL**

* rough out simple classes+headers Node, ControlNode, Switch
* parallel all init : constructors etc.
* parallel all value setters
* gradually replace getters

hold toggle now moved over to Switch class

tweaks TODO:
* make Shout attack/decay shorter
* add voice on to nasal switch



**2020-04-13** *version chatterbox_1.1.0*

So...the Atom-based PlatformIO IDE seemed ok, only platform-ide-terminal refused to install. After a couple of hours trying to sort it out, concluded the current version is broken. So went back to VSCode-based PlatformIO.

This took me another couple of hours. It didn't like the imported project and I got weird include path issues. Also the Tasks didn't show up, [this](https://community.platformio.org/t/how-to-get-project-tasks-in-new-workspace/5996/5) sorted that part.

I've got Chatterbox building and uploading on PlatformIO now. Have also separated out NoiseMaker and Shapers into separate classes (mostly to give me some simple C++ to start with).

Now thinking about turning some other things into C++ objects. The overall system I reckon makes sense as a kind of flow pipeline, ie. graph, with nodes (Input and Output) and arcs (connectors).

I'm starting with the switches, they seem least demanding.

I want to get some unit tests in place. I don't need the ESP32-specific stuff yet, most of this I can build up native (desktop gcc). One little issue was the Arduino String class which is non-standard but I use it throughout. Including Arduino.h led to a bunch of other problems, but it seems just including WString.h seems to do the trick. [Notes on Arduino's String](https://hackingmajenkoblog.wordpress.com/2016/02/04/the-evils-of-arduino-strings/).




**2020-04-12** *version chatterbox_1.1.0*

Refactoring time. 1116 lines of code!

First thing, get it working as-is.

* fix warnings
* add forward references

* (extract header files)

**2020-04-11** *version chatterbox_1.1.0*

So far I've been using the Arduino IDE. But that's meant I've got all the code (except the SV filter) in a single huge file.

Looking at the external control from WebSocket, this will be overly difficult given the way things currently are, there are lots of interactions needed. Time to **refactor**.

As a first step, I'm going to try moving to PlatformIO on VSCode.

Spent hours, couldn't get it to work. Moved over to PlatformIO on Atom.

This had plenty of problems with install, but I believe I've now got the IDE working, now to get the code working...

~~Configuring PlatformIO on VSCode~~

* imported chatterbox_1.0.21 into PlatformIO (Import from Arduino), it saved it as a project folder under /Documents. This I renamed chatterbox_1.1.0 and moved to ~/chatterbox/src/
* renamed chatterbox_1.1.0.ino to chatterbox_1.1.0.cpp and imported it as existing project

In C/C++ configurations -> Include path

${workspaceFolder}/**
~/arduino/**
~/chatterbox/src/lib/**
~/esp32/**

**2020-04-10** *version chatterbox_1.0.20*

**UPLOAD WON'T WORK IF SING IS ENABLED**

Yay! Got pitch control from browser over WebSocket working!

This will need a *lot* of work to sort out to get all the controls working, but it is a little breakthrough.


**2020-04-09** *version chatterbox_1.0.19*

Had a few days away from code but have been reading, also setting up the box this thing will ultimately live in.

The todo is currently pretty much the same as previously, but with a little more knowledge. The first thing here is important, I should take a look soon. But the priority is getting placeholder code in place for Creak, then doing more on web interface.

1. pitch change not smooth - this crept in at some point, it hops between tones. I suspect it's a silly mistake somewhere. Now I've got the additional wavetables it should be straightforward to trace, checking 'scope displays.
2. creak/shout/growl noise modulation
3. spectral tilt
4. sub-octave for creak/shout/growl
5. web interface
6. MIDI

A big issue with the noise modulation is that using white (or even pink) noise directly seems to feed too much HF through. But I've stumbled on another algorithm that looks like it has potential - [Perlin noise](https://en.wikipedia.org/wiki/Perlin_noise). It's usually used for CGI graphics, creating textures, but an audio version exists. There's a C++ implementation in the [Andes synth](https://github.com/artfwo/andes) that might be usable without too much effort.

It also occured to me that I could make a different kind of 'LF' noise by starting with white (or pink) noise and interpolating between points, the [Smoothstep](https://en.wikipedia.org/wiki/Smoothstep) algorithm (not unlike the softClip() I have already implemented) hopefully being suitable to cut high harmonics.

Hmm, I wonder if I should wrap all this noise gen stuff into a seperate C++ class...

So...

I've got the interpolated/smoothstep going.

Currently, when it's amplitude modulating I'm calling it growl and enabling with Shout. Level controlled by the pot otherwise used for larynx.

When it's larynx-modulating, I've got it hooked up to Creak.



**2020-04-03** *version chatterbox_1.0.18*

Hmm, this messing with filter gains has led me to think some kind of level metering might be useful, and maybe some kind of automatic level control, and perhaps even compression beyond the softClip() thing.

I think I'll step back from aiming for definitive Creak, Sing and Shout for now, instead adding support for a few more features, figure out how to apply these to the vocal styles later.

This morning's reading, [Waveform recognition in Growl](https://cvtresearch.com/waveform-recognition-in-growl/), suggests to me that it may well be useful to go back to the original laryngeal waveform generation and allow more variation there, I reckon by expanding it to :

* (current) larynx wavetable
* sawtooth wavetable
* sine wavetable
* mix of the three

Ok, implemented. I believe that's working correctly.

The push switch logic isn't perfect and nasal has a looping sound to it. But mostly good, so a version uptick :)

*version chatterbox_1.0.18*

Perhaps add sub-octave? - waveform and/or filters?

If larynx wave isn't in use, maybe use P1 to vary sine/saw mix?

I think I may need 2 separate web interface pages - one corresponding to actual controls, one for mixer & other 'hidden' parameters.

Also, from [Acoustic Features and Auditory Impressions of Death Growl and Screaming Voice](https://www.researchgate.net/publication/262251558_Acoustic_Features_and_Auditory_Impressions_of_Death_Growl_and_Screaming_Voice) there's:

* Jitter expresses fluctution of glottal pulse interval
* Shimmer is a fluctuation of amplitude of glottal pulses
* The harmonic-to-noise ratio (HNR) is the ratio of energies of harmonic and non-harmonic components of the signal

I reckon this is a good prompt for me to look at a low-freq noise source to act as modulator - as I've already started for Creak, but still not got anything useful.

Thirdly,

* spectral tilt - overall freq balance / _ \

- seems a must-have. The implementations I've seen for this seem a bit involved, notably stepwise approximation. I don't think this needs to be particularly sophisticated.

terms to take note of : "Vocal tract resonances R1 and R2, Open quotient, Fundamental frequency, voice intensity, larynx height, lip aperture and spreading"

**2020-04-02** *version chatterbox_1.0.17*

Nasal gone glitchy!

Oh, I tidied up a bit of the switching logic, the glitchiness went away.

Reordered the filter-application bit and added 3rd harmonic tracking filters to f1 & f2. Immediate bad distortion.

Put lower values in setGain() in the filters - made no difference. Looking at the SVF source, it doesn't do anything except in the shelf filters!

So I've put in defines of multipliers corresponding to gain for things (floats). Needs tweaking...

For now a reasonably stable build.

TODO
* **check order of filters**
* update block diagram

possible enhancement : add formants that related to F1 & F2 by tube resonance calculations - having a spectral tilt filter could control ratios (or via direct control)

**2020-04-01** *version chatterbox_1.0.16*

I've switched white noise in SF1, SF2, SF3 for pink, white, blue

de-stress is now also de-Q, drop the Q of all filters

[The Singer's Formant and Speaker's Ring Resonance: A Long-Term Average Spectrum Analysis](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC2671792/) : " trained tenor's voice has the conventional singer's formant at the region of 3 kHz and another energy peak at 8-9 kHz. "

* singing : increased attack & decay; peaks added at 3kHz and 8kHz

* shout : tilt (to boost low); amplitude; distortion?


Read about:

https://www.audiologyonline.com/articles/dynamics-third-dimension-speech-813

* laterals - see nasals-laterals doc
* liquids
* rhotics

----
5. Liquids

a) Liquids are normally voiced (though they are sometimes devoiced or realised as voiceless fricatives e.g. following a voiceless obstruent) - hence clear formant structure, though with less energy than vowels.

b) Laterals have antiformants, though usually less strong than in nasals. For example, in /l/ there is one around 2500 Hz, between the apparent F2 and F3.

c) In both [ɹ] and [l], F1 and F2 are lower than in adjacent vowels: in [l], and laterals in general, transitions very sudden (cf. nasals); in [ɹ], less so.

d) In [ɹ], F3 also falls from adjacent vowels; in [l] this is much less pronounced.

e) Clear and dark /l/ differ in their formant structure; F2 is somewhat higher - about 1500 Hz - for a clear /l/ and lower for a dark /l/.

f) Trills are characterised acoustically by a 'pattern of pulses of closures and openings' (Lindau, 1986). Their spectral structures vary considerably.

----
**Nasal**

From different sources :

1. nasals are characterized by less high frequency energy
than vowels; Little energy above approximately 3500 Hz.
2. F1 in characteristic nasal murmur at 250 Hz;  F1 (sometimes called N1 in nasals) is low in all nasals - typical figures 250 - 300 Hz.
3. Formants much weaker in nasals than in vowel sounds.
4. Nasals have several antiformants. Also, the vocal tract produces an antiformant between 800 and 2000 Hz - the region of F2 for most vowels.

[m] has an antiformant between 750 Hz and 1250 Hz. N2 between 1000 Hz and 1300 Hz. F2 therefore generally rising to following vowels.
[n] Fujimura gives N2 at 1000 Hz and antiformant at 1450 - 2200. On a spectrogram, second visible formant is often N4, at about 2000 Hz, giving similar transitions to alveolar stops.
[ŋ] Main anti-resonance is that above 3000 Hz. theoretically, N2 is at 1100 Hz, N3 at 1900. In practice, N2 is rather weak. There is considerable speaker to speaker variation.

from 1. : fixed LPF at 3500Hz
from 2. : fixed F1 at 250Hz
(from 3. : lower Q on F2
from 4. : flip F1 to NOTCH (N1)

Ok, implemented, not bad, is a bit more nasal-sounding, though notch F1 hardly makes any difference.

Trying F1 notch with Q values from 1 to 20, still not really making any difference apart from a bit of distortion on part of range. I don't expect much effect from notch, but this is virtually nothing.
Trying the biquad filter...

• F1 tends to be low (250-300Hz) and F2 around 2500Hz. In between (where vowels have F2) there is little energy.

- so maybe leave joystick X controlling a bandpass (with lower Q), have notch track midpoint F1-F2?

Wait - reverted to SVF notch, changed freq range (lower) is now more audible. I do want to try tracking notch (or peak?) too...

Tracking didn't make a lot of difference.


**2020-03-31** *version chatterbox_1.0.15*

Added a load of links.

I reckon what I'll do today is put *something* in place for the remaining switches, so I can move back to the web interface.

* nasal push switch : I reckon I'll try simply popping different filter parameters in place for F1 & F2 - the joystick. The docs seem to suggest notch filters with similar slopes as the normal formant bandpasses

* stress push switch - implemented as simply volume boost, need to look at spectral tilt -

* constant tone/sib - done

* croaky - logistic map, isn't any use yet

* shout - need research

* singing - need research, but one thing is how pro singers develop a 3kHz peak... Maybe increase attack/decay time?

**MUST REMEMBER** there are 2 signal outputs. It would be feasible to hook up controls to a totally different set of filters etc, have the output piped this way, mix done analog externally

Had a glitch in uploading. Put a 10u cap between EN and ground, it worked once, but then didn't run. Now seems ok after removing cap...

Left it in a slightly odd state, toggle 3 now asigns a LP and a HP to the joystick through a parallel chain.

Is difficult to experiment with at the moment because of the time taken to upload changes.
I must have a think about using web interface to upload patches.

First, get web interface working for current configuration...

**2020-03-30** *version chatterbox_1.0.15*

Bit more reading, found some interesting stuff relating to the larynx waveform (see [Links](links.md) -> speech). Certainly looks worth playing with, maybe relevant to singing & shouting voices. *If* this makes a significant difference, then it might be worth making more wavetables...

Yesterday's switch logic tangle was incredibly easy to sort out, simply added switchType[] - PUSH 0, TOGGLE 1


**2020-03-29** *version chatterbox_1.0.15*

Started playing with a chaotic noise generator ([Logistic map](https://en.wikipedia.org/wiki/Logistic_map)), flipping between wavetable voice and this using toggle switch 2 and overloading pitch pot to change constant in logistic equation.

To allow this without making things too messy I've introduced the distinction between ACTUAL_POTs and VIRTUAL_POTSs. The virtual ones support all the same parameters as pot controls did before, assigning them to actual ones as required.

But...my concentration's not been very good today. Plagued by silly bugs. Just noticed the switch logic isn't wired properly, isn't distinguishing between push and toggle switches properly.

Hey ho, sort out tomorrow, hopefully have a clearer head.

**2020-03-28** *version chatterbox_1.0.14*

I've implemented stressed & de-stressed push buttons. I'm really not sure about these, the de-stressed especially doesn't seem very useful. But leave them for now.

I've also done the constant tone/sib toggle. now called **Hold**, when off the push switches behave as before, push to play. When on, the push switches toggle value on every push. This is really good! I can see it being very useful. A keeper.

I also had a little play with modulating the voice signal (pitch then amplitude) with a pink noise generator. I couldn't get anything resembling a croaky voice, but have a few more ideas to try.

Now I need to do a bit more research for the other added controls.

----

I reckon I need the 3 sibilant buttons noises to enter the processing chain later, after F1 & F2.

**2020-03-27** *version chatterbox_1.0.13*

*A convenient way of configuring the system on the fly might be to supply an Inputs x Outputs connection matrix, akin to the patchboard on the EMS Synthi. One for later...*

----

Slightly impulsively added 3 push and 4 switches.

Took a game of musical chairs to allocate, but all now working, got values going to serial.

Provisionally, for the push switches:

* stressed - boost volume
* de-stressed - cut volume
* nasal - parallel filter chain, notes below

For toggles:

* constant tone/sib
* croaky/[Vocal fry](https://en.wikipedia.org/wiki/Vocal_fry_register) - pink? noise mod
* shout - need research
* singing - need research, but one thing is how pro singers develop a 3kHz peak... Maybe increase attack/decay time?

FRY : try x[n+1] = kx[n](1-x[n]) modulating larynx? tableStep?

It would be nice to have a combination that produces Death Metal voice.

Other possibilities are male/female toggle, though the changes should already be covered by existing controls.

The toggle switches could allow overloading of the functionality of the pots, but I want to go easy on this, could make things too complicated.

This leaves me with:

32 GPOI 21 [I2C SDA]  ...(((DISPLAY?)))
33 GPOI 3 [UART 0 RX] ...(((MIDI IN)))
34 GPOI 1 [UART 0 TX] ...(((MIDI OUT)))
35 GPOI 22 [I2C SCL] ...(((DISPLAY?)))

"Pins 34, 35, 36, 37, 38 and 39 do not have internal pull-up or pull-down resistors, like the other I/O pins." - [Reddit](https://www.reddit.com/r/esp32/comments/7b8ud4/possible_to_skip_the_10k_resistor_button/)

**2020-03-26**

This morning I did a bit of reading around phonemes, planning ahead. Found lots of tables of formant frequencies for vowels, a lot less similar material for consonants. I've been trying to compile the different material into an RDF representation (Turtle) so I can then run SPARQL against it, pull out the bits I need.

On a skim of a few papers, the formant freqs given seem to vary a lot. What might be interesting is to pull values from my own voice, robot danny.

I had a quick look at how to do analysis with Octave (/Matlab), but then stumbled on Praat. It's really good, though user interface is awful...

Ok, I just had a more thorough look at analysing speech. In Audacity I sampled me saying "foooood" and trimmed it to "oooo".
The material is in chatterbox/analysis/

**Audacity**
Spectrogram, Hann Window, eyeballing peaks.

83Hz -7.2dB
171Hz -8dB (flattish curve)
276Hz -1.2dB
2085Hz -45dB
3232Hz -49dB

**Octave LPC Formant Extraction**
chatterbox/analysis/food-analysis.m, based on [Formant Estimation with LPC Coefficients](https://it.mathworks.com/help/signal/ug/formant-estimation-with-lpc-coefficients.html)
Needed chatterbox/analysis/lpc.m - found [around the Octave site](https://savannah.gnu.org/patch/?8575) but hasn't made it into the distro yet.

Freq     BW
246Hz    10Hz
8567Hz   457Hz
16296Hz  225Hz
19413Hz  651Hz

**Praat**
I'm really not very clear on a lot of what I'm looking at here, but the most direct thing seemed to be to autocorrelation, then go for the formants menu. I've dropped a couple of screenshots into chatterbox/analysis/

Pitch 91Hz
F1 205Hz
F2 283Hz
F3 2238Hz
F4 3174Hz

And as a sanity check, here are some values I got from the literature:

F1 250Hz
F2 595Hz

F1 440Hz
F2 1020Hz
F3 2240Hz

F1 290Hz
F2 680Hz
F3 2320Hz
F4 3150Hz

Hardly any agreement!

**Command line interface** added to TODO list. This is in effect already covered as a testing/precursor to the web interface, use WebSocket for comms. But having it read text would be a nice target.

----

One hardware change I'm tempted to make is the addition of another switch for nasal sounds. This would turn on the voiced signal, maybe have a (flattish) overall fixed filter but then also take the value(s) from joystick and use those to mod the characteristics of totally different filter(s).
 "Nasals have several antiformants. Little energy above approximately 3500 Hz. Also, the vocal tract produces an antiformant between 800 and 2000 Hz"
 [Consonant acoustics](http://www.phon.ox.ac.uk/jcoleman/consonant_acoustics.htm)
 So flip the bandpass filters of F1, F2 into bandstops?

 See also [Nasal Acoustics Notes](https://www.phonetik.uni-muenchen.de/~hoole/kurse/akustikfort/nasalacousticsnotes.pdf).

This is a point though - there are likely to be free digital GPIO pins left available, why not wire these to further switches?
Possibilities:

* nasal as above
* stressed : up the amplitude
* roughness : pink noise modulate the pitch
* de-stressed : down the amplitude
* squillo : "Studies of the frequency spectrum of trained classical singers, especially male singers, indicate a clear formant around 3000 Hz (between 2800 and 3400 Hz) that is absent in speech or in the spectra of untrained singers." [Wikipedia : Formant]

(I like the first 3, this would make 8 switches in total, tidy :)

Another point here is that any of the controls can be overidden, so when say roughness is enabled, the larynx control could flip over to controlling the level of the roughness.

Toggle switches could be used for something(s) too...

**2020-03-25** *version chatterbox_1.0.11*

I've got values read from pots (ADCs) being written to WebSocket, picked up by node WS client. Format is :

...
f1f:831
f1f:773
f2f:1810
f2f:1725
f1f:732
...

The client crashes with fast changes.

I also had to set the resolution for change detection to be 32 (/4096) because of the massive variation just to noise (?).

This needs work!

I messed up a doc-global search & replace (ignore case), had to go through line-by-line to properly fix. Took the opportunity to do a bit of tidying up, added some more comments.

**2020-03-24**

Aside: thinking about handling manual control inputs, is has occurred to me that interrupts might have been appropriate, only for the obvious kind of implementation is a non-starter due to the noise the controls pick up. It may be that something event-driven might be a good idea for performance purposes, but right now that seems like unnecessary complexity - stick to polling.

**2020-03-23** *version chatterbox_1.0.10*

**Boo!** I wasted hours trying to get nginx working as a WebSocket proxy. The whole idea was for it to be a timesaver... First big problem was I'd forgotten that I had nginx running on this machine in a Docker container, so when I thought I was changing the config I wasn't, d'oh! But even once I'd realised, I couldn't get a config to work, so I've given up on that.

**Hooray!** I used the demo code from [this tutorial](https://techtutorialsx.com/2018/08/14/esp32-async-http-web-server-websockets-introduction/) for a minimal WS server on the ESP32, Python client. Worked first time!


**2020-03-18**

- [x] 1. node.js WebSocket client & server on port 8080

**bleahhhh!**

- [ ] 2. nginx on 80 proxying WebSocket to 8080
- [ ] 3. node.js WebSocket server on 8080, client on 80
- [ ] 4. Python client to 80
- [ ] 5. minimal browser client to 80
- [ ] 6. work out message shape
- [ ] 7. mock Chatterbox on node.js WebSocket server on 8080
- [ ] 8. Chatterbox browser client talking to mock
- [ ] 9. minimal ESPAsyncWebServer WebSocket on ESP32
- [ ] 10. ESPAsyncWebServer WebSocket on ESP32 to Chatterbox internals

**2020-03-17** *version chatterbox_1.0.10*

Ok, so to some extent the choice of comms with the Chatterbox is quite arbitrary. But webiness is *good*, WebSockets is nearby and supported by [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer), which I'm already using.

So...

* I found a [tutorial using ESPAsyncWebServer for WebSockets](https://techtutorialsx.com/2018/08/14/esp32-async-http-web-server-websockets-introduction/), has code for a simple WebSockets server and some (Python) client code.
* A [WebSocket server on node.js](https://github.com/websockets/ws) looks straightforward.  
* The [nginx documentation for WebSockets proxying](https://www.nginx.com/blog/websocket-nginx/) looks pretty good (I anticipate using port 80 for everything, this may be useful).

- should get me started.

**2020-03-16** *version chatterbox_1.0.9*

Spent hours yesterday working on web interface, sliders based on JQueryUI. Working out the UI just running a laptop-local nginx server.

I'm very out of practice with this stuff and was trying to adapt demos to what I wanted. Could hardly even remember any CSS. Eventually gave up.

Today, started again on it, bit more thought before action. Dynamically building the sliders via JQuery. Aah, progress.

Got the sliders close to what I want, so now trying to hook into place with the server on the ESP32. Oops, **ESP32 SPIFFS doesn't support directories**. D'oh! Oh well, just use online-remote JQuery/JQueryUI files for now.

I've got some very basic comms going on now, but depends on reloading the page to update values. Need to look at polling or some other protocol to avoid this.

Also my lovely layout appears to have broken, boo!


**2020-03-14**

Yay! Very crude, but Web interface now showing control values :

![Web Interface 1](https://github.com/danja/chatterbox/blob/master/media/web-interface-1.png "Web Interface 1")

**2020-03-12** *version chatterbox_1.0.9*

So I've got [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) working ok, reading static files from SPIFFS. This also includes a minimal templater, which is very convenient.

So next I guess is to publish all the settings & control values to a Web page.

The standard way of configuring network settings in a system like this seems to be to use a captive portal. Example code is available, something to look at later.

**2020-03-11**

Ok, I want to play around with parameters. But the edit-compile-upload cycle is very tedious. So I reckon it's time to look at the Web interface.

[ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) (with [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)) looked promising. The *Hello World!* using it described [here](https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/) worked on first attempt!

I keep forgetting that the ESP32 I'm using is dual-core... I currently have the Chatterbox input control managed by one of these, the signal generation/filtering the other. I want the sig gen part as fast as possible, so I need to get the Web server running as a thread on the input core. Time to RTFM.

After a lot of playing around, decided for now to keep it simple and forget about the core question unless it becomes an issue.

Also, for now I've got network config hardcoded.

**2020-03-10** *version chatterbox_1.0.7*

Made a lot of progress in the past few days, despite myself. Getting impatient, tried to do too much at once, made loads of silly mistakes, had to backtrack a long way to fix.

But basically all working!

Performance appeared to become an issue, weird artifacts on wave generation, so I temporarily moved sample rate from 22050 down to 16000, which fixed the worst. The SVF I'm using uses doubles internally. Flipping them all to floats has allowed me to go back up to 22050, but it's a nice warning that I'm getting close to the limit.

There is an awful lot of tweaking to do, getting the timing & filter parameters optimised.
I may well need to work on simple transfer functions from the pot/ADC values to the filter parameters. I also need to do something to deal with the effects of noise on the controls. As I'm typing it's making occasional popping noises all on it's own.

Given that the source signal is very simple, using a wavetable is rather overkill (and inefficient, the way I have it set up). Inline generation is a definite possibility. But I do quite like the versatility this could offer, as and when it has more music-oriented synth bit (ie. MIDI).

If I've got enough processor performance space, I really want to try adding extra filters with their frequencies locked to multiples of those of formants F1 and F2.
Either way, having the auxiliary filter F3 with pot-variable freq & Q, as a low pass seems to offer some nice effects.

So, tweaking for good sounds and optimisation is on the cards.

But I also should start thinking about Web & MIDI interfaces. I'm hoping that the hard work for these has already been done for the ESP32.

Once I've got skeleton functionality for those parts working I can get on with shifting the thing of the breadboard and into a decent box. Oh yeah, and build a little audio amp.

**2020-03-06**

NOTES TO SELF: remember larynx wave is recalculated on every change of that pot, source of glitches. Larynx wave is not at full scale

Last night I did a quick check, C's built-in random function does seem to produce reasonable-sounding white noise using :

random(-32768, 32767)/32768.0f

pushed directly to the DAC2 output.

Now seems a good time to look at the envelope-shaping components. I'm thinking of having a simple attack (on switch push), sustain (on switch hold) and decay (on switch release). Looking at the literature 10mS seems to be ballpark time for each of A & D. But what shape curve? I suppose it makes sense to start with linear and see what it sounds like.

Lol, that couldn't have been easier! And no need for switch debouncing!

**2020-03-05**

Out of the box, the biquad filter I've tried has stability issues, so now on the lookout for something better.
[SvfLinearTrapOptimised2 (State Variable Filter)](https://github.com/FredAntonCorvest/Common-DSP) based on [this paper](http://www.cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf) seems promising, though I'm struggling to get the levels right.

*(I never learnt C++ properly so there's a lot of guesswork around using 3rd party code, so many different styles...)*

Something else I'm looking at is allowing quite mad ranges on things like the Q but inserting code into the filter chain to keep levels within bounds. The easiest auto level control is soft clipping, for which the tanh function is indicated, and pretty fast approximations are possible : [Approximating hyperbolic tangent](https://mathr.co.uk/blog/2017-09-06_approximating_hyperbolic_tangent.html)

Wow! The combination of SvfLinearTrapOptimised2 and soft clipping is wonderful!
(Might have been as good with the original biquad, may play later).

I've put this type of filter on all controls, saved as *version chatterbox_1.0.5*.

Added documents : [Design Notes](https://github.com/danja/chatterbox/blob/master/manual/design.md), [ToDo list](https://github.com/danja/chatterbox/blob/master/todo.md)

**2020-03-03** : **Milestone!** first formant filter essentially working.

*version chatterbox_1.0.3*


**2020-03-02**

*version chatterbox_1.0.2*

I want to start looking at filter design. To creep towards this I've split out the two channels, so wavetableL[] is pumped to the left channel out of the DAC, wavetableR[] to the right.

Later on I'll need to figure out some kind of general pipeline for

**wavetable/source  -> filters -> DAC**

but for now I've simply got

**wavetableL[] -> [*insert filter here*] -> wavetableR[]**

**wavetableL[] -> DAC_L**
**wavetableR[] -> DAC_R**

as a placeholder for a filter I've got **value = -value**.

I've also tweak the square wave wavetable generator to give it a variable pulse width, controlled by pot 4.
So right now I can get an output like this:

![pulse-inverted.png](https://github.com/danja/chatterbox/blob/master/media/pulse-inverted.png "pulse-inverted.png")

*LATER*

So I had a quick go at coding a biquad filter. There wasn't any difference between input and output. So then I had a look for an existing implementation. Ok, this looked essentially the same as mine...and I got the same lack of effect.

The constructor looks like this: **Biquad(int type, double f, double Q, double peakGainDB)**

where f is actually cutoff freq/sample rate.

D'oh! Silly me. I was turning the pitch pot expecting an effect, but the filter was only happening as a one-shot event at start up. But as I was trying to figure this out, another - significant - d'oh! hit me. Given the dependency of the filter's response on the sample rate, I wanted to find what that was. I had been assuming it was being determined by the delay in my processing. **Nope.** The DAC must be acting synchronously, the max freq is exactly half the sample rate specified for the DAC.

I'm stopping now, too tired to think straight.
But the code version I'm saving now  chatterbox_1.0.3 is funny! Industrial sounds in a box. Well, pulsed farty noises.

I tried putting a buffer on the output datastream, applying filter to it, freq controlled by pot 3. Got something horrendously wrong.

Tomorrow's another day.


**2020-02-29**

For now at least I've settled on using 6 ADC channels (all on ADC1). I've hooked these and all 5 switches up to code, although the only controls that are currently doing anything are:

* Switch 0 : Waveform Switch - larynx/sine/square/sawtooth/triangle
* Pot 5 : Pitch (freq range is around what's needed, low end would never be an issue (10Hz is quite clean), high end it gets to about 8kHz)
* Pot 4 : Larynx Waveshape

The original Chatterbox article suggests the larynx typically produces a waveform like this:

![Larynx Waveform](https://github.com/danja/chatterbox/blob/master/media/larynx-wave.png "Larynx Waveform")

Various papers I've seen online pretty much concur, just slightly more ramp-like and less sharp transitions (hence probably much lower high harmonics). It's also noted that the amount of breath-force on the larynx changes the resulting waveform.I was curious to see what difference changing the ratio of 'flat' and 'peak' sections would make, so I rigged this up to a potentiometer. Interesting, maybe serenditous. The effect does sound like a human-ish vocalisation, so this control *might* stay.

At one extreme, it's just a regular triangle wave:

![Triangle Waveform](https://github.com/danja/chatterbox/blob/master/media/triangle-wave.png "Triangle Waveform")

Towards the other, it's getting more like an impulse train, with the expected broad spread of harmonics (and aliasing when the freq is high).

![Extreme Larynx Waveform](https://github.com/danja/chatterbox/blob/master/media/larynx-wave-extreme.png "Extreme Larynx Waveform")

I've refactored the code a bit, using arrays for the inputs. I've also flipped from using the ESP-IDF style methods & macros to using the usual Android IDE ones, so it's now like :

analogRead(potChannel[pot])

rather than

adc1_get_raw(ADC1_CHANNEL_0)

**Issues**
* There is still a significant problem with aliasing on the signal harmonics, but I'm not going to worry about that right now. I want to get the filters set up to get a clearer picture of what will be required.
* The switches are seriously bouncy! Putting an RC filter on them doesn't seem to make a great deal of difference. But again, I'm not going to worry about this until a bit more of the meat of the code is done.

**2020-02-28**

Wondering about inputs.

Have been reading [Klatt 1979] : "one might wish to vary as many as 20 of the 39 parameters to achieve optimum matches to an arbitrary English sentence".

The original Chatterbox has :

* slider pot (pitch)
* joystick (filters)  
* 5 switches
	* voicing
	* whisper
	* **S**alt
	* **F**ish
	* **Sh**ips

On prototype hardware v1, for input I've (fairly arbitrarily) provided :

* 5 rotary pots
* joystick
* 5 push switches

Now, something I'd skimmed over is the ESP32 I/O. First impressions is it's virtually unlimited, but looking closer, not all the chip pins are exposed on the dev boards, and then pretty much everything's multiplexed.
Most annoyingly, ADC2 is out-of-bounds when WiFi is enabled. I want this thing to be HTTP-accessible (ideally both for programming the DSP config and for using the controls as a UI).
*"Note that GPIO6-11 are usually used for SPI flash. GPIO34-39 can only be set as input mode and do not have software pullup or pulldown functions."*

Which only leaves 6 channels on ADC1.
*Potentially*, I could multiplex the inputs, but that would detract from immediacy, would prefer to avoid.

This is a useful [reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)

Available ADCs :

GPIO 32
GPIO 33
GPIO 34
GPIO 35
GPIO 36
GPIO 39

Regarding switch inputs, I'm currently using the following for DAC interface:

GPIO 25          WSEL
GPIO 26          BLCK
GPIO 27 (was 33) DIN

So, I think I can use:

GPIO 1
GPIO 3
GPIO 12
GPIO 13
GPIO 14

## Provisional Controls
Pitch
Joystick X : Formant 1
Joystick Y : Formant 2
Formant 3 freq
Nasal (cut filter) freq
Nasal BW

**2020-02-27**

While I'm getting a relatively clean sine wave, other waveforms (including the larynx simulation) suffer quite badly from aliasing artifacts on harmonics. The frequency-changing caused by noise on the pitch ADC make this particularly noticeable & horrible.

There are a good few strategies for reducing the aliasing. Interpolation is the obvious first thing to try.

Implemented. Slight performance hit. Not entirely sure about improvements, but reckon I'll leave in place.

**2020-02-26**

Trying to get a clean wavetable-generated waveform, pitch controlled by a pot.

I've got two threads running, one reading from ADC, one sending wavetable value to DAC.

The max freq at present is 3.5kHz, relatively clean. But dropping pitch makes it full of strange artifacts.

This is at 1kHz :

![Unclean Waveform](https://github.com/danja/chatterbox/blob/master/media/scope_2020-02-26.png "Unclean Waveform")

Two likely causes -

1. the looping around the wavetable has timing issues.
2. the ESP32 docs say the ADCs can be prone to noise

I've got a bit ahead of myself. Reckon I should dummy-out the ADC reading to isolate the timing.

Later... that made no difference. Finally spotted a *hilarious* oversight. The sine wave I was putting into the wavetable was from sin(0) to sin(TABLESIZE), not sin(0) to sin(2*PI)!

Much, much better. Good enough for now.

**NB.** the docs suggest putting a 100n capacitor across the ADC input(s) to reduce noise - this actually seemed to make it worse.

Ok, the current freq range is around 100Hz - 8kHz. A bit warbly when changing freq.

For testing purposes I've added code to fill the wavetable with sine, square, sawtooth, triangle, or 'larynx' waveforms, according to the position of another pot. (This might be nice to have user-selectable in final version? Maybe add an LCD display? scope creep...).

The other waveforms are more glitchy, to varying degrees.

This paper looks promising for tricks to reduce aliasing artifacts : [Table Lookup Oscillators Using Generic Integrated Wavetables](http://mtg.upf.edu/node/485)

Also discovered [Faust](https://faust.grame.fr/), DSP/synth kit, has support for ESP32. Looks awesome, but as one of my motivations for doing this is to learn the details of signal processing on little devices, I'm going to continue from scratch. Is fun!
