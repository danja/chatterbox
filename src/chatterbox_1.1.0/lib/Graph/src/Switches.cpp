#include <Arduino.h>
#include <Switches.h>
#include <Switch.h>

// Switch switchArray[N_SWITCHES];

Switches::Switches()
{

    /*
    for (int i = 0; i < N_SWITCHES; i++)
    {
        pinMode(switchArray[i].channel(), INPUT);
        pinMode(switchArray[i].channel(), INPUT_PULLDOWN);
    }
    */
}

void Switches::init()
{
    switchArray[SWITCH_SF1] = Switch("sf1", 12, PUSH);             //  S1
    switchArray[SWITCH_SF2] = Switch("sf2", 19, PUSH);             // S2
    switchArray[SWITCH_SF3] = Switch("sf3", 5, PUSH);              // S3
    switchArray[SWITCH_VOICED] = Switch("voiced", 14, PUSH);       // S7
    switchArray[SWITCH_ASPIRATED] = Switch("aspirated", 17, PUSH); // S5
    switchArray[SWITCH_NASAL] = Switch("nasal", 23, PUSH);         // S6
    switchArray[SWITCH_DESTRESS] = Switch("destressed", 18, PUSH); // S4
    switchArray[SWITCH_STRESS] = Switch("stressed", 13, PUSH);     // S0

    switchArray[TOGGLE_HOLD] = Switch("hold", 16, TOGGLE);   // T0
    switchArray[TOGGLE_CREAK] = Switch("creak", 4, TOGGLE);  // T1
    switchArray[TOGGLE_SING] = Switch("sing", 2, TOGGLE);    // T2
    switchArray[TOGGLE_SHOUT] = Switch("shout", 15, TOGGLE); // T3

    for (int i = 0; i < N_SWITCHES; i++)
    {
        pinMode(switchArray[i].channel(), INPUT);
        pinMode(switchArray[i].channel(), INPUT_PULLDOWN);
        switchArray[i].on_ = false;
    }
}

Switch& Switches::getSwitch(int switchN)
{
    return switchArray[switchN];
}