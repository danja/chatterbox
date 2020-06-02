#ifndef Receiver_h
#define Receiver_h

#include <WString.h>
#include <dispatcher.hpp>

enum EventType {VALUE_CHANGE, EVENT2, EVENT3};

class Receiver
{

public:
    Receiver();
    void registerCallback(Dispatcher<EventType, String, float> &dispatcher);

private:
    void listener(EventType type, String name, float value);
};

#endif