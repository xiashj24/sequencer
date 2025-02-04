#pragma once
#include "Step.h"

#define STEP_SEQ_MAX_LENGTH 16
#define STEP_SEQ_DEFAULT_LENGTH 16

namespace Sequencer {

class Track {
public:
  // TODO: polyphony and voice stealing...

  enum class PlayMode { Forward, Backward, Random, Bounce, Brownian };

  Track(int channel = 1,
        int len = STEP_SEQ_DEFAULT_LENGTH,
        PlayMode mode = PlayMode::Forward)
      : channel_(channel), length_(len), playMode_(mode), enabled_(true) {}

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

  Step& operator[](int index) {
#ifdef JUCE_DEBUG
    jassert(index >= 0 && index < STEP_SEQ_MAX_LENGTH);
#endif
    return steps_[index];
  }

  // TODO: track utilities (randomize, humanize, rotate, Euclidean, Grids,
  // etc.)

private:
  int channel_;  // MIDI channel

  // track parameters as seen by the user
  int length_;
  [[maybe_unused]] PlayMode playMode_;
  [[maybe_unused]] double swing;  // TODO: implement swing
  [[maybe_unused]] bool resync_to_longest_track;

  // function-related variables
  bool enabled_;
  Step steps_[STEP_SEQ_MAX_LENGTH];
};

}  // namespace Sequencer