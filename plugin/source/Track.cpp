#include "E3Seq/Track.h"

namespace Sequencer {

// TODO: setStepAtIndex will cause a data race between the message thread and
// the audio thread, better have a (non-blocking) solution for that
// refer to Dave Rowland real time ADC talk

int Track::getCurrentStepIndex() const {
  return (tick_ + HALF_STEP_TCIKS) / TICKS_PER_STEP;
}

// insert a future MIDI message into the MIDI buffer based on its timestamp
void Track::renderMidiMessage(juce::MidiMessage message) {
  int tick = static_cast<int>(message.getTimeStamp());
  if (tick < trackLength_ * TICKS_PER_STEP - HALF_STEP_TCIKS) {
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

    // render the step just right before its note on time
    // render it even earlier should be fine but
    if (tick_ == getStepRenderTick(index)) {
      renderStep(index);
    }

    // send current tick's MIDI events
    for (int i = firstRun_.getNextIndexAtTime(tick_);
         i < firstRun_.getNextIndexAtTime(tick_ + 1); i++) {
      auto midi_event = firstRun_.getEventPointer(i);
      sendMidiMessage(midi_event->message);
    }
  }

  // advance ticks and overwrap from (length-0.5) to (-0.5) step
  // because the first step could start from negative steps
  tick_ += 1;
  if (tick_ == trackLength_ * TICKS_PER_STEP - HALF_STEP_TCIKS) {
    tick_ = -HALF_STEP_TCIKS;
    // move second run into first run
    firstRun_.swapWith(secondRun_);
    secondRun_.clear();
  }
}

}  // namespace Sequencer