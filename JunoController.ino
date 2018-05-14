/*
  Juno Controller
  Chris Elliot 20170725
  Branched from https://bitbucket.org/loveaurell/roland-alpha-juno-mks-50-midi-controller
  Converted for Alpha Juno 1
*/

#include <MIDI.h> // Using version 4.3 of MIDI library
#include <Bounce.h>
#include <LiquidCrystal595.h>
#include <ResponsiveAnalogRead.h> // Improve analog read response
#include "junoController.h"
#include "MIDI_LED_indicator.h"

ResponsiveAnalogRead analog(0, false);

// Analog controls are mapped by [multiplexerChannel][pinIndex]
ControlPin analogControls[NUMBER_OF_MULTIPLEXER_CHANNELS][NUMBER_OF_MULTIPLEXERS];

// Digital controls are mapped by [pinIndex]
ControlPin digitalControls[NUMBER_OF_DIGITAL_PARAMETER_SWITCHES];

// Midi LED
MidiLedIndicator midiLedIndicator = MidiLedIndicator(MIDI_INDICATOR_LED);

// PWM LED
int PWMledState = LOW;
unsigned long PWMpreviousMillis = 0;

// LCD display
LiquidCrystal595 lcd(7, 8, 9); // datapin, latchpin, clockpin
LCDDisplay LCDdisplay; // LCD Text per Control

// Variables used for sending the state of all controls to the MIDI OUT
Bounce controlDumpButton = Bounce(DIGITAL_INPUT_PIN_DUMP_CONTROLS_BUTTON, BUTTON_DEBOUNCE_DELAY);

// Variables used to assign modulation wheel to any variable parameter
Bounce modWheelAssignButton =   Bounce(DIGITAL_INPUT_PIN_MOD_WHEEL_ASSIGN_BUTTON, BUTTON_DEBOUNCE_DELAY);
bool modWheelAssignActive = false;
int modWheelParam = -1;
int modWheelParamGroup = -1;

// Variables used to assign control pedal to any variable parameter
Bounce modPedalAssignButton =   Bounce(DIGITAL_INPUT_PIN_MOD_PEDAL_ASSIGN_BUTTON, BUTTON_DEBOUNCE_DELAY);
bool modPedalAssignActive = false;
int modPedalParam = -1;
int modPedalParamGroup = -1;

Bounce vcaHoldButton = Bounce(DIGITAL_INPUT_PIN_VCA_HOLD, BUTTON_DEBOUNCE_DELAY);
bool vcaHoldActive = false;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {
  initializePins();

  mapAnalogControlsToMultiplexers();
  addJitterThresholdToAnalogControls();
  analogReadAveraging(ANALOG_READ_RESOLUTION);
  
  mapDigitalControlsToPins();
  
  MIDI.setHandleNoteOn(handleMidiNoteOn);
  MIDI.setHandleNoteOff(handleMidiNoteOff);
  MIDI.setHandleControlChange(handleMidiControlChange);
  MIDI.begin(DEFAULT_MIDI_CHANNEL);

  initialiseLCD();

  assignAnalogControlCurrentValues();
  assignDigitalControlCurrentValues();
}

void loop() {
  readControlDumpButton();
  readModWheelAssignButton();
  readModPedalAssignButton();
  readVcaHoldButton();

  readAnalogControls();
  readDigitalControls();

  setDcoPwmLed();

  readMidiIn();
  midiLedIndicator.update();
}

/*
  Setup functions
*/
void initialiseLCD() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Juno Controller");
  lcd.setCursor(0, 1);
  lcd.print("Version 1.1");
  delay(1000);
}

