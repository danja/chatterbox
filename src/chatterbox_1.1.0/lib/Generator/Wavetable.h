#ifndef Wavetable_h
#define Wavetable_h

#include <Node.h>

const int TABLESIZE = 2048;
const float tablesize = 2048.0f;

class Wavetable : public Node
{
public:
    void init();
    float wavetable[TABLESIZE];

private:
};

#endif
