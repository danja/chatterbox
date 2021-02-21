#ifndef MIDIConnector_h
#define MIDIConnector_h

#include <WString.h>
#include <dispatcher.hpp>
#include <EventReceiver.h>

class MIDIConnector : public EventReceiver
{

public:
    MIDIConnector();
    void start();
    void read();
    Dispatcher<EventType, String, float> midiDispatcher;
    void registerCallback(Dispatcher<EventType, String, float> &dispatcher);
 // void handleNoteOn(byte channel, byte note, byte velocity);
   // void handleNoteOff(byte channel, byte note, byte velocity);

private:
    void listener(const EventType& type, const String& name, const float& value);
   
};
#endif