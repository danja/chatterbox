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
