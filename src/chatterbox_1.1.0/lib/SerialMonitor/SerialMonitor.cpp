#include <Arduino.h>
#include <dispatcher.hpp>
#include <SerialMonitor.h>

SerialMonitor::SerialMonitor()
{
    // delay(2000); // let it connect

   // Serial.begin(serial_rate);

   // Serial.println("\n*** Starting Chatterbox ***\n");
}

void SerialMonitor::registerCallback(Dispatcher<EventType, String, float> &dispatcher)
{
    using namespace std::placeholders;
    dispatcher.addCallback(std::bind(&SerialMonitor::listener, this, _1, _2, _3));
}

void SerialMonitor::listener(EventType type, String name, float value)
{
    Serial.print(name + " SM: ");
    Serial.println(value, DEC);
}
