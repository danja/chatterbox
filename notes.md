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


I'm currently using the following for DAC interface:

GPIO 25          WSEL
GPIO 26          BLCK
GPIO 27 (was 33) DIN

So, I think I can use:

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