void initializePins() {
  pinMode(MULTIPLEXER_CHANNEL_CONTROL_PIN_1, OUTPUT);
  pinMode(MULTIPLEXER_CHANNEL_CONTROL_PIN_2, OUTPUT);
  pinMode(MULTIPLEXER_CHANNEL_CONTROL_PIN_3, OUTPUT);

  pinMode(PWM_ACTIVE_INDICATOR_LED, OUTPUT);
  pinMode(MIDI_INDICATOR_LED, OUTPUT);

  pinMode(DIGITAL_INPUT_PIN_CHORUS_SWITCH, INPUT_PULLUP);
  pinMode(DIGITAL_INPUT_PIN_PORTAMENTO_SWITCH, INPUT_PULLUP);
  pinMode(DIGITAL_INPUT_PIN_POLYPHONY_SWITCH, INPUT_PULLUP);

  pinMode(DIGITAL_INPUT_PIN_DUMP_CONTROLS_BUTTON, INPUT_PULLUP);
  pinMode(DIGITAL_INPUT_PIN_MOD_WHEEL_ASSIGN_BUTTON, INPUT_PULLUP);
  pinMode(DIGITAL_INPUT_PIN_MOD_PEDAL_ASSIGN_BUTTON, INPUT_PULLUP);
  pinMode(DIGITAL_INPUT_PIN_VCA_HOLD, INPUT_PULLUP);

  analogReadResolution(ANALOG_READ_RESOLUTION);
}

