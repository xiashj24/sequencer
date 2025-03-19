#pragma once
#include "E3Seq/Step.h"
#include <juce_audio_basics/juce_audio_basics.h>  // juce::MidiMessageSequence

/*
  core functionality of a one track monophonic sequencer

  created by Shijie Xia in 2025/2
  maintained by {put your name here if you have to maintain this} in {date}

  Please allow me to rant a little about the coding culture in Korg
  that every file seems to be created and maintained by a single programmer
  and no one seems to care about writing good documentations or enforcing good
  coding styles, which leads to the bloated and cryptic codebase that Spark is
  :(
*/

#define STEP_SEQ_MAX_LENGTH 16  // TODO: test as large as 128
#define STEP_SEQ_DEFAULT_LENGTH 16
#define MAX_MOTION_SLOTS 8  // not used now
#define TICKS_PER_STEP 24   // one step is broken into {TICKS_PER_STEP} ticks
// note: TICKS_PER_STEP over 24 makes little sense since tick() need to be
// called more frequently than 1kHz to achieve such precision

namespace Sequencer {

class Track {
public:
  enum class PlayMode { Forward, Backward, Random, Bounce, Brownian };

  Track(int channel = 1,
        int len = STEP_SEQ_DEFAULT_LENGTH,
        PlayMode mode = PlayMode::Forward,
        bool legato = false)
      : channel_(channel),
        trackLength_(len),
        playMode_(mode),
        legato_(legato),
        enabled_(true),
        tick_(0) {}

  void setEnabled(bool enabled) { enabled_ = enabled; }
  void setChannel(int channel) { channel_ = channel; }
  void setLength(int length) { trackLength_ = length; }
  int getChannel() const { return channel_; }
  bool isEnabled() const { return enabled_; }
  int getLength() const { return trackLength_; }

  // caller should register a callback to receive MIDI messages
  std::function<void(juce::MidiMessage msg)> sendMidiMessage;

  // this function should be called (on average) {TICKS_PER_STEP} times per step
  // some amount of time jittering should be fine
  void tick();

  void returnToStart();  // for resync

  Step getStepAtIndex(int index) const { return steps_[index]; }
  void setStepAtIndex(int index,
                      Step step,
                      bool ignore_alternate_count = false);

  int getCurrentStepIndex() const;

  // TODO: track utilities (randomize, humanize, rotate, Euclidean, Grids,
  // etc.)

private:
  int channel_;

  // track parameters as seen by the user

  int trackLength_;
  [[maybe_unused]] PlayMode playMode_;
  bool legato_;
  [[maybe_unused]] float
      swing_;  // TODO: implement swing (should not affect roll)
  [[maybe_unused]] bool resync_to_longest_track_;  // or master length?

  bool enabled_;

  // functiona related variables
  int tick_;
  Step steps_[STEP_SEQ_MAX_LENGTH];

  static constexpr int half_step_ticks = TICKS_PER_STEP / 2;

  void renderStep(int index);

  // timestamp is in ticks (not seconds or samples)
  void renderMidiMessage(juce::MidiMessage message);

  int getStepNoteOnTick(int index) const;
  int getStepNoteOffTick(int index) const;

  // double MIDI buffer inspired by the endless scrolling
  // background technique in early arcade games
  // invariant: MIDI messages are always sorted by timestamp
  // note: when porting to Spark/Prologue, change from juce::MidiMessageSequence
  // to a more performant data structure
  juce::MidiMessageSequence firstRun_;
  juce::MidiMessageSequence secondRun_;
};

}  // namespace Sequencer