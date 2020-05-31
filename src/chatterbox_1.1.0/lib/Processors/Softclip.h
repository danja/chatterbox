#include <Processor.h>

class Softclip : public Processor
{

public:
    Softclip();

    float process(float x);
    static float clamp(float x, float lowerlimit, float upperlimit);

};