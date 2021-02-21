#ifndef EventReceiver_h
#define EventReceiver_h

#include <WString.h>
#include <dispatcher.hpp>

enum EventType {NOTE_ON, NOTE_OFF, VALUE_CHANGE, EVENT2, EVENT3};

class EventReceiver
{

public:
    EventReceiver();
    void registerCallback(Dispatcher<EventType, String, float> &dispatcher);

private:
// void listener(EventType type, String name, float value);
    void listener(const EventType& type, const String& name, const float& value);
};

#endif