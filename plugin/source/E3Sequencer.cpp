#include "E3Seq/E3Sequencer.h"

void E3Sequencer::tick(NoteEvent* noteOn,
                       NoteEvent* noteOff,
                       ControlChangeEvent* cc) {
  if (!running_)
    return;

  float one_step_time = 60.f / bpm_;
  [[maybe_unused]]float one_micro_step_time = one_step_time / RESOLUTION;

  // TODO: implement backward/bounce sequencing
  // this might be difficult since note length can extend well beyond one step

  // for each track
  for (auto track : tracks_) {
    if (track.isEnabled()) {
      // for all steps in the track (because steps can last very long)
      for (int i = 0; i < track.getLength(); i++) {
        // generate a NoteEvent if NoteOn/NoteOff time lies in
        // [time, time + tickTime)
        auto step = track[i];
        if (step.enabled) {
          auto note_on_time = ((float)i + step.offset) * one_step_time;
          auto note_off_time = note_on_time + step.gate * one_step_time;
          // TODO: noteOn timing quantization
          if (note_on_time >= time_ && note_on_time < time_ + tickTime_) {
            NoteEvent note_on_event(track, step);
            note_on_event.time_since_last_tick = note_on_time - time_;

            if (noteOn) {
              *noteOn = note_on_event;
            }
          }

          if (note_off_time > time_ && note_off_time < time_ + tickTime_) {
            NoteEvent note_off_event(track, step);
            note_off_event.time_since_last_tick = note_off_time - time_;

            if (noteOff) {
              *noteOff = note_off_event;
            }
          }
        }
      }
    }
  }

  if (cc) {
    // For now, no CC events are generated
  }

  // time update
  time_ += tickTime_;  // this might be prone to precision loss after
                       // running for a long time, overwrap?

  return;
}