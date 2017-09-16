#include "Arduino.h"
#include "MIDI_LED_indicator.h"

MidiLedIndicator::MidiLedIndicator(int pin) {
  pinMode(pin, OUTPUT);
  
  _ledPin = pin;
  _ledActive = false;
  _numberOfActiveNotes = 0;
  _lastCcTimestamp = 0;
  
}

void MidiLedIndicator::noteOn() {
  _numberOfActiveNotes++;
}

void MidiLedIndicator::noteOff() {
  _numberOfActiveNotes--;
  if (_numberOfActiveNotes < 0) {
    _numberOfActiveNotes = 0;
  }
}

void MidiLedIndicator::cc() {
  _lastCcTimestamp = millis();
}

void MidiLedIndicator::update() {
  bool ledShouldBeActive = (_numberOfActiveNotes > 0 || (millis() - _lastCcTimestamp < 100));
  
  if (_ledActive != ledShouldBeActive) {
    digitalWrite(_ledPin, ledShouldBeActive);
    _ledActive = ledShouldBeActive;
  }
}
