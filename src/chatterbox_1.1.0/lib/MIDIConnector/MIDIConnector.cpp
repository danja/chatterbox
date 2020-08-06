#include <WString.h>
#include <MIDI.h>
#include <HardwareSerial.h>
#include <MIDIConnector.h>
#include <dispatcher.hpp>
#include <Receiver.h>

#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_22)

static const int RX_BUF_SIZE = 1024;

using Transport = MIDI_NAMESPACE::SerialMIDI<HardwareSerial>;
int rxPin = 21;                                    //  1; //
int txPin = 22;                                    // 3;
HardwareSerial hardwareSerial = HardwareSerial(1); // UART_NUM_1
Transport serialMIDI(hardwareSerial);
MIDI_NAMESPACE::MidiInterface<Transport> MIDI((Transport &)serialMIDI);


/*
void handleNoteOn(byte channel, byte note, byte velocity) {
    float pitch = midiNoteToFreq(note);
    midiDispatcher.broadcast(VALUE_CHANGE, "pitch", pitch);
}

void handleNoteOff(byte channel, byte note, byte velocity) {
}
*/

void MIDIConnector::start() {
    MIDI.begin(1); // MIDI_CHANNEL_OMNI Listen to all incoming messages
    hardwareSerial.begin(31250, SERIAL_8N1, rxPin, txPin, false, 100);
    //    MIDI.setHandleNoteOn(handleNoteOn);
      //  MIDI.setHandleNoteOff(handleNoteOff);
}

// https://en.wikipedia.org/wiki/MIDI_tuning_standard
int freqToMIDINote(float freq)
{
    return 69 + 12.0f * log2(freq / 440.0f);
}

float midiNoteToFreq(int note)
{
    float pwr = ((float)note - 69.0f) / 12.0f;
    return 440.0f * powf(2.0f, pwr);
}



//////////////////////////////
MIDIConnector::MIDIConnector() {}


// void HardwareSerial::begin
    // (unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms)
    // Each byte is prefaced with a start bit (always zero),
    // followed by 8 data bits, then one stop bit (always high). MIDI doesn't use parity bits.

void MIDIConnector::registerCallback(Dispatcher<EventType, String, float> &dispatcher)
{
    using namespace std::placeholders;
    dispatcher.addCallback(std::bind(&MIDIConnector::listener, this, _1, _2, _3));
}


void MIDIConnector::read() {
    if (MIDI.read())
    {
        byte type = MIDI.getType();
        
        Serial.println(type,HEX);

        float pitch;
        float gain;

        switch (type) {

        case 0x90: // Note On midiDefs.h
            pitch = midiNoteToFreq(MIDI.getData1());
            gain = (float)MIDI.getData2()/128.0f;
            midiDispatcher.broadcast(NOTE_ON, "pitch", pitch);
            midiDispatcher.broadcast(NOTE_ON, "gain", gain);
            break;

        case 0x80: // Note Off midiDefs.h
            midiDispatcher.broadcast(NOTE_OFF, "gain", 0);
            break;
        }
    }
}

void MIDIConnector::listener(EventType type, String id, float value)
{
    char snum[5];
    itoa(value, snum, 10);
    String message = id + ":" + snum;
    // Serial.println(message);
    // ws.textAll(message);
        // MIDI.sendNoteOff(freqToMIDINote(patchbay.pitch), 127, 1); // temp test

           //   MIDI.sendNoteOn(freqToMIDINote(patchbay.pitch), 127, 1);
}