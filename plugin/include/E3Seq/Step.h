#pragma once

#define DEFAULT_NOTE 72       // C4
#define DEFAULT_VELOCITY 100  // 1..127 since 0 is the same as NoteOff
#define DEFAULT_GATE 0.75

namespace Sequencer {

struct Step {
  // step parameters as seen by the user
  int note = DEFAULT_NOTE;
  double gate = DEFAULT_GATE;  // note: gate can be greater than 1 but should
                               // be smaller than track length
  int velocity = DEFAULT_VELOCITY;
  double offset = 0.0;

  // double pitchbend = 0.0;
  int roll = 1;
  double probability = 1.0;
  int alternate = 1;

  // function-related variables
  bool enabled = false;
  bool tie = false;  // or use something like gate > MAX_GATE ?
};

}  // namespace Sequencer
