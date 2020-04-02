## Chatterbox Controls

There are 6 analog controls, all feeding to ADCs. The physical controls/value readings in code are referred here to as 'actual'. Each is mapped to a 'virtual' control which links the actual control to it's functionality. Some controls are mapped to secondary functions, dependent on toggle switch setting.

### Analog
P0 (Joystick X) = f1f / nasal
P1 (Joystick Y) = f2f
P2 (Pot) = f3f
P3 (Pot) = f3q 
P4 (Pot) = larynx
P5 (Pot) = pitch

### Digital
Each digital control has an associated ID string. The associated functionality is fixed. When multiple switches are in the 'on' position, the effect is additive.

#### Push Switches
SW0 Fricative/sibilance, low frequency
SW1 Fricative/sibilance, medium frequency
SW2 Fricative/sibilance, high frequency
SW3 Voiced Signal
SW4 Aspirated Signal
SW5 Nasal Effect
SW6 Stressed Effect
SW7 De-Stressed Effect

#### Toggle Switches
**fixed functionality, all function on/off**

SW8 Hold
SW9 Creak
SW10 Sing
SW11 Shout

### Virtual Analog Controls
Each virtual control has an associated ID string.

**Defaults**
P0 "f1f"
P1 "f2f"
P2 "f3f"
P3 "f3q"
P4 "larynx"
P5 "pitch"

**Alternate**
P0 "nasal"
("logistic")







### Switches

Toggle vs. Push

Bounce - envelope

Potentiometers

ADC Characteristics

**Pitch**

[Auditory scales of frequency representation](https://web.archive.org/web/20110427105916/http://www.ling.su.se/staff/hartmut/bark.htm), [Bark Scale](https://en.wikipedia.org/wiki/Bark_scale), [Mel Scale](https://en.wikipedia.org/wiki/Mel_scale).
