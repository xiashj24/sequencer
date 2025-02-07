#pragma once
#include "Step.h"
#include <juce_audio_basics/juce_audio_basics.h>    // juce::MidiMessageSequence
#include <juce_audio_devices/juce_audio_devices.h>  // juce::MidiMessageCollector

// I'm sorry I broke the rule of not using juce
// because juce::MidiMessageSequence is too good to not use for a sequencer

// TODO: per track swing

// polyphony will probably have to be implemented simply as {MAX_POLYPHONY} mono
// tracks use a separate assigner class to handle note stealing and overdubbing
// between different tracks

#define STEP_SEQ_MAX_LENGTH 16  // TODO: support as large as 128
#define STEP_SEQ_DEFAULT_LENGTH 16
#define MAX_POLYPHONY 8
#define MAX_MOTION_SLOTS 8  // not used now
#define STEP_RESOLUTION 24  // one step is broken into {STEP_RESOLUTION} ticks

namespace Sequencer {

class Track {
public:
  enum class PlayMode { Forward, Backward, Random, Bounce, Brownian };

  Track(int channel = 1,
        int len = STEP_SEQ_DEFAULT_LENGTH,
        PlayMode mode = PlayMode::Forward)
      : channel_(channel),
        length_(len),
        playMode_(mode),
        enabled_(true),
        tick_(0) {}

  void setEnabled(bool enabled) {
    enabled_ = enabled;
  }  // need to be called before use
  void setChannel(int channel) { channel_ = channel; }
  void setLength(int length) {
#ifdef JUCE_DEBUG
    jassert(length > 0);
#endif
    length_ = length;
  }
  int getChannel() const { return channel_; }
  bool isEnabled() const { return enabled_; }
  int getLength() const { return length_; }
  void setTickToZero() { tick_ = 0; }

  // note: midiCollector.reset need to be called outside
  void tick(juce::MidiMessageCollector& midiCollector) {
    if (enabled_) {
      auto index = oneLoopMidiSeq_.getNextIndexAtTime(tick_);
      // there might be more than one midiEvent during this tick
      while (oneLoopMidiSeq_.getEventTime(index) < tick_ + 1) {
        auto midi_event = oneLoopMidiSeq_.getEventPointer(index);
        midiCollector.addMessageToQueue(midi_event->message); // BUG: time stamp cannot be zero
      }                                                       // time conversion is tricky but you are almost there, you can do it!!!!!!!
    }

    tick_ = (tick_ + 1) % (length_ * STEP_RESOLUTION);
  }

  Step getStepAtIndex(int index) const { return steps_[index]; }
  void setStepAtIndex(int index, Step step);
  // TODO: track utilities (randomize, humanize, rotate, Euclidean, Grids,
  // etc.)

private:
  int channel_;

  // track parameters as seen by the user
  int length_;
  [[maybe_unused]] PlayMode playMode_;
  [[maybe_unused]] double swing_;  // TODO: implement swing
  [[maybe_unused]] bool resync_to_longest_track_;

  // function-related variables
  bool enabled_;
  int tick_;  // set to 0 for resync
  Step steps_[STEP_SEQ_MAX_LENGTH];

  // contains all the MIDI messages in one run of the track
  // time stamp is in ticks (one step is broken down into {STEP_RESOLUTION}
  // ticks)
  juce::MidiMessageSequence oneLoopMidiSeq_;
};

}  // namespace Sequencer