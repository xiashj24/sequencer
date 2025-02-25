#pragma once

#define DEFAULT_NOTE 72       // C4
#define DEFAULT_VELOCITY 100  // 1..127 since 0 is the same as NoteOff
#define DEFAULT_GATE 0.75

#define OFFSET_MIN -0.5
#define OFFSET_MAX 0.49  // TODO: use this

namespace Sequencer {

// TODO: show offset names on UI labels
enum class Offset {
  _1o384,
  _1o192,
  _1o96,
  _5o384,
  _1o64,
  _7o384,
  _1o48,
  _3o128,
  _5o192,
  _11o384,
  _1o32
};

// this is just a dat
struct Step {
  int note = DEFAULT_NOTE;
  double gate = DEFAULT_GATE;  // note: gate can be greater than 1 but should
                               // be smaller than track length
  bool tie = false;            // or use something like gate > MAX_GATE ?

  int velocity = DEFAULT_VELOCITY;
  double offset = 0.0;

  int roll = 1;
  double probability = 1.0;
  int alternate = 1;
  int count = 0;

  bool enabled = false;
};

}  // namespace Sequencer
