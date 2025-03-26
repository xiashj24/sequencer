#pragma once

#define DEFAULT_NOTE 60  // C4
#define DISABLED_NOTE 20
#define DEFAULT_VELOCITY 100  // 1..127 since 0 is the same as NoteOff
#define DEFAULT_LENGTH 0.75f

#define POLYPHONY 4

namespace Sequencer {

struct Note {
  int number = DEFAULT_NOTE;  // for now we use the convention that note number
                              // <= 20 indicates disabled
  int velocity = DEFAULT_VELOCITY;
  float offset = 0.f;             // relative the step index
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
  float probability = 1.0;
  // note: no retrigger or alternate for poly step
  Note notes[POLYPHONY];

  int note_write_index = 0;  // should you move this outside of PolyStep?

  void reset(int velocity = DEFAULT_VELOCITY,
             float offset = 0.f,
             float length = DEFAULT_LENGTH) {
    enabled = false;
    probability = 1.0;
    note_write_index = 0;
    for (int i = 0; i < POLYPHONY; ++i) {
      notes[i] = Note{.number = DISABLED_NOTE,
                      .velocity = velocity,
                      .offset = offset,
                      .length = length};
    }
  }

  void alignWithNote(int index) {
    for (int i = 0; i < POLYPHONY; ++i) {
      if (i != index) {
        notes[i].velocity = notes[index].velocity;
        notes[i].offset = notes[index].offset;
        notes[i].length = notes[index].length;
      }
    }
  }

  void addNote(Note note) {
    if (enabled) {
      // same note replacement
      for (int i = 0; i < POLYPHONY; ++i) {
        if (notes[i].number == note.number) {
          notes[i] = note;
          // alignWithNote(i);
          return;
        }
      }
    } else {
      // when enabled throgh live rec, align other notes with the new note
      reset(note.velocity, note.offset, note.length);
    }
    // oldest note replacement
    notes[note_write_index] = note;
    // alignWithNote(note_write_index);
    note_write_index = (note_write_index + 1) % POLYPHONY;
    if (note_write_index == 0) {
      note_write_index = 1;  // do not allow overwrite notes[0] for now
    }
    enabled = true;
  }

  PolyStep() {
    for (int i = 0; i < POLYPHONY; ++i) {
      notes[i].number = 0;  // all notes disabled
    }
  }
};

}  // namespace Sequencer
