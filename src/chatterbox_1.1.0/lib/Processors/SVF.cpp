#include <SvfLinearTrapOptimised2.hpp>
#include <SVF.h>

SVF::SVF(){}

// SvfLinearTrapOptimised2::FLT_TYPE f1Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;


float SVF::process(float x){
    return filter.tick(x);
}

void SVF::updateCoefficients(float cutoff, float q, SvfLinearTrapOptimised2::FLT_TYPE type, float sampleRate){
   filter.updateCoefficients(cutoff, q, type, sampleRate);
}
  