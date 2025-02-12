#include "E3Seq/Track.h"

namespace Sequencer {

/*
deferred MIDI event delete if step_before is playing

note: there is no deferred add if note_after is currently playing
this might result in NoteOff out of nowhere, which can be safely ignored

to test: modidy a step's each parameter while sequencer is running and make
sure no noteOff is missing

also, be aware of race conditions when setStepAtIndex() is called from a
different thread from tick()

one property of oneLoopMidiSeq: MidiMessageSequence works on the premise that
MIDI events are always sorted by timestamp
*/

void Track::setStepAtIndex(int index, Step step) {
  // a simple buffering mechanism should prevent race conditions
  // if problems shall happen, do not hesitate to use atomic operation
  steps_buffer_[index] = step;
}

void Track::tick() {
  // write steps_buffer_ through to steps_ if current tick is not inside the
  // time window of step_[index]
  for (int index = 0; index < STEP_SEQ_MAX_LENGTH; index++) {
    if (steps_buffer_[index].has_value()) {  // if step[index] has been modified
      Step step_before = steps_[index];
      Step step_after = steps_buffer_[index].value();

      int step_before_note_on_tick =
          (int)((index + step_before.offset) * TICKS_PER_STEP);
      int step_before_note_off_tick =
          (int)((index + step_before.offset + step_before.gate) *
                TICKS_PER_STEP);

      int step_after_note_on_tick =
          (int)((index + step_after.offset) * TICKS_PER_STEP);
      int step_after_note_off_tick =
          (int)((index + step_after.offset + step_after.gate) * TICKS_PER_STEP);

      bool inside_step_before = (tick_ > step_before_note_on_tick &&
                                 tick_ <= step_before_note_off_tick);
      // TODO: tick_for_last_loop                           
      bool inside_step_after = (tick_ > step_after_note_on_tick &&
                                tick_ <= step_after_note_off_tick);

      // this is a rather strict condition, but it should guarantee no
      // mismatched NoteOn and NoteOff
      if (!inside_step_before && !inside_step_after) {
        // write through
        steps_[index] = step_after;
        steps_buffer_[index].reset();

        if (step_before.enabled) {
          // delete MidiEvents from oneLoopMidiSeq_
          for (int i =
                   oneLoopMidiSeq_.getNextIndexAtTime(step_before_note_on_tick);
               i <
               oneLoopMidiSeq_.getNextIndexAtTime(step_before_note_on_tick + 1);
               i++) {
            auto midiEvent = oneLoopMidiSeq_.getEventPointer(i);
            if (midiEvent->message.isNoteOn()) {
              oneLoopMidiSeq_.deleteEvent(i, true);  // TODO: polyphony
            }
          }
        }

        if (step_after.enabled) {
          // add NoteOn and NoteOff MidiEvents to oneLoopMidiSeq_
          juce::MidiMessage note_on_message = juce::MidiMessage::noteOn(
              channel_, step_after.note, (juce::uint8)step_after.velocity);
          note_on_message.setTimeStamp(step_after_note_on_tick);

          juce::MidiMessage note_off_message = juce::MidiMessage::noteOff(
              channel_, step_after.note, (juce::uint8)step_after.velocity);
          note_off_message.setTimeStamp(step_after_note_off_tick);

          oneLoopMidiSeq_.addEvent(note_on_message);
          oneLoopMidiSeq_.addEvent(note_off_message);
        }
      }
      oneLoopMidiSeq_
          .updateMatchedPairs();  // is it ok to just call this once outside?
    }
  }  // end step buffer write through

  // send out MIDI messages
  if (enabled_) {
    for (int i = oneLoopMidiSeq_.getNextIndexAtTime(tick_);
         i < oneLoopMidiSeq_.getNextIndexAtTime(tick_ + 1); i++) {
      auto midi_event = oneLoopMidiSeq_.getEventPointer(i);
      sendMidiMessage(midi_event->message);
    }
    // for noteOffs that are out of boundary
    int tick_for_last_loop = tick_ + length_ * TICKS_PER_STEP;
    for (int i = oneLoopMidiSeq_.getNextIndexAtTime(tick_for_last_loop);
         i < oneLoopMidiSeq_.getNextIndexAtTime(tick_for_last_loop + 1); i++) {
      auto midi_event = oneLoopMidiSeq_.getEventPointer(i);
      sendMidiMessage(midi_event->message);
    }
  }
  // advance ticks and wrap up from length-0.5 to -0.5 step
  // because the first step could start from negative ticks
  int half_step_ticks = TICKS_PER_STEP / 2;
  tick_ += 1;
  if (tick_ == length_ * TICKS_PER_STEP - half_step_ticks) {
    tick_ = -half_step_ticks;
  }
}

}  // namespace Sequencer