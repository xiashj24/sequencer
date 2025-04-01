#pragma once
#include "E3Seq/Step.h"
#include "E3Seq/KeyboardMonitor.h"
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

// TODO: make this a static const variable of TRACK
#define STEP_SEQ_MAX_LENGTH 16  // TODO: test as large as 128
#define STEP_SEQ_DEFAULT_LENGTH 16
#define MAX_MOTION_SLOTS 8  // not used now
#define TICKS_PER_STEP 24   // one step is broken into {TICKS_PER_STEP} ticks
// note: TICKS_PER_STEP over 24 (96 ppq) makes little sense since tick() need to
// be called more frequently than 1kHz to achieve such precision

namespace Sequencer {

class Track {
public:
  enum class PlayMode {
    Forward,
    Backward,
    Random,
    Bounce,
    Brownian
  };  // unused now

  Track(int channel,
        const KeyboardMonitor& keyboard,
        int length = STEP_SEQ_DEFAULT_LENGTH,
        PlayMode mode = PlayMode::Forward)
      : keyboardRef(keyboard),
        channel_(channel),
        trackLength_(length),
        playMode_(mode),
        enabled_(true),
        tick_(0) {}

  ~Track() = default;

  void setEnabled(bool enabled) { enabled_ = enabled; }

  // what's the use of this method. for runtime channel swithing?
  void setChannel(int channel) { channel_ = channel; }
  /*
    early worrying note:
    when setLength(length) is called and length > current play position
    some kind of additonal processing might be needed to ensure that everything
    plays nicely in sync
    will revisit this when I start implement polymeter
  */
  void setLength(int length) { trackLength_ = length; }
  int getChannel() const { return channel_; }
  bool getIsEnabled() const { return enabled_; }
  int getLength() const { return trackLength_; }

  // caller should register a callback to receive MIDI messages
  std::function<void(juce::MidiMessage msg)> sendMidiMessage;

  // this function should be called (on average) {TICKS_PER_STEP} times per step
  // some amount of time jittering should be fine
  void tick();

  void returnToStart();  // for resync

  int getCurrentStepIndex() const;  // exposed to GUI to show play position

  // TODO: track utilities (randomize, humanize, rotate, Euclidean, Grids,
  // etc.)

protected:
  void renderNote(int index, Note note);

  // timestamp in ticks (not seconds or samples)
  void renderMidiMessage(juce::MidiMessage message);

  // for note stealing
  const KeyboardMonitor& keyboardRef;

  static constexpr int HALF_STEP_TICKS = TICKS_PER_STEP / 2;

private:
  int channel_;

  // track parameters as seen by the user

  int trackLength_;
  [[maybe_unused]] PlayMode playMode_;
  // TODO: implement swing (should not affect roll)
  [[maybe_unused]] float swing_;
  [[maybe_unused]] bool resync_to_longest_track_;  // or master length?

  bool enabled_;

  // function related variables
  int tick_;

  // derived class must implement renderStep and getStepNoteRenderTick
  virtual void renderStep(int index) = 0;
  virtual int getStepRenderTick(int index) const = 0;

  /*
    double MIDI buffer inspired by the endless scrolling background technique in
    early arcade games
    invariant: MIDI messages are always sorted by timestamp
    note: when porting to Spark/Prologue, change from juce::MidiMessageSequence
    to a more performant data structure
  */

  juce::MidiMessageSequence firstRun_;
  juce::MidiMessageSequence secondRun_;
};

}  // namespace Sequencer