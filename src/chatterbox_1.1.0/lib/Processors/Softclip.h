#ifndef _SOFTCLIP_H
#define _SOFTCLIP_H

#include <Processor.h>

class Softclip : public Processor
{

public:
    Softclip();

    float process(float x);
    static float clamp(float x, float lowerlimit, float upperlimit);

};

#endif