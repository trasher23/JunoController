#ifndef MIDI_LED_indicator_h
#define MIDI_LED_indicator_h

#include "Arduino.h"

class MidiLedIndicator
{
  public:
    MidiLedIndicator(int pin);
    void noteOn();
    void noteOff();
    void cc();
    void update();
  private:
    int _ledPin;
    bool _ledActive;
    int _numberOfActiveNotes;
    int _lastCcTimestamp;
};

#endif