// Midi controls are mapped to [multiplexerChannel][multiplexerIndex]
void mapAnalogControlsToMultiplexers() {

  /*
    MULTIPLEXER 1
    pin, paramGroup, param, maxValue, ControlName
  */
  analogControls[0][0] = {ANALOG_INPUT_PIN_1, PARAM_GROUP_TONE, TONE_PARAM_DCO_PWM_RATE,        CONTROL_MAX_VALUE,            "DCO: PWM Rate"};
  analogControls[1][0] = {ANALOG_INPUT_PIN_1, PARAM_GROUP_TONE, TONE_PARAM_DCO_PWM_DEPTH,       CONTROL_MAX_VALUE,            "DCO: PWM"};
  analogControls[2][0] = {ANALOG_INPUT_PIN_1, PARAM_GROUP_TONE, TONE_PARAM_DCO_WAVEFORM_SUB,    DCO_WAVEFORM_SUB_MAX_VALUE,   "DCO: Sub"};
  analogControls[3][0] = {ANALOG_INPUT_PIN_1, PARAM_GROUP_TONE, TONE_PARAM_DCO_SUB_LEVEL,       DCO_SUB_LEVEL_MAX_VALUE,      "DCO: Sub Level"};
  analogControls[4][0] = { -1}; // Fine Tune not implemented on Juno 1
  analogControls[5][0] = {ANALOG_INPUT_PIN_1, PARAM_GROUP_TONE, TONE_PARAM_DCO_WAVEFORM_PULSE,  DCO_WAVEFORM_PULSE_MAX_VALUE, "DCO: Pulse"};
  analogControls[6][0] = {ANALOG_INPUT_PIN_1, PARAM_GROUP_TONE, TONE_PARAM_DCO_OCTAVE,          DCO_OCTAVE_MAX_VALUE,         "DCO: Octave"};
  analogControls[7][0] = {ANALOG_INPUT_PIN_1, PARAM_GROUP_TONE, TONE_PARAM_DCO_WAVEFORM_SAW,    DCO_WAVEFORM_SAW_MAX_VALUE,   "DCO: Saw"};

  /*
    MULTIPLEXER 2
  */
  analogControls[0][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_TONE, TONE_PARAM_DCO_AFTERTOUCH_MOD,  CONTROL_MAX_VALUE,            "DCO: Aftertouch"};
  analogControls[1][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_TONE, TONE_PARAM_DCO_LFO_MOD,         CONTROL_MAX_VALUE,            "DCO: LFO Mod"};
  analogControls[2][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_TONE, TONE_PARAM_DCO_ENV_MODE,        ENV_MODE_MAX_VALUE,           "DCO: Env Mode"};
  analogControls[3][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_TONE, TONE_PARAM_DCO_NOISE_LEVEL,     DCO_NOISE_LEVEL_MAX_VALUE,    "DCO: Noise"};
  analogControls[4][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_TONE, TONE_PARAM_CHORUS_RATE,         CONTROL_MAX_VALUE,            "Chorus: Rate"};
  analogControls[6][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_PATCH, PATCH_PARAM_PORTAMENTO_TIME,   CONTROL_MAX_VALUE,            "Glide: Time"};
  analogControls[5][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_TONE, TONE_PARAM_LFO_DELAY,           CONTROL_MAX_VALUE,            "LFO: Delay"};
  analogControls[7][1] = {ANALOG_INPUT_PIN_2, PARAM_GROUP_TONE, TONE_PARAM_DCO_ENV_MOD,         CONTROL_MAX_VALUE,            "DCO: Env Mod"};

  /*
    MULTIPLEXER 3
  */
  analogControls[0][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_VCF_KEY_FOLLOW,      CONTROL_MAX_VALUE,            "VCF: Key Follow"};
  analogControls[1][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_VCF_ENV_MOD,         CONTROL_MAX_VALUE,            "VCF: Env Mod"};
  analogControls[2][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_HPF_CUTOFF,          HPF_MAX_VALUE,                "VCF: HPF Cutoff"};
  analogControls[3][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_VCF_RESONANCE,       CONTROL_MAX_VALUE,            "VCF: Resonance"};
  analogControls[4][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_VCF_ENV_MODE,        ENV_MODE_MAX_VALUE,           "VCF: Env Mode"};
  analogControls[5][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_VCF_LFO_MOD,         CONTROL_MAX_VALUE,            "VCF: LFO Mod"};
  analogControls[6][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_VCF_CUTOFF,          CONTROL_MAX_VALUE,            "VCF: Cutoff"};
  analogControls[7][2] = {ANALOG_INPUT_PIN_5, PARAM_GROUP_TONE, TONE_PARAM_VCF_AFTERTOUCH_MOD,  CONTROL_MAX_VALUE,            "VCF: Aftertouch"};


  /*
    MULTIPLEXER 4
  */
  analogControls[0][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_L2,              CONTROL_MAX_VALUE,            "ENV: L2"};
  analogControls[1][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_L3,              CONTROL_MAX_VALUE,            "ENV: L3"};
  analogControls[2][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_T3,              CONTROL_MAX_VALUE,            "ENV: T3"};
  analogControls[3][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_T2,              CONTROL_MAX_VALUE,            "ENV: T2"};
  analogControls[4][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_T4,              CONTROL_MAX_VALUE,            "ENV: T4"};
  analogControls[5][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_KEY_FOLLOW,      CONTROL_MAX_VALUE,            "ENV: Key Follow"};
  analogControls[6][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_T1,              CONTROL_MAX_VALUE,            "ENV: T1"};
  analogControls[7][3] = {ANALOG_INPUT_PIN_3, PARAM_GROUP_TONE, TONE_PARAM_ENV_L1,              CONTROL_MAX_VALUE,            "ENV: L1"};

  /*
    MULTIPLEXER 5
  */
  analogControls[2][4] = {ANALOG_INPUT_PIN_4, PARAM_GROUP_TONE, TONE_PARAM_VCA_LEVEL,           CONTROL_MAX_VALUE,            "VCA: Level"};
  analogControls[0][4] = {ANALOG_INPUT_PIN_4, PARAM_GROUP_TONE, TONE_PARAM_VCA_AFTERTOUCH_MOD,  CONTROL_MAX_VALUE,            "VCA: Aftertouch"};
  analogControls[1][4] = {ANALOG_INPUT_PIN_4, PARAM_GROUP_TONE, TONE_PARAM_VCA_ENV_MODE,        ENV_MODE_MAX_VALUE,           "VCA: Env Mode"};
  analogControls[3][4] = { -1}; // Not implemented
  analogControls[5][4] = {ANALOG_INPUT_PIN_4, PARAM_GROUP_PATCH, PATCH_PARAM_TRANSPOSE,         PATCH_PARAM_TRANSPOSE,        "MSTR: Transpose"};
  analogControls[6][4] = {ANALOG_INPUT_PIN_4, PARAM_GROUP_TONE, TONE_PARAM_BEND_RANGE,          PATCH_PARAM_MONO_BEND_RANGE,  "MSTR: Bend Rng"};
  analogControls[7][4] = {ANALOG_INPUT_PIN_4, PARAM_GROUP_PATCH, PATCH_PARAM_MASTER_VOLUME,     CONTROL_MAX_VALUE,            "MSTR: Vol"};
  analogControls[4][4] = {ANALOG_INPUT_PIN_4, PARAM_GROUP_TONE, TONE_PARAM_LFO_RATE,            CONTROL_MAX_VALUE,            "LFO: Rate"};
}

