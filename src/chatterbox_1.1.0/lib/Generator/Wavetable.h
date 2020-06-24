#ifndef Wavetable_h
#define Wavetable_h

#include <Node.h>

#define TABLESIZE 2048

class Wavetable : public Node
{
public:
    // Control();
    void init();
float wavetable[TABLESIZE];

private:
    
};

#endif
