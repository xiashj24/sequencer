#pragma once

#define DEFAULT_NOTE 72       // C4
#define DEFAULT_VELOCITY 100  // 1..127 since 0 is the same as NoteOff
#define DEFAULT_LENGTH 0.75f

#define POLYPHONY 8

namespace Sequencer {

struct Note {
  int number = DEFAULT_NOTE;
  int velocity = DEFAULT_VELOCITY;
  float offset = 0.f;
  float length = DEFAULT_LENGTH;  // in (0, TrackLength]
};

struct MonoStep {
  bool enabled = false;

  Note note;

  float retrigger_rate = 0.f;  // 0 (and minus values) means no retrigger
  float probability = 1.f;
  int alternate = 1;
  int count = 0;
};

struct PolyStep {
  bool enabled = false;

  Note notes[POLYPHONY];

  // note: no retrigger for now
  // maybe interesting to have a arp generator inside polystep

  float probability = 1.f;
  int alternate = 1;  // I think only one of these will remain in poly step
  int count = 0;
};

}  // namespace Sequencer
