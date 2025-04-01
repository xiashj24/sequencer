#pragma once
#include <cmath>

#define DEFAULT_NOTE 60  // C4
#define DISABLED_NOTE 20
#define STOLEN_NOTE 19
#define DEFAULT_VELOCITY 100  // 1..127 since 0 is the same as NoteOff
#define DEFAULT_LENGTH 0.75f

#define POLYPHONY 4  // make this a static const member of PolyStep

namespace Sequencer {

struct Note {
  int number = DEFAULT_NOTE;  // for now we use the convention that note number
                              // <= 20 indicates disabled
  int velocity = DEFAULT_VELOCITY;
  float offset = 0.f;             // relative the step index
  float length = DEFAULT_LENGTH;  // in (0, TrackLength]

  void reset() {
    number = DISABLED_NOTE;
    velocity = DEFAULT_VELOCITY;
    offset = 0.f;  // relative the step index
    length = DEFAULT_LENGTH;
  }
};

struct MonoStep {
  bool enabled = false;
  Note note;

  float retrigger_rate = 0.f;  // 0 (and minus values) means no retrigger
  float probability = 1.f;
  int alternate = 1;
  int count = 0;
};

// TODO: poly step is a bit more complicated, so it needs to have better
// encapsulation
struct PolyStep {
  bool enabled = false;
  float probability = 1.0;  // should you keep this?
  Note notes[POLYPHONY];

  void reset() {
    enabled = false;
    probability = 1.0;
    for (auto& note : notes) {
      note.reset();
    }
    notes[0].number = DEFAULT_NOTE;
  }

  void align(int velocity = DEFAULT_VELOCITY,
             float offset = 0.f,
             float length = DEFAULT_LENGTH) {
    for (auto& note : notes) {
      note.velocity = velocity;
      note.offset = offset;
      note.length = length;
    }
  }

  bool isEmpty() const {
    bool is_empty = true;
    for (auto note : notes) {
      if (note.number != DISABLED_NOTE && note.number != STOLEN_NOTE) {
        is_empty = false;
      }
    }
    return is_empty;
  }

  // MARK: note stealing
  // noteNumber should not be interrupted by this step
  // TODO: take offset into consideration
  // invariant: after this call, the number of sloten note should increase by 1
  void stealNote(int noteNumber) {
    // note stealing policy:
    // replace same note -> occupy vacant(disabled) slot -> replace closest
    // note (if there are 2 closest note just choose a random one)
    for (auto& note : notes) {
      if (note.number == noteNumber) {
        note.number = STOLEN_NOTE;
        if (isEmpty()) {
          reset();
        }
        return;
      }
    }

    for (auto& note : notes) {
      if (note.number == DISABLED_NOTE) {
        note.number = STOLEN_NOTE;
        if (isEmpty()) {
          reset();
        }
        return;
      }
    }

    int closest_index = 0;
    int closest_distance = 127;
    for (int i = 0; i < POLYPHONY; ++i) {
      int distance = std::abs(notes[i].number - noteNumber);
      if (distance <= closest_distance) {
        closest_distance = distance;
        closest_index = i;
      }
    }

    notes[closest_index].number = STOLEN_NOTE;

    if (isEmpty()) {
      reset();
    }
  }

  void addNote(Note new_note) {
    if (!enabled) {  // when enabled through live rec
      reset();
      notes[0] = new_note;
      align(new_note.velocity, new_note.offset, new_note.length);
      enabled = true;
      return;
    };

    // similar to note stealing policy
    for (auto& note : notes) {
      if (note.number == new_note.number) {
        note = new_note;
        return;
      }
    }

    for (auto& note : notes) {
      if (note.number <= DISABLED_NOTE) {
        note = new_note;
        return;
      }
    }

    int closest_index = 0;
    int closest_distance = 127;
    for (int i = 0; i < POLYPHONY; ++i) {
      int distance = std::abs(notes[i].number - new_note.number);
      if (distance <= closest_distance) {
        closest_distance = distance;
        closest_index = i;
      }
    }

    notes[closest_index] = new_note;
  }

  PolyStep() { reset(); }
};

}  // namespace Sequencer
