#ifndef MIDIConnector_h
#define MIDIConnector_h

#include <WString.h>
#include <dispatcher.hpp>
#include <Receiver.h>

class MIDIConnector : public Receiver
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
    void listener(EventType type, String name, float value);
   
};
#endif