void mapDigitalControlsToPins() {
  digitalControls[0] = {DIGITAL_INPUT_PIN_CHORUS_SWITCH,     PARAM_GROUP_TONE,  TONE_PARAM_CHORUS_SWITCH,      1, "Chorus"};
  digitalControls[1] = {DIGITAL_INPUT_PIN_PORTAMENTO_SWITCH, PARAM_GROUP_PATCH, PATCH_PARAM_PORTAMENTO_SWITCH, 1, "Glide"};
  digitalControls[2] = {DIGITAL_INPUT_PIN_POLYPHONY_SWITCH,  PARAM_GROUP_PATCH, PATCH_PARAM_POLYPHONY_SWITCH,  1, "Polyphony"};
}

// Adds a threshold that controls how much a signal must differ to count as a value change.
void addJitterThresholdToAnalogControls() {
  for (int channel = 0; channel < NUMBER_OF_MULTIPLEXER_CHANNELS; channel++) {
    for (int multiplexer = 0; multiplexer < NUMBER_OF_MULTIPLEXERS; multiplexer++) {
      ControlPin& control = analogControls[channel][multiplexer];
      // 1.8 gets it to around 7, which appears to be enough to dial-in each value between 0 and 125 (can't get 127 yet, will work it out eventually) and still prevent jitter
      // 20180513: Changed to 1
      control.jitterThreshold = (MAX_ANALOG_READ_VALUE * 1.8) / ((control.maxValue + 1) << 1);
    }
  }
}

void assignAnalogControlCurrentValues() {
  for (int channel = 0; channel < NUMBER_OF_MULTIPLEXER_CHANNELS; channel++) {
    selectMultiplexerChannel(channel);

    for (int multiplexer = 0; multiplexer < NUMBER_OF_MULTIPLEXERS; multiplexer++) {
      ControlPin& control = analogControls[channel][multiplexer];
      // Control Pin {-1} is Null
      if (control.pin != -1) {
        control.currentAnalogReading = analogRead(control.pin);
        control.previousAnalogReading = control.currentAnalogReading;
        control.currentValue = handleAnalogControlChange(control, map(control.currentAnalogReading, 0, MAX_ANALOG_READ_VALUE + 1, 0, control.maxValue + 1));
        control.previousValue = control.currentValue;
        sendSysExc(control);
      }
    }
  }
}

void assignDigitalControlCurrentValues() {
  for (int i = 0; i < NUMBER_OF_DIGITAL_PARAMETER_SWITCHES; i++) {
    ControlPin& control = digitalControls[i];

    control.currentValue = digitalRead(control.pin);
    control.previousValue = control.currentValue;
    handleDigitalControlChange(control);
  }
}

/*
  Loop functions
*/
void readControlDumpButton() {
  if (controlDumpButton.update()) {
    lcd.setCursor(0, 0);
    lcd.print("Send Ctrl State");
    
    lcd.setCursor(0, 1);
    lcd.print("                ");
    delay(1000);
    
    assignAnalogControlCurrentValues();
    assignDigitalControlCurrentValues();
  }
}

void readModWheelAssignButton() {
  if (modWheelAssignButton.update()) {
    if (modWheelAssignButton.risingEdge()) {
      //Serial.println("Mod wheel assign on");
      modWheelParam = -1;
      modWheelAssignActive = true;
    } else {
      //Serial.println("Mod wheel assign off");
      modWheelAssignActive = false;
    }
  }
}

