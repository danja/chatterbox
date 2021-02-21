#include <Arduino.h>
#include <dispatcher.hpp>
#include <EventReceiver.h>

EventReceiver::EventReceiver()
{
    // delay(2000); // let it connect

   // Serial.begin(serial_rate);

   // Serial.println("\n*** Starting Chatterbox ***\n");
}

void EventReceiver::registerCallback(Dispatcher<EventType, String, float> &dispatcher)
{
    using namespace std::placeholders;
    dispatcher.addCallback(std::bind(&EventReceiver::listener, this, _1, _2, _3));
}

void EventReceiver::listener(const EventType& type, const String& name, const float& value)
{
   // Serial.print(name + " : ");
   // Serial.println(value, DEC);
}
