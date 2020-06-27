#ifndef Wavetable_h
#define Wavetable_h

#include <Node.h>

const int TABLESIZE = 1024;
const float tablesize = 1024.0f;

class Wavetable : public Node
{
public:
    // Control();
    void init();
float wavetable[TABLESIZE];

private:
    
};

#endif
