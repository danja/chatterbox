#include <Manual.h>
#include <Pots.h>
#include <Switches.h>
// #include <Arduino.h>
#include <driver/adc.h> 

Manual::Manual(){}

void Manual::initInputs()
{
    // init ADC inputs (pots)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

    pots.init();
    switches.init();
}