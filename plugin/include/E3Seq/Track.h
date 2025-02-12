#pragma once
#include "Step.h"
#include <juce_audio_basics/juce_audio_basics.h>  // juce::MidiMessageSequence

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
#define TICKS_PER_STEP 24   // one step is broken into {TICKS_PER_STEP} ticks
// note: TICKS_PER_STEP over 24 do not make much sense since time keeping
// functions like processBlock() or MIDI callbacks are not called frequently
// enough

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
  void returnToStart() { tick_ = 0; }  // for resync

  // callback to pass MIDI messages to caller
  std::function<void(juce::MidiMessage msg)> sendMidiMessage;

  // this function should be called {TICKS_PER_STEP} times per step
  void tick();

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
  int tick_;
  Step steps_[STEP_SEQ_MAX_LENGTH];
  std::optional<Step> steps_buffer_[STEP_SEQ_MAX_LENGTH];

  // contains all the MIDI messages in one run of the track
  // time stamp is in ticks (one step is broken down into {TICKS_PER_STEP}
  // ticks)
  juce::MidiMessageSequence oneLoopMidiSeq_;
};

}  // namespace Sequencer