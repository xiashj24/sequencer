#pragma once
#include "E3Seq/Step.h"
#include <juce_audio_basics/juce_audio_basics.h>  // juce::MidiMessageSequence

/*
  core functionality of a one track sequencer

  created by Shijie Xia in 2025/2
  maintained by {put your name here if you have to maintain this} in {date}

  Please allow me to rant a little about the coding culture in Korg
  that every file seems to be created and maintained by a single programmer
  and no one seems to care about writing good documentations or enforcing good
  coding styles. This leads to the bloated and cryptic codebase that Spark is
  and no one seems to understand the big picture :(
*/

/*
  Guide for runtime testing: modidy a step's parameter while the sequencer is
  running and make sure no noteOff is missing
*/

// TODO: per track swing

// Note: polyphony will probably have to be implemented simply as
// {MAX_POLYPHONY} mono tracks use a separate assigner class to handle note
// stealing and overdubbing between different tracks?

#define STEP_SEQ_MAX_LENGTH 16  // TODO: support as large as 128
#define STEP_SEQ_DEFAULT_LENGTH 16
#define MAX_MOTION_SLOTS 8  // not used now
#define TICKS_PER_STEP 24   // one step is broken into {TICKS_PER_STEP} ticks

// TICKS_PER_STEP over 24 make little sense since tick() is not
// called frequently enough to achieve such precision

namespace Sequencer {

class Track {
public:
  enum class PlayMode { Forward, Backward, Random, Bounce, Brownian };

  Track(int channel = 1,
        int len = STEP_SEQ_DEFAULT_LENGTH,
        PlayMode mode = PlayMode::Forward)
      : channel_(channel),
        trackLength_(len),
        playMode_(mode),
        enabled_(true),
        tick_(0) {}

  void setEnabled(bool enabled) { enabled_ = enabled; }
  void setChannel(int channel) { channel_ = channel; }
  void setLength(int length) { trackLength_ = length; }
  int getChannel() const { return channel_; }
  bool isEnabled() const { return enabled_; }
  int getLength() const { return trackLength_; }

  // caller should register a callback to receive Midi messages
  std::function<void(juce::MidiMessage msg)> sendMidiMessage;

  // this function should be called {TICKS_PER_STEP} times per step
  void tick();

  void returnToStart();

  Step getStepAtIndex(int index) const { return steps_[index]; }
  void setStepAtIndex(int index, Step step) { steps_[index] = step; }

  int getCurrentStepIndex() const;

  // TODO: track utilities (randomize, humanize, rotate, Euclidean, Grids,
  // etc.)

private:
  int channel_;

  // track parameters as seen by the user
  int trackLength_;
  [[maybe_unused]] PlayMode playMode_;
  [[maybe_unused]] double swing_;  // TODO: implement swing
  [[maybe_unused]] bool resync_to_longest_track_; // or master length

  // function-related variables
  bool enabled_;
  int tick_;
  Step steps_[STEP_SEQ_MAX_LENGTH];

  static constexpr int half_step_ticks = TICKS_PER_STEP / 2;

  void renderStep(int index);

  // timestamp is in ticks (not seconds or samples)
  void renderMidiMessage(juce::MidiMessage message);

  // invariant: MIDI events are always sorted by timestamp
  // note: when porting to Spark, change from juce::MidiMessageSequence to a
  // simpler custom class
  juce::MidiMessageSequence firstRun_;
  juce::MidiMessageSequence secondRun_;
};

}  // namespace Sequencer