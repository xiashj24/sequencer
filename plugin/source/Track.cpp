#include "E3Seq/Track.h"

namespace Sequencer {

// TODO: setStepAtIndex will cause data race between the message thread and
// real-time thread
// better have a solution for that

void Track::renderMidiMessage(juce::MidiMessage message) {
  int tick = static_cast<int>(message.getTimeStamp());
  if (tick < length_ * TICKS_PER_STEP - half_step_ticks) {
    firstRun_.addEvent(message);
  } else {
    message.setTimeStamp(tick - length_ * TICKS_PER_STEP);
    secondRun_.addEvent(message);
  }
}

void Track::renderStep(int index) {
  auto step = steps_[index];
  if (step.enabled) {
    // probability check
    if (juce::Random::getSystemRandom().nextDouble() >= step.probability) {
      return;
    }

    int note_on_tick = static_cast<int>((index + step.offset) * TICKS_PER_STEP);
    int note_off_tick =
        static_cast<int>((index + step.offset + step.gate) * TICKS_PER_STEP);

    // NoteOn message should always go to the first run
#if JUCE_DEBUG
    jassert(note_on_tick < length_ * TICKS_PER_STEP - half_step_ticks);
#endif

    juce::MidiMessage note_on_message = juce::MidiMessage::noteOn(
        channel_, step.note, (juce::uint8)step.velocity);
    note_on_message.setTimeStamp(note_on_tick);
    renderMidiMessage(note_on_message);

    for (int i = 1; i < step.roll; i++) {
      int roll_tick =
          note_on_tick + static_cast<int>((note_off_tick - note_on_tick) *
                                          ((double)i / step.roll));

      juce::MidiMessage roll_note_off_message = juce::MidiMessage::noteOff(
          channel_, step.note, (juce::uint8)step.velocity);
      roll_note_off_message.setTimeStamp(roll_tick);
      juce::MidiMessage roll_note_on_message = juce::MidiMessage::noteOn(
          channel_, step.note, (juce::uint8)step.velocity);
      roll_note_on_message.setTimeStamp(roll_tick);
      renderMidiMessage(roll_note_off_message);
      renderMidiMessage(roll_note_on_message);
    }

    juce::MidiMessage note_off_message = juce::MidiMessage::noteOff(
        channel_, step.note, (juce::uint8)step.velocity);
    note_off_message.setTimeStamp(note_off_tick);
    renderMidiMessage(note_off_message);
  }
}

void Track::returnToStart() {
  firstRun_.clear();
  secondRun_.clear();
  tick_ = 0;
}

void Track::tick() {
  if (this->enabled_) {
    int index = (tick_ + half_step_ticks) / TICKS_PER_STEP;
    int note_on_tick_of_current_step =
        static_cast<int>((index + steps_[index].offset) * TICKS_PER_STEP);

    if (note_on_tick_of_current_step == tick_) {
      renderStep(index);
    }

    // send current tick's MIDI events
    for (int i = firstRun_.getNextIndexAtTime(tick_);
         i < firstRun_.getNextIndexAtTime(tick_ + 1); i++) {
      auto midi_event = firstRun_.getEventPointer(i);
      sendMidiMessage(midi_event->message);
    }
  }

  // advance ticks and wrap up from (length-0.5) to (-0.5) step
  // because the first step could start from negative ticks
  tick_ += 1;
  if (tick_ == length_ * TICKS_PER_STEP - half_step_ticks) {
    tick_ = -half_step_ticks;
    // move the second run's Midi events to the first run
    firstRun_.swapWith(secondRun_);
    secondRun_.clear();
  }
}

}  // namespace Sequencer