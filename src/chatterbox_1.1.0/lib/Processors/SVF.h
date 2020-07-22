#ifndef SVF_h
#define SVF_h

#include <WString.h>
#include <Processor.h>
#include <SvfLinearTrapOptimised2.hpp>


// f1.updateCoefficients(f1f, F1Q, f1Type, samplerate);
// void updateCoefficients(float cutoff, float q = 0.5, FLT_TYPE type = LOW_PASS_FILTER, float sampleRate = 44100)
class SVF : public Processor
{

public:
    SVF();

    float process(const float x);
    // static float clamp(float x, float lowerlimit, float upperlimit);
   // SvfLinearTrapOptimised2 filter;
  void initParameters(float cutoff, float q, String type, float sampleRate, float gain);

private:
      SvfLinearTrapOptimised2 filter;
      float gain_ = 1.0f;
};

#endif