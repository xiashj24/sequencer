#pragma once
#include <juce_audio_processors/juce_audio_processors.h>  //juce::MidiMessage

namespace Sequencer {
class KeyboardMonitor {
public:
  KeyboardMonitor() : channel_(0) {
    for (auto& flag : flags_) {
      flag = -1;
    }
  }

  // reports the MIDI channel of the last handled MIDI message handled
  // if note midi message has been handled, it will return 0
  // for now, we can safely assume that a keyboard will not switch channel in
  // the middle of a note
  int getActiveChannel() const { return channel_; }

  // note: a valid flag should be non-negative
  void processNoteOn(juce::MidiMessage noteOn, int flag) {
#ifdef JUCE_DEBUG
    jassert(flag >= 0);
#endif
    noteOns_[noteOn.getNoteNumber()] = noteOn;
    flags_[noteOn.getNoteNumber()] = flag;
    channel_ = noteOn.getChannel();
  }

  void processNoteOff(juce::MidiMessage noteOff) {
    flags_[noteOff.getNoteNumber()] = -1;
  }

  bool isNoteOn(int noteNumber) const { return flags_[noteNumber] >= 0; }

  bool getNoteOn(int noteNumber, juce::MidiMessage& noteOn, int& flag) const {
    if (flags_[noteNumber] < 0) {
      return false;
    }

    noteOn = noteOns_[noteNumber];
    flag = flags_[noteNumber];
    return true;
  }

  // return a list of note numbers (size<=POLYPHNY) that are (supoosed to be)
  // currently playing
  // note: constructing a new vector on the fly is not memory
  // firendly, need to change implementation when porting to prologue
  std::vector<int> getActiveNotes(size_t maxNumOfNotes) const {
    std::vector<juce::MidiMessage> held_down_keys;
    for (int i = 0; i < 128; ++i) {
      if (flags_[i] >= 0) {
        held_down_keys.push_back(noteOns_[i]);
      }
    }

    std::sort(held_down_keys.begin(), held_down_keys.end(),
              [](const juce::MidiMessage& a, const juce::MidiMessage& b) {
                return a.getTimeStamp() > b.getTimeStamp();
              });

    // only keep the most recent {maxNumOfNotes}
    if (held_down_keys.size() > maxNumOfNotes) {
      held_down_keys.resize(maxNumOfNotes);
    }
    std::vector<int> notes;
    for (const auto& key : held_down_keys) {
      notes.push_back(key.getNoteNumber());
    }
    return notes;
  }

private:
  juce::MidiMessage noteOns_[128];
  int flags_[128];  // also used to store step index for note ons
                    // negative value indicates invalid
  int channel_;
};
}  // namespace Sequencer
