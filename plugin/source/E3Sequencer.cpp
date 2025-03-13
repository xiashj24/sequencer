#include "E3Seq/E3Sequencer.h"
#include <cmath>

// this class is the interface between the underlying sequencer logic and the
// outside app framework. It uses information such as BPM and MIDI clock to
// translate real world time to the "ticks" used by the sequencer
// ideally users of this class should not need to know about ticks at all

namespace Sequencer {

E3Sequencer::E3Sequencer(juce::MidiMessageCollector& midiCollector, double bpm)
    : bpm_(bpm),
      running_(false),
      armed_(false),
      timeSinceStart_(0.0),
      startTime_(0.0),
      midiCollector_(midiCollector) {
  // MARK: track config
  for (int i = 0; i < STEP_SEQ_NUM_TRACKS; i++) {
    getTrack(i).setChannel(i + 1);
    getTrack(i).sendMidiMessage = [this](juce::MidiMessage msg) {
      // time translation
      int tick = (int)msg.getTimeStamp();
      double real_time_stamp = startTime_ + getOneTickTime() * tick;
      // this time translation code is smelling....need some further thinking
      // TODO: is it possible to implement note stealing via a simple queue?
      this->midiCollector_.addMessageToQueue(
          msg.withTimeStamp(real_time_stamp));
    };
  }
}

void E3Sequencer::process(double deltaTime) {
  if (!running_)
    return;

  // time update
  // note: deltaTime should be much smaller than oneTickTime or precision will
  // be bad
  timeSinceStart_ += deltaTime;
  double one_tick_time = getOneTickTime();

  if (timeSinceStart_ >= one_tick_time) {
    for (auto& track : tracks_) {
      track.tick();  // tick and collect midi events from each track
    }

    // TODO: also send MIDI clock, or just pass through incoming MIDI clock?
    timeSinceStart_ -= one_tick_time;
  }

  return;
}

void E3Sequencer::start(double startTime) {
  running_ = true;
  timeSinceStart_ = 0.0;
  startTime_ = startTime;
  for (auto& track : tracks_) {
    track.returnToStart();
  }
}

void E3Sequencer::handleNoteOn(juce::MidiMessage noteOn) {
  if (this->isArmed() && this->isRunning()) {
    int channel = noteOn.getChannel();
    int step_index = getTrack(channel - 1).getCurrentStepIndex();
    lastNoteOnEachKey_[step_index].emplace(noteOn);
  }
}

void E3Sequencer::handleNoteOff(juce::MidiMessage noteOff) {
  // find corresponding note on
  int note = noteOff.getNoteNumber();
  int channel = noteOff.getChannel();
  for (int index = 0; index < STEP_SEQ_MAX_LENGTH; ++index) {
    if (lastNoteOnEachKey_[index].has_value()) {
      auto noteOn = lastNoteOnEachKey_[index].value();
      if (noteOn.getNoteNumber() == note && noteOn.getChannel() == channel) {
        // calculate offset and length
        auto offset = (noteOn.getTimeStamp() - startTime_) / getOneStepTime();
        offset -= std::round(offset);  // wrap in [-0.5, 0.5)

        auto length =
            (noteOff.getTimeStamp() - noteOn.getTimeStamp()) / getOneStepTime();
        length = std::min(length, static_cast<double>(STEP_SEQ_MAX_LENGTH));

        getTrack(channel - 1)
            .setStepAtIndex(index, {.enabled = true,
                                    .note = note,
                                    .velocity = noteOn.getVelocity(),
                                    .offset = static_cast<float>(offset),
                                    .length = static_cast<float>(length)});
        lastNoteOnEachKey_[index].reset();
      }
    }
  }
}
}  // namespace Sequencer
