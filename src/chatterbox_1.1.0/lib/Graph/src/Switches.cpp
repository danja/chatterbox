#include <Arduino.h>
#include <Switches.h>
#include <Switch.h>

// Switch svitches[N_SWITCHES];

Switches::Switches(){

svitches[SWITCH_SF1] = Switch("sf1", 12, PUSH);             //  S1
  svitches[SWITCH_SF2] = Switch("sf2", 19, PUSH);             // S2
  svitches[SWITCH_SF3] = Switch("sf3", 5, PUSH);              // S3
  svitches[SWITCH_VOICED] = Switch("voiced", 14, PUSH);       // S7
  svitches[SWITCH_ASPIRATED] = Switch("aspirated", 17, PUSH); // S5
  svitches[SWITCH_NASAL] = Switch("nasal", 23, PUSH);         // S6
  svitches[SWITCH_DESTRESS] = Switch("destressed", 18, PUSH); // S4
  svitches[SWITCH_STRESS] = Switch("stressed", 13, PUSH);     // S0

  for (int i = 0; i < N_SWITCHES; i++)
  {
    pinMode(svitches[i].channel(), INPUT);
    pinMode(svitches[i].channel(), INPUT_PULLDOWN);
  }
}


Switch Switches::getSwitch(int switchN){
    return svitches[switchN];
}