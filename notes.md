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

Added documents : [Design Notes](https://github.com/danja/chatterbox/blob/master/design.md), [ToDo list](https://github.com/danja/chatterbox/blob/master/todo.md)

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






