#ifndef SerialMonitor_h
#define SerialMonitor_h

#include <Arduino.h>
#include <dispatcher.hpp>
#include <EventReceiver.h>

// enum EventType {VALUE_CHANGE, EVENT2, EVENT3};
const int serial_rate = 115200;


class SerialMonitor: public EventReceiver
{

public:
    SerialMonitor();
    void registerCallback(Dispatcher<EventType, String, float> &dispatcher);

private:
    void listener(EventType type, String name, float value);
};

#endif