void readModPedalAssignButton() {
  if (modPedalAssignButton.update()) {
    if (modPedalAssignButton.risingEdge()) {
      //Serial.println("Mod pedal assign on");
      modPedalParam = -1;
      modPedalAssignActive = true;
    } else {
      //Serial.println("Mod pedal assign off");
      modPedalAssignActive = false;
    }
  }
}

void readVcaHoldButton() {
  if (vcaHoldButton.update()) {
    vcaHoldActive = vcaHoldButton.fallingEdge();
    // Mimic hold on/off using sustain
    sendSustainChange(vcaHoldActive);
  }
}

void readAnalogControls() {
  for (int channel = 0; channel < NUMBER_OF_MULTIPLEXER_CHANNELS; channel++) {
    selectMultiplexerChannel(channel);
    readMultiplexersOnChannel(channel);
    readMidiIn();
  }
}

void selectMultiplexerChannel(int channel) {
  digitalWrite(MULTIPLEXER_CHANNEL_CONTROL_PIN_3, HIGH && (channel & B00000100));
  digitalWrite(MULTIPLEXER_CHANNEL_CONTROL_PIN_2, HIGH && (channel & B00000010));
  digitalWrite(MULTIPLEXER_CHANNEL_CONTROL_PIN_1, HIGH && (channel & B00000001));

  // Allow signals to stablize
  delayMicroseconds(MUX_CHANNEL_SWITCH_DELAY_IN_MICROSECONDS);
}

void readMultiplexersOnChannel(int channel) {
  for (int multiplexer = 0; multiplexer < NUMBER_OF_MULTIPLEXERS; multiplexer++) {
    ControlPin& control = analogControls[channel][multiplexer];

    // Control Pin {-1} is Null
    if (control.pin != -1) {
      analog.update(analogRead(control.pin));
      control.currentAnalogReading = analog.getValue();
      
      if ((abs(control.currentAnalogReading - control.previousAnalogReading) > control.jitterThreshold) && analog.hasChanged()) {        
        control.previousAnalogReading = control.currentAnalogReading;
     
        control.currentValue = handleAnalogControlChange(control, map(control.currentAnalogReading, 0, MAX_ANALOG_READ_VALUE + 1, 0, control.maxValue));
        
        if (control.currentValue != control.previousValue) {

          Serial.println(control.previousValue);
          Serial.println(control.currentValue);
        
          control.previousValue = control.currentValue;
          sendSysExc(control);
          updateLCD(control);
        }
      }
    }
  }
}

int handleAnalogControlChange(struct ControlPin& control, int value) {
  if (control.paramGroup == PARAM_GROUP_TONE) {
    switch (control.param) {

      case TONE_PARAM_BEND_RANGE:
        value++; // Useless to have 0 as possible bend range, and wanted to fit in the max range 12 in a 12 step rotary switch.
        createAndSendSysexMessage(PARAM_GROUP_PATCH, PATCH_PARAM_MONO_BEND_RANGE, value); // Mono and poly bend range are two separate params so we need to send an extra message.
        break;

      // Power leads soldered back to front for Rotary Switches, Doh!
      // Reverse the value!
      case TONE_PARAM_DCO_OCTAVE:
        value = DCO_OCTAVE_MAX_VALUE - value;
        break;

      case TONE_PARAM_DCO_WAVEFORM_PULSE:
        value = DCO_WAVEFORM_PULSE_MAX_VALUE - value;
        break;

      case TONE_PARAM_DCO_WAVEFORM_SAW:
        value = DCO_WAVEFORM_SAW_MAX_VALUE - value;
        break;

      case TONE_PARAM_DCO_WAVEFORM_SUB:
        value = DCO_WAVEFORM_SUB_MAX_VALUE - value;
        break;

      case TONE_PARAM_DCO_SUB_LEVEL:
        value = DCO_SUB_LEVEL_MAX_VALUE - value;
        break;

      case TONE_PARAM_DCO_NOISE_LEVEL:
        value = DCO_NOISE_LEVEL_MAX_VALUE - value;
        break;

      case TONE_PARAM_DCO_ENV_MODE:
        value = ENV_MODE_MAX_VALUE - value;
        break;

      case TONE_PARAM_VCF_ENV_MODE:
        value = ENV_MODE_MAX_VALUE - value;
        break;

      case TONE_PARAM_VCA_ENV_MODE:
        value = ENV_MODE_MAX_VALUE - value;
        break;

      default :
        break;
    }

  } else if (control.paramGroup == PARAM_GROUP_PATCH) {
    switch (control.param) {

      case PATCH_PARAM_TRANSPOSE:
        value -= (TRANSPOSE_MAX_VALUE >> 1); // MKS-50 wants "negative" values as MAX_VALUE - theUnsignedNegativeValue
        break;

      case PATCH_PARAM_FINE_TUNE:
        value -= (FINE_TUNE_MAX_VALUE >> 1); // MKS-50 wants "negative" values as MAX_VALUE - theUnsignedNegativeValue
        break;

      default :
        break;
    }
  }
  return value;
}

