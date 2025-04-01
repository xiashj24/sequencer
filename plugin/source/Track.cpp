#include "E3Seq/Track.h"

namespace Sequencer {

// Warning: data race if setStepAtIndex() and tick() are called from different
// threads

int Track::getCurrentStepIndex() const {
  return (tick_ + HALF_STEP_TICKS) / TICKS_PER_STEP;
}

void Track::renderNote(int index, Note note) {
  if (note.number <= DISABLED_NOTE)
    return;

  int note_on_tick = static_cast<int>((index + note.offset) * TICKS_PER_STEP);
  int note_off_tick =
      static_cast<int>((index + note.offset + note.length) * TICKS_PER_STEP);

  // force note off before the next note on of the same note
  // search all midi messages after note_on_tick
  // if there is a note off with the same note number
  // delete that and insert a new note off at note_on_tick

  bool note_off_deleted = false;
  for (int i = firstRun_.getNextIndexAtTime(tick_);
       i < firstRun_.getNumEvents(); ++i) {
    auto message = firstRun_.getEventPointer(i)->message;
    if (message.isNoteOff() && message.getNoteNumber() == note.number) {
      firstRun_.deleteEvent(i, false);
      note_off_deleted = true;
    }
  }

  if (!note_off_deleted) {  // search second run
    for (int i = 0; i < secondRun_.getNumEvents(); ++i) {
      auto message = secondRun_.getEventPointer(i)->message;
      if (message.isNoteOff() && message.getNoteNumber() == note.number) {
        secondRun_.deleteEvent(i, false);
        note_off_deleted = true;
      }
    }
  }

  if (note_off_deleted) {
    juce::MidiMessage early_note_off_message = juce::MidiMessage::noteOff(
        getChannel(), note.number, (juce::uint8)note.velocity);
    early_note_off_message.setTimeStamp(note_on_tick);  // -1?
    renderMidiMessage(early_note_off_message);
  }

  // note on
  juce::MidiMessage note_on_message = juce::MidiMessage::noteOn(
      getChannel(), note.number, (juce::uint8)note.velocity);
  note_on_message.setTimeStamp(note_on_tick);
  renderMidiMessage(note_on_message);

  // note off
  juce::MidiMessage note_off_message = juce::MidiMessage::noteOff(
      getChannel(), note.number, (juce::uint8)note.velocity);
  note_off_message.setTimeStamp(note_off_tick);
  renderMidiMessage(note_off_message);
}

// insert a future MIDI message into the MIDI buffer based on its timestamp
void Track::renderMidiMessage(juce::MidiMessage message) {
  int tick = static_cast<int>(message.getTimeStamp());

  if (tick < trackLength_ * TICKS_PER_STEP - HALF_STEP_TICKS) {
    firstRun_.addEvent(message);

  } else {
    message.setTimeStamp(tick - trackLength_ * TICKS_PER_STEP);
    secondRun_.addEvent(message);
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

    // render the step just right before it's too late
    if (tick_ == getStepRenderTick(index)) {
      renderStep(index);
    }

    // send current tick's MIDI events
    for (int i = firstRun_.getNextIndexAtTime(tick_);
         i < firstRun_.getNextIndexAtTime(tick_ + 1); ++i) {
      auto message = firstRun_.getEventPointer(i)->message;

      // do not note off if held by the keyboard
      if (message.isNoteOff() &&
          keyboardRef.isNoteOn(message.getNoteNumber())) {
        continue;
      }

      sendMidiMessage(message);
    }
  }

  // advance ticks and overwrap from (length-0.5) to (-0.5) step
  // because the first step could start from negative steps
  tick_ += 1;
  if (tick_ == trackLength_ * TICKS_PER_STEP - HALF_STEP_TICKS) {
    tick_ = -HALF_STEP_TICKS;
    // move second run into first run
    firstRun_.swapWith(secondRun_);
    secondRun_.clear();
  }
}

}  // namespace Sequencer