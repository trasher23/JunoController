/*
	Juno Controller
*/

#ifndef junoController_h
#define junoController_h

/* 

Define SysEx
  MKS-50 sysex info:
  F0 [Exclusive]
  41 [Roland ID#]
  36 [Individual Tone Parameters] (Not sure what this means)
  0N [N=MIDI channel (N=0-F, Chan 1=00 Chan 16=0F)]
  23 [Format type]
  20 [Message "type"] (In my code, PARAM_GROUP_TONE=0x20 or PARAM_GROUP_PATCH=0x30. )
  01 [unknown]
  XX [Parameter number]
  YY [Value (0-127)]
  F7 [End of Exclusive]

Example:
  A SysEx message for editing the VCF Resonance Parameter on MIDI channel 1:
  F0 41 36 00 23 20 01 11 00 F7
  
*/

#define NUMBER_OF_SYSEX_DIGITS                      10

// The parameters are divided into two groups, TONE and PATCH.
// Control parameter values
#define PARAM_GROUP_TONE                            0x20

#define TONE_PARAM_DCO_ENV_MODE                     0x00 //0  0=Normal, 1=Inverted, 2=Normal-Dynamic, 3=Inv.-Dynamic
#define TONE_PARAM_VCF_ENV_MODE                     0x01 //1  0=Normal, 1=Inverted, 2=Normal-Dynamic, 3=Dynamic
#define TONE_PARAM_VCA_ENV_MODE                     0x02 //2  0=Normal, 1=Gate, 2=Normal-Dynamic, 3=Gate-Dynamic
#define TONE_PARAM_DCO_WAVEFORM_PULSE               0x03 //3  (0..3)
#define TONE_PARAM_DCO_WAVEFORM_SAW                 0x04 //4  (0..5)
#define TONE_PARAM_DCO_WAVEFORM_SUB                 0x05 //5  (0..5)
#define TONE_PARAM_DCO_OCTAVE                       0x06 //6  (0=4', 1=8', 2=16', 3=32')
#define TONE_PARAM_DCO_SUB_LEVEL                    0x07 //7  (0..3)
#define TONE_PARAM_DCO_NOISE_LEVEL                  0x08 //8  (0..3)
#define TONE_PARAM_HPF_CUTOFF                       0x09 //9  (0..3)
#define TONE_PARAM_CHORUS_SWITCH                    0x0A //10 (0=Off, 1=On)
#define TONE_PARAM_DCO_LFO_MOD                      0x0B //11 (0...7F)
#define TONE_PARAM_DCO_ENV_MOD                      0x0C //12 (0...7F)
#define TONE_PARAM_DCO_AFTERTOUCH_MOD               0x0D //13 (0...7F)
#define TONE_PARAM_DCO_PWM_DEPTH                    0x0E //14 (0...7F)
#define TONE_PARAM_DCO_PWM_RATE                     0x0F //15 (0...7F) 0 = Pulse Width Manual 1..7F = PW LFO Rate
#define TONE_PARAM_VCF_CUTOFF                       0x10 //16 (0...7F)
#define TONE_PARAM_VCF_RESONANCE                    0x11 //17 (0...7F)
#define TONE_PARAM_VCF_LFO_MOD                      0x12 //18 (0...7F)
#define TONE_PARAM_VCF_ENV_MOD                      0x13 //19 (0...7F)
#define TONE_PARAM_VCF_KEY_FOLLOW                   0x14 //20 (0...7F)
#define TONE_PARAM_VCF_AFTERTOUCH_MOD               0x15 //21 (0...7F)
#define TONE_PARAM_VCA_LEVEL                        0x16 //22 (0...7F)
#define TONE_PARAM_VCA_AFTERTOUCH_MOD               0x17 //23 (0...7F)
#define TONE_PARAM_LFO_RATE                         0x18 //24 (0...7F)
#define TONE_PARAM_LFO_DELAY                        0x19 //25 (0...7F)
#define TONE_PARAM_ENV_T1                           0x1A //26 (0...7F) Attack time
#define TONE_PARAM_ENV_L1                           0x1B //27 (0...7F) Attack level
#define TONE_PARAM_ENV_T2                           0x1C //28 (0...7F) Break time
#define TONE_PARAM_ENV_L2                           0x1D //29 (0...7F) Break level
#define TONE_PARAM_ENV_T3                           0x1E //30 (0...7F) Decay time
#define TONE_PARAM_ENV_L3                           0x1F //31 (0...7F) Sustain level
#define TONE_PARAM_ENV_T4                           0x20 //32 (0...7F) Release time
#define TONE_PARAM_ENV_KEY_FOLLOW                   0x21 //33 (0...7F)
#define TONE_PARAM_CHORUS_RATE                      0x22 //34 (0...7F)
#define TONE_PARAM_BEND_RANGE                       0x23 //35 (0...0C)

// Parameter values
// These are not available on the Alpha Juno 1!
#define PARAM_GROUP_PATCH                           0x30

