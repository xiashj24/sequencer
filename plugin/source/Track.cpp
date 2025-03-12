#include "E3Seq/Track.h"

namespace Sequencer {

// TODO: setStepAtIndex will cause a data race between the message thread and
// the audio thread, better have a (non-blocking) solution for that

int Track::getCurrentStepIndex() const {
  return (tick_ + half_step_ticks) / TICKS_PER_STEP;
}

int Track::getStepNoteOnTick(int index) const {
  return static_cast<int>((index + steps_[index].offset) * TICKS_PER_STEP);
}

int Track::getStepNoteOffTick(int index) const {
  return static_cast<int>(
      (index + steps_[index].offset + steps_[index].length) * TICKS_PER_STEP);
}

// insert a future MIDI message into the MIDI buffer based on its timestamp
void Track::renderMidiMessage(juce::MidiMessage message) {
  int tick = static_cast<int>(message.getTimeStamp());
  if (tick < trackLength_ * TICKS_PER_STEP - half_step_ticks) {
    firstRun_.addEvent(message);
  } else {
    message.setTimeStamp(tick - trackLength_ * TICKS_PER_STEP);
    secondRun_.addEvent(message);
  }
}

void Track::renderStep(int index) {
  auto& step = steps_[index];
  if (step.enabled) {
    // alternate check
    if ((step.count++) % step.alternate != 0) {
      return;
    }

    // probability check
    if (juce::Random::getSystemRandom().nextFloat() >= step.probability) {
      return;
    }

    int note_on_tick = getStepNoteOnTick(index);
    int note_off_tick = getStepNoteOffTick(index);

    // force note off before the next active step if legato mode is off
    if (!legato_)
    {
      // TODO:
      while (getStepAtIndex((++index) % trackLength_).enabled == false);
      auto next_active_step = getStepAtIndex(index); // do not re use index
    }

    // note on message should always go to the first run
#if JUCE_DEBUG
    jassert(note_on_tick < trackLength_ * TICKS_PER_STEP - half_step_ticks);
#endif

    // note on
    juce::MidiMessage note_on_message = juce::MidiMessage::noteOn(
        channel_, step.note, (juce::uint8)step.velocity);
    note_on_message.setTimeStamp(note_on_tick);
    renderMidiMessage(note_on_message);

    // retrigger
    if (step.retrigger_interval > 0.0) {
      int retrigger_interval_in_ticks =
          static_cast<int>(step.retrigger_interval * TICKS_PER_STEP);
      for (int tick = note_on_tick + retrigger_interval_in_ticks;
           tick < note_off_tick; tick += retrigger_interval_in_ticks) {
        juce::MidiMessage retrigger_note_off_message =
            juce::MidiMessage::noteOff(channel_, step.note,
                                       (juce::uint8)step.velocity);
        retrigger_note_off_message.setTimeStamp(tick);
        juce::MidiMessage retrigger_note_on_message = juce::MidiMessage::noteOn(
            channel_, step.note, (juce::uint8)step.velocity);
        retrigger_note_on_message.setTimeStamp(tick);
        renderMidiMessage(retrigger_note_off_message);
        renderMidiMessage(retrigger_note_on_message);
      }
    }

    // note off
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
    int index = getCurrentStepIndex();
    int note_on_tick = getStepNoteOnTick(index);

    if (note_on_tick == tick_) {
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
  if (tick_ == trackLength_ * TICKS_PER_STEP - half_step_ticks) {
    tick_ = -half_step_ticks;
    // move the second run's Midi events to the first run
    firstRun_.swapWith(secondRun_);
    secondRun_.clear();
  }
}

}  // namespace Sequencer