void sendSysExc(struct ControlPin& control) {
  if (modWheelAssignActive) {
    modWheelParam = control.param;
    modWheelParamGroup = control.paramGroup;
    createAndSendSysexMessage(PARAM_GROUP_PATCH, PATCH_PARAM_MOD_SENSITIVITY, 0);
//          Serial.print("Assigning param ");
//          Serial.print(control.param);
//          Serial.println(" to mod wheel");
  }

  if (modPedalAssignActive) {
    modPedalParam = control.param;
    modPedalParamGroup = control.paramGroup;
//          Serial.print("Assigning param ");
//          Serial.print(control.param);
//          Serial.println(" to mod pedal");
  }

  if (control.paramGroup == PARAM_GROUP_PATCH && control.param == PATCH_PARAM_PORTAMENTO_TIME) {
    createAndSendCCMessage(control.param, control.currentValue);
  } else if (control.paramGroup == PARAM_GROUP_PATCH && control.param == PATCH_PARAM_MASTER_VOLUME) {
    createAndSendCCMessage(control.param, abs(control.currentValue)); // abs to get around negative Master Volume
  } else {
    createAndSendSysexMessage(control.paramGroup, control.param, control.currentValue);
  }
}

void readDigitalControls() {
  for (int i = 0; i < NUMBER_OF_DIGITAL_PARAMETER_SWITCHES; i++) {
    ControlPin& control = digitalControls[i];

    control.currentValue = digitalRead(control.pin);
    if (control.currentValue != control.previousValue) {
      control.previousValue = control.currentValue;
      handleDigitalControlChange(control);
      updateLCD(control);
    }
  }
}

void handleDigitalControlChange(struct ControlPin& control) {
  int value;

  switch (control.param) {

    case PATCH_PARAM_POLYPHONY_SWITCH:
      if (control.currentValue == 1) {
        value = 0;
      } else {
        value = 1;
      }

      createAndSendCCMessage(PATCH_PARAM_POLYPHONY_SWITCH, value);
      break;

    case PATCH_PARAM_PORTAMENTO_SWITCH:
      if (control.currentValue == 1) {
        value = 127;
      } else {
        value = 0;
      }

      createAndSendCCMessage(PATCH_PARAM_PORTAMENTO_SWITCH, value);
      break;

    default :
      createAndSendSysexMessage(control.paramGroup, control.param, control.currentValue);
      break;
  }
}

void createAndSendCCMessage(uint8_t param, uint8_t value) {
  MIDI.sendControlChange(param, value, DEFAULT_MIDI_CHANNEL);
}

void createAndSendSysexMessage(uint8_t group, uint8_t param, uint8_t value) {
  MIDI.sendSysEx(NUMBER_OF_SYSEX_DIGITS, createSysExMessage(group, param, value), true);
}

