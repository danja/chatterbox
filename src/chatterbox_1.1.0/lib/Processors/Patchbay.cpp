#include <Patchbay.h>
#include <SVF.h>

Patchbay::Patchbay() {}

void Patchbay::setModules(SVF const& svf1){
    svf1_ = svf1;
}

float Patchbay::process(float x)
{
    return Patchbay::svf1_.process(x);
}

