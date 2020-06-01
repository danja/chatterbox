#include <SvfLinearTrapOptimised2.hpp>
#include <SVF.h>

SVF::SVF() {}

// SvfLinearTrapOptimised2::FLT_TYPE f1Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;

float SVF::process(float x)
{
    return filter.tick(x);
}

void SVF::initParameters(float freq, float q, String typeString, float sampleRate)
{
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
