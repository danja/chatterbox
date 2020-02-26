** 2020-02-26 **

Trying to get a clean wavetable-generated waveform, pitch controlled by a pot.

I've got two threads running, one reading from ADC, one sending wavetable value to DAC. 

The max freq at present is 3.5kHz, relatively clean. But dropping pitch makes it full of strange artifacts.

This is at 1kHz :

![Unclean Waveform](https://github.com/danja/chatterbox/blob/master/media/scope_2020-02-26.png "Unclean Waveform")

Two likely causes - 

1. the looping around the wavetable has timing issues.
2. the ESP32 docs say the ADCs can be prone to noise


I've got a bit ahead of myself. Reckon I should dummy-out the ADC reading to isolate the timing. 


