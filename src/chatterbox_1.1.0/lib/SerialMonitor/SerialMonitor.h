#include <Arduino.h>
#include <dispatcher.hpp>

enum EventType {VALUE_CHANGE, EVENT2, EVENT3};

class SerialMonitor
{

public:
    SerialMonitor();
    void registerCB(Dispatcher<EventType, String, float> &dispatcher);
    const int serial_rate = 115200;

private:
    void listener(EventType type, String name, float value);
};