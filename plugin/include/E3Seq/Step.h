#pragma once

#define DEFAULT_NOTE 72       // C4
#define DEFAULT_VELOCITY 100  // 1..127 since 0 is the same as NoteOff
#define DEFAULT_LENGTH 0.75f

#define POLYPHONY 8

namespace Sequencer {

struct Step {
  bool enabled = false;

  int note = DEFAULT_NOTE;
  int velocity = DEFAULT_VELOCITY;
  float offset = 0.f;
  float length = DEFAULT_LENGTH;  // note: can be greater than 1 but
                                  // should be smaller than track length

  float retrigger_rate = 0.f;  // 0 (and minus values) means no retrigger
  float probability = 1.f;
  int alternate = 1;
  int count = 0;
};

struct Note {
  int number;
  int velocity;
  float offset;
  float length;
};

struct PolyStep {
  bool enabled = false;

  Note notes[POLYPHONY];

  // note: no retrigger
  
  float probability = 1.f;
  int alternate = 1;
  int count = 0;
};

}  // namespace Sequencer
