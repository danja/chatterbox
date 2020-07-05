#ifndef Patchbay_h
#define Patchbay_h

#include <WString.h>
#include <Processor.h>
#include <SVF.h>

class Patchbay : public Processor
{

public:
    Patchbay();

    float process(float x);
    void setModules(Processor const& svf1);

private:
      Processor svf1_;
};

#endif