#define PATCH_PARAM_PORTAMENTO_TIME                 5 // CC Controller Number
#define PATCH_PARAM_PORTAMENTO_SWITCH               65 // CC Controller Number
#define PATCH_PARAM_MOD_SENSITIVITY                 0x05 //5  (0...0x7F)
#define PATCH_PARAM_TRANSPOSE                       0x06 //6  (0...0x0C)
#define PATCH_PARAM_MASTER_VOLUME                   7 // CC Controller Number
#define PATCH_PARAM_FINE_TUNE                       0x08 //8  (0...0x3F=0...63, 0x41...7F=-63...-1)
#define PATCH_PARAM_MONO_BEND_RANGE                 0x0A //10 (0...0C)
#define PATCH_PARAM_CHORD_MEMORY                    0x0B //11 (0...0F) 0=Mono
#define PATCH_PARAM_POLYPHONY_SWITCH                126 // CC Controller Number

// Control values
#define CONTROL_MAX_VALUE                            127
#define ENV_MODE_MAX_VALUE                           3
#define DCO_OCTAVE_MAX_VALUE                         3
#define DCO_NOISE_LEVEL_MAX_VALUE                    3
#define DCO_WAVEFORM_PULSE_MAX_VALUE                 3
#define DCO_WAVEFORM_SAW_MAX_VALUE                   5
#define DCO_WAVEFORM_SUB_MAX_VALUE                   5
#define DCO_SUB_LEVEL_MAX_VALUE                      3
#define CHORD_MEMORY_MAX_VALUE                       15
#define BEND_RANGE_MAX_VALUE                         11
#define TRANSPOSE_MAX_VALUE                          24
#define FINE_TUNE_MAX_VALUE                          126
#define HPF_MAX_VALUE                                3

// Step values
#define ENV_MODE_NORMAL                             0
#define ENV_MODE_INVERTED                           1
#define ENV_MODE_NORMAL_DYNAMIC                     2
#define ENV_MODE_INVERTED_DYNAMIC                   3

#define ENV_MODE_GATE                               1
#define ENV_MODE_GATE_DYNAMIC                       3

#define DCO_PULSE_WAVEFORM_OFF                      0
#define DCO_PULSE_WAVEFORM_1                        1
#define DCO_PULSE_WAVEFORM_2                        2
#define DCO_PULSE_WAVEFORM_3                        3

#define DCO_SAW_WAVEFORM_OFF                        0
#define DCO_SAW_WAVEFORM_1                          1
#define DCO_SAW_WAVEFORM_2                          2
#define DCO_SAW_WAVEFORM_3                          3
#define DCO_SAW_WAVEFORM_4                          4
#define DCO_SAW_WAVEFORM_5                          5

#define DCO_SUB_WAVEFORM_1                          0
#define DCO_SUB_WAVEFORM_2                          1
#define DCO_SUB_WAVEFORM_3                          2
#define DCO_SUB_WAVEFORM_4                          3
#define DCO_SUB_WAVEFORM_5                          4
#define DCO_SUB_WAVEFORM_6                          5

#define POLYPHONY_CHORD                             0x40
#define POLYPHONY_POLY                              0

// Define multiplexers
#define NUMBER_OF_MULTIPLEXER_CHANNELS             	8
#define NUMBER_OF_MULTIPLEXERS                     	5

#define MULTIPLEXER_CHANNEL_CONTROL_PIN_1          	2
#define MULTIPLEXER_CHANNEL_CONTROL_PIN_2          	3
#define MULTIPLEXER_CHANNEL_CONTROL_PIN_3          	4

// Mapping pins for multiplexers
#define ANALOG_INPUT_PIN_1                         	23
#define ANALOG_INPUT_PIN_2                          22
#define ANALOG_INPUT_PIN_3                          21
#define ANALOG_INPUT_PIN_4                          20
#define ANALOG_INPUT_PIN_5                          19

// These switches are not bound to actual parameters. They provide additional funtionality.
#define DIGITAL_INPUT_PIN_DUMP_CONTROLS_BUTTON      11
#define DIGITAL_INPUT_PIN_MOD_WHEEL_ASSIGN_BUTTON   12
#define DIGITAL_INPUT_PIN_MOD_PEDAL_ASSIGN_BUTTON   14
#define DIGITAL_INPUT_PIN_VCA_HOLD                  15

// "Digital parameter switches" refers to the switches that are bound to an actual parameter on the synth
#define NUMBER_OF_DIGITAL_PARAMETER_SWITCHES        3
#define DIGITAL_INPUT_PIN_CHORUS_SWITCH             16
#define DIGITAL_INPUT_PIN_PORTAMENTO_SWITCH         17
#define DIGITAL_INPUT_PIN_POLYPHONY_SWITCH          18

#define MIDI_INDICATOR_LED                          6
#define PWM_ACTIVE_INDICATOR_LED                    5

// Midi setup
#define DEFAULT_MIDI_CHANNEL                        1
#define MOD_WHEEL_PARAM                             1
#define MOD_PEDAL_PARAM                             4
#define SUSTAIN_PEDAL_PARAM                         64

// Sets analog read resolution to 10-bit (0-1023)
#define ANALOG_READ_RESOLUTION             	        10
#define MAX_ANALOG_READ_VALUE                       1024

#define MUX_CHANNEL_SWITCH_DELAY_IN_MICROSECONDS    100
#define BUTTON_DEBOUNCE_DELAY                       200

// Structure to store control parameters
struct ControlPin {
  int pin;
  int paramGroup;
  int param;
  int maxValue;
  
  String controlName;
  String LCDtext;
  
  int jitterThreshold;

  int previousAnalogReading;
  int currentAnalogReading;
  
  int previousValue;
  int currentValue;
};

// Structure for LCD display values
struct LCDDisplay {
  String currentControlName;
  String currentValue;
};

#endif
