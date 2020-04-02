## Modeling the Human Voice

[Source-Filter Model](https://github.com/danja/chatterbox/blob/master/references/source-filter.pdf)

Manipulation of the larynx is used to generate a source sound with a particular fundamental frequency, or pitch. This source sound is altered as it travels through the vocal tract, configured differently based on the position of the tongue, lips, mouth, and pharynx.

![Human head and neck](https://github.com/danja/chatterbox/blob/master/manual/media/head-neck.jpg "Human head and neck")

Vocal tract : in mammals it consists of the laryngeal cavity, the pharynx, the oral cavity and the nasal cavity.

Average length in humans is around 17cm (male), 14cm (female).

![Source-Filter Model](https://github.com/danja/chatterbox/blob/master/manual/media/source-filter.png "Source-Filter Model")

For a uniform tube resonant frequencies are described by -

one end closed :

F[i] = (2i + 1) * c / 4 * L

both ends closed by:

F[i] = i * c / 2 * L

where i is the formant number, c is the speed of sound
(350 m/s), LL is vocal tract length (in m) and Fi is the
frequency (in Hz) of ith formant.

For a uniform tube regardless of end conditions, the frequency difference between
the successive resonances is constant and given by :

F[i] - F[i-1] = c / 2 * L


