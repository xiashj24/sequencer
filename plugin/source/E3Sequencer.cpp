#include "E3Seq/E3Sequencer.h"

#include <cmath>

namespace Sequencer {

void E3Sequencer::process(NoteEvent* noteOn,
                       NoteEvent* noteOff,
                       ControlChangeEvent* cc) {
  if (!running_)
    return;

  double one_step_time =
      60.0 / bpm_ /
      4.0;  // TODO: implement time signature (refer to JUCE documentation)
  [[maybe_unused]] double one_micro_step_time = one_step_time / RESOLUTION;

  // TODO: implement backward/bounce sequencing
  // this might be difficult since note length can extend well beyond one step

  // for each track
  for (auto track : tracks_) {
    if (track.isEnabled()) {
      // for all steps in the track (because steps can last very long)
      auto track_loop_time = track.getLength() * one_step_time;
      auto local_time = std::fmod(time_, track_loop_time);
      // TODO: visualize playback position using local_time
      for (int i = 0; i < track.getLength(); i++) {
        // generate a NoteEvent if NoteOn/NoteOff time lies in
        // [time, time + tickTime)
        auto step = track[i];
        if (step.enabled) {
          auto note_on_time =
              ((double)i + step.offset) * one_step_time +
              oneTickTime_;  // add 1-tick delay because for the second loop local
                          // time will not start exactly from 0. Hopefully this
                          // will not cause problems such as missing noteOff :)
          auto note_off_time = note_on_time + step.gate * one_step_time;

          // note_on_time and note_off_time could be negative
          if (note_on_time < 0) {
            note_on_time += track_loop_time;
          }
          if (note_off_time < 0) {
            note_off_time += track_loop_time;
          }

          // TODO: noteOn timing quantization
          if (note_on_time >= local_time &&
              note_on_time < local_time + oneTickTime_) {
            NoteEvent note_on_event(track, step);
            note_on_event.time_since_last_tick = note_on_time - local_time;

            if (noteOn) {
              *noteOn = note_on_event;
            }
          }

          if (note_off_time > local_time &&
              note_off_time < local_time + oneTickTime_) {
            NoteEvent note_off_event(track, step);
            note_off_event.time_since_last_tick = note_off_time - local_time;

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
  time_ += oneTickTime_;  // note: this might suffer from low precision after
                       // running for a long time, need further testing

  return;
}

}  // namespace Sequencer
