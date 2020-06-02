#include <Arduino.h>
#include <dispatcher.hpp>
#include <Receiver.h>

Receiver::Receiver()
{
    // delay(2000); // let it connect

   // Serial.begin(serial_rate);

   // Serial.println("\n*** Starting Chatterbox ***\n");
}

void Receiver::registerCallback(Dispatcher<EventType, String, float> &dispatcher)
{
    using namespace std::placeholders;
    dispatcher.addCallback(std::bind(&Receiver::listener, this, _1, _2, _3));
}

void Receiver::listener(EventType type, String name, float value)
{
   // Serial.print(name + " : ");
   // Serial.println(value, DEC);
}
