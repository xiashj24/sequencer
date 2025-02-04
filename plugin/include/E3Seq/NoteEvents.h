#pragma once
#include "Step.h"
#include "Track.h"

namespace Sequencer {

struct NoteEvent {
  bool enabled;
  int note;
  int velocity;
  int channel;
  double time_since_last_tick;

  NoteEvent()
      : enabled(false),
        note(DEFAULT_NOTE),
        velocity(DEFAULT_VELOCITY),
        channel(1),
        time_since_last_tick(0.0) {}

  NoteEvent(Step step)
      : enabled(true),
        note(step.note),
        velocity(step.velocity),
        channel(1),
        time_since_last_tick(0.0) {}

  NoteEvent(Track track, Step step)
      : enabled(true),
        note(step.note),
        velocity(step.velocity),
        channel(track.getChannel()),
        time_since_last_tick(0.0) {}
};

}  // namespace Sequencer