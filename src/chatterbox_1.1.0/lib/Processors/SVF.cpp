#include <SvfLinearTrapOptimised2.hpp>
#include <SVF.h>
#include <Arduino.h>

SVF::SVF() {}

// SvfLinearTrapOptimised2::FLT_TYPE f1Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;

float SVF::process(const float x)
{
   // Serial.println("SVF");
   // Serial.println(x, DEC);
    float out = gain_ * filter.tick(x);
   // Serial.println("SVF out v");
   // Serial.println(out, DEC);
   // Serial.println("SVF out ^");
    return out;
}

// (f1f, F1_NASALQ, "notch", samplerate);

void SVF::initParameters(float freq, float q, String typeString, float sampleRate, float gain)
{
    gain_ = gain;
    floatParameter("freq", freq);
    floatParameter("q", q);
    floatParameter("sampleRate", sampleRate);
    stringParameter("type", typeString);

    if (typeString == "lowPass")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::LOW_PASS_FILTER, sampleRate);
    }
    else if (typeString == "bandPass")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::BAND_PASS_FILTER, sampleRate);
    }
    else if (typeString == "highPass")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::HIGH_PASS_FILTER, sampleRate);
    }
    else if (typeString == "notch")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::NOTCH_FILTER, sampleRate);
    }
    else if (typeString == "peak")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::PEAK_FILTER, sampleRate);
    }
    else if (typeString == "allPass")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::ALL_PASS_FILTER, sampleRate);
    }
    else if (typeString == "bell")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::BELL_FILTER, sampleRate);
    }
    else if (typeString == "lowShelf")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::LOW_SHELF_FILTER, sampleRate);
    }
    else if (typeString == "highShelf")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::HIGH_SHELF_FILTER, sampleRate);
    }
    else if (typeString == "none")
    {
        filter.updateCoefficients(freq, q, SvfLinearTrapOptimised2::NO_FLT_TYPE, sampleRate);
    }
}
