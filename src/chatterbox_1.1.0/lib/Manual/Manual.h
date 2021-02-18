#ifndef Manual_h
#define Manual_h

#include <Pots.h>
#include <Switches.h>

class Manual
{
public:
    Manual();
    void initInputs();
    Switches switches;
    Pots pots;
};

#endif