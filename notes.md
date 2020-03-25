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






