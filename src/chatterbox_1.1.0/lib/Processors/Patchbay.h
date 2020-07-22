#ifndef Patchbay_h
#define Patchbay_h

#include <WString.h>
#include <Processor.h>
#include <SVF.h>

class Patchbay : public Processor
{

public:
    Patchbay();

    float process(const float x);
    void setModules(SVF const& svf1);

private:
      SVF svf1_;
};

#endif