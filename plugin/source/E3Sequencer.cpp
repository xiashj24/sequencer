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
  for (int channel = 0; channel <= STEP_SEQ_NUM_TRACKS; ++channel) {
    Track& track = getTrack(channel);
    track.setChannel(channel);
    track.sendMidiMessage = [this](juce::MidiMessage msg) {
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

#ifdef JUCE_DEBUG
  jassert(deltaTime < getOneTickTime());
#endif

  timeSinceStart_ += deltaTime;
  double one_tick_time = getOneTickTime();

  if (timeSinceStart_ >= one_tick_time) {
    for (int channel = 1; channel <= STEP_SEQ_NUM_TRACKS; ++channel) {
      getTrack(channel).tick();
    }
    timeSinceStart_ -= one_tick_time;
  }

  return;
}

void E3Sequencer::start(double startTime) {
  running_ = true;
  timeSinceStart_ = 0.0;
  startTime_ = startTime;
  for (int channel = 1; channel <= STEP_SEQ_NUM_TRACKS; ++channel) {
    getTrack(channel).returnToStart();
  }
}

// TODO: test edge case
// what will hanppen when hold a note for more than one bar???
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
  int note_number = noteOff.getNoteNumber();
  int channel = noteOff.getChannel();

  if (channel <= STEP_SEQ_NUM_MONO_TRACKS) {
    for (int index = 0; index < STEP_SEQ_MAX_LENGTH; ++index) {
      if (lastNoteOn_[index].has_value()) {
        auto noteOn = lastNoteOn_[index].value();
        if (noteOn.getNoteNumber() == note_number &&
            noteOn.getChannel() == channel) {
          // calculate offset and length

          double offset = 0.0;
          if (!quantizeRec_) {
            offset = (noteOn.getTimeStamp() - startTime_) / getOneStepTime();
            offset -= std::round(offset);  // wrap in [-0.5, 0.5)
          }

          auto length = (noteOff.getTimeStamp() - noteOn.getTimeStamp()) /
                        getOneStepTime();
          length = std::min(length, static_cast<double>(STEP_SEQ_MAX_LENGTH));

          MonoStep step{.enabled = true,
                        .note.number = note_number,
                        .note.velocity = noteOn.getVelocity(),
                        .note.offset = static_cast<float>(offset),
                        .note.length = static_cast<float>(length)};
          getMonoTrack(channel - 1).setStepAtIndex(index, step);
          // notify AudioProcessor about parameter change
          notifyProcessor(channel - 1, index, step);
          lastNoteOn_[index].reset();
        }
      }
    }
    // } else {
    //   // TODO: handle live rec for poly tracks
    // }
  }
}
}  // namespace Sequencer
