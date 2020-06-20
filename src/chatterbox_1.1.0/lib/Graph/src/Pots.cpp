#include <Arduino.h>
#include <Pots.h>
#include <Pot.h>


Pots::Pots()
{
}

void Pots::init()
{
    float tablesize = (float)TABLESIZE; // move

    potArray[POT_P0] = Pot("f1f", 36);
    potArray[POT_P1] = Pot("f2f", 39);
    potArray[POT_P2] = Pot("f3f", 32);
    potArray[POT_P3] = Pot("f3q", 33);
    potArray[POT_P4] = Pot("larynx", 34);
    potArray[POT_P5] = Pot("pitch", 35);

    for (int i = 0; i < N_POTS_ACTUAL; i++)
    {
        adcAttachPin(potArray[i].channel());
    }

    potArray[POT_P0].range(ADC_TOP, F1F_LOW, F1F_HIGH);
    potArray[POT_P1].range(ADC_TOP, F2F_LOW, F2F_HIGH);
    potArray[POT_P2].range(ADC_TOP, F3F_LOW, F3F_HIGH);
    potArray[POT_P3].range(ADC_TOP, F3Q_MIN, F3Q_MAX);
    potArray[POT_P4].range(ADC_TOP, tablesize * LARYNX_MIN / 100.0f, tablesize * LARYNX_MAX / 100.0f);
    potArray[POT_P5].range(ADC_TOP, PITCH_MIN, PITCH_MAX);

    potArray[POT_GROWL].range(ADC_TOP, GROWL_MAX, GROWL_MIN);
}

Pot &Pots::getPot(int n)
{
    return potArray[n];
}