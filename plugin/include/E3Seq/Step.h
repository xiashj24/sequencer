#pragma once

#define DEFAULT_NOTE 72       // C4
#define DEFAULT_VELOCITY 100  // 1..127 since 0 is the same as NoteOff
#define DEFAULT_GATE 0.75

// #define MAX_POLYPHONY 8
// tips: use a fixed length noteData[MAX_POLYPHONY] when std::vector is not available
// reference: https://chatgpt.com/share/67be979a-48d4-800a-bab9-1e7c5f8ea9d9



// TODO: put every parameter min and max into here

namespace Sequencer {

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

// TODO: use Note[polyphony] inside Step
struct Note {
  int number;
  double length;
  int velocity;
  double offset;
};

struct Step {
  int note = DEFAULT_NOTE;
  double length = DEFAULT_GATE;   // note: length can be greater than 1 but should
                                // be smaller than track length
  bool tie = false;             // or use something like length > MAX_GATE ?
                                // tie should only be used for mono tracks
  int velocity = DEFAULT_VELOCITY;
  double offset = 0.0;

  int roll = 1;
  double probability = 1.0;
  int alternate = 1;
  int count = 0;

  bool enabled = false;
};

}  // namespace Sequencer
