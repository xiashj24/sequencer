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
      quantizeRec_(false),
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
      this->midiCollector_.addMessageToQueue(
          msg.withTimeStamp(real_time_stamp));
    };
  }
}

void E3Sequencer::process(double deltaTime) {
  if (!running_)
    return;

  // note: deltaTime should be much smaller than oneTickTime
  timeSinceStart_ += deltaTime;
  double one_tick_time = getOneTickTime();

  if (timeSinceStart_ >= one_tick_time) {
    for (auto& track : tracks_) {
      track.tick();  // tick and collect midi events from each track
    }

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
  // ignore MIDI messages for channels > 8, will remove later
  if (noteOn.getChannel() > 8)
    return;

  if (this->isArmed() && this->isRunning()) {
    int channel = noteOn.getChannel();
    int step_index = getTrack(channel - 1).getCurrentStepIndex();
    lastNoteOn_[step_index].emplace(noteOn);
  }
}

void E3Sequencer::handleNoteOff(juce::MidiMessage noteOff) {
  // find corresponding note on
  int note = noteOff.getNoteNumber();
  int channel = noteOff.getChannel();
  for (int index = 0; index < STEP_SEQ_MAX_LENGTH; ++index) {
    if (lastNoteOn_[index].has_value()) {
      auto noteOn = lastNoteOn_[index].value();
      if (noteOn.getNoteNumber() == note && noteOn.getChannel() == channel) {
        // calculate offset and length

        double offset = 0.0;
        if (!quantizeRec_) {
          offset = (noteOn.getTimeStamp() - startTime_) / getOneStepTime();
          offset -= std::round(offset);  // wrap in [-0.5, 0.5)
        }

        auto length =
            (noteOff.getTimeStamp() - noteOn.getTimeStamp()) / getOneStepTime();
        length = std::min(length, static_cast<double>(STEP_SEQ_MAX_LENGTH));

        Step step{.enabled = true,
                  .note = note,
                  .velocity = noteOn.getVelocity(),
                  .offset = static_cast<float>(offset),
                  .length = static_cast<float>(length)};
        getTrack(channel - 1).setStepAtIndex(index, step);
        // notify AudioProcessor about parameter change
        notifyProcessor(channel - 1, index, step);
        lastNoteOn_[index].reset();
      }
    }
  }
}
}  // namespace Sequencer
