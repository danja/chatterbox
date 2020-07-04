//
//  WaveUtils.h
//
//  Created by Nigel Redmon on 2/18/13
//
//

#ifndef M_PI
  #define M_PI 3.1415927
#endif

#ifndef WaveUtils_h
#define WaveUtils_h

#include "WaveTableOsc.h"

int fillTables(WaveTableOsc *osc, double *freqWaveRe, double *freqWaveIm, int numSamples);
int fillTables2(WaveTableOsc *osc, double *freqWaveRe, double *freqWaveIm, int numSamples, double minTop = 0.4, double maxTop = 0);
float makeWaveTable(WaveTableOsc *osc, int len, double *ar, double *ai, double scale, double topFreq);

// examples
WaveTableOsc *sawOsc(void);
WaveTableOsc *waveOsc(double *waveSamples, int tableLen);

#endif