const uint8_t* createSysExMessage(uint8_t group, uint8_t param, uint8_t value) {
  static uint8_t data[NUMBER_OF_SYSEX_DIGITS] = {0xF0, 0x41, 0x36, 0x0, 0x23, 0x20, 0x01, 0x0, 0x0, 0xF7};

  data[3] = DEFAULT_MIDI_CHANNEL - 1;
  data[5] = group;
  data[7] = param;
  data[8] = value;

  return data;
}

void handleMidiNoteOn(byte channel, byte note, byte velocity) {
  //Serial.println("Note on");
  midiLedIndicator.noteOn();
}

void handleMidiNoteOff(byte channel, byte note, byte velocity) {
  //Serial.println("Note off");
  midiLedIndicator.noteOff();
}

void handleMidiControlChange(byte channel, byte number, byte value) {
  //Serial.println("Control Change");
  midiLedIndicator.cc();
  //If a parameter is assigned to mod wheel, send sysex to change that value
  if (MOD_WHEEL_PARAM == number && modWheelParam > -1) {
    createAndSendSysexMessage(modWheelParamGroup, modWheelParam, value);
    //If a parameter is assigned to mod pedal, send sysex to change that value
  } else if (MOD_PEDAL_PARAM == number && modPedalParam > -1) {
    createAndSendSysexMessage(modPedalParamGroup, modPedalParam, value);
  } else if (SUSTAIN_PEDAL_PARAM == number && vcaHoldActive) {
    if (value < 64) {
      //If sustain off is sent, we want to retrigger the sustain to mimic hold.
      //To do this properly we would actually not want to send the sustain off message at all, but it's done automatically by the soft thru logic,
      //and I haven't found a way to filter out certain messages only.
      sendSustainChange(true);
    }
  }
}

void readMidiIn() {
  MIDI.read();
}

void sendSustainChange(bool on) {
  int value;

  if (on) {
    //Serial.println("Sustain Active");
    value = 127;
  } else {
    value = 0;
  }

  createAndSendCCMessage(SUSTAIN_PEDAL_PARAM, value);
}

void setDcoPwmLed() {
  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  // Relate this to PWM Rate value
  // 1100 = (Max Control Value + 100) to prevent 0 time interval
  long PWMinterval = 1100 - (analogControls[0][0].currentValue * 8.5) - log(1100);

  bool dcoPulse3Active = (analogControls[5][0].currentValue == DCO_PULSE_WAVEFORM_3);
  bool dcoSaw3Active = (analogControls[7][0].currentValue == DCO_SAW_WAVEFORM_3);

  if ((dcoPulse3Active || dcoSaw3Active)) {
    if (currentMillis - PWMpreviousMillis >= PWMinterval) {
      // save the last time you blinked the LED
      PWMpreviousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (PWMledState == LOW) {
        PWMledState = HIGH;
      } else {
        PWMledState = LOW;
      }
    }
  } else if (!dcoPulse3Active && !dcoSaw3Active) {
    PWMledState = LOW;
  }

  digitalWrite(PWM_ACTIVE_INDICATOR_LED, PWMledState);
}

void updateLCD(struct ControlPin& control) {
  if (control.controlName != LCDdisplay.currentControlName) {
    LCDdisplay.currentControlName = padLCDdisplay(control.controlName);

    lcd.setCursor(0, 0);
    lcd.print(LCDdisplay.currentControlName);
  }

  if (String(control.currentValue) != LCDdisplay.currentValue) {
    LCDdisplay.currentValue = padLCDdisplay(String(abs(control.currentValue))); // abs is temp for Master Volume, it has negative value

    lcd.setCursor(0, 1);
    lcd.print(LCDdisplay.currentValue);
  }
}

String padLCDdisplay(String value) {
  // Ensure string is 16 characters in length with space chars, to prevent need to use lcd.clear each time
  int padValue;

  if (value.length() < 16 ) {
    padValue = (15 - value.length());
    for (int i = 0; i < padValue; i++) {
      value = value + " ";
    }
  }
  return value;
}
