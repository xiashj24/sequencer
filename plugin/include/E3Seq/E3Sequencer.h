//******************************************************************************
//	E3Sequencer.h
//	(c)2025 KORG Inc. / written by Shijie Xia
//
//	platform-agonistic polyphonic MIDI step sequencer
//******************************************************************************
// note : avoid JUCE API and cpp STL inside this class

#pragma once
#include "E3Seq/MonoTrack.h"
#include "E3Seq/PolyTrack.h"
#include "E3Seq/KeyboardMonitor.h"
#include <juce_audio_devices/juce_audio_devices.h>  // juce::MidiMessageCollector

// TODO: Doxygen documentation
// TODO: add example code

#define STEP_SEQ_NUM_MONO_TRACKS 8
#define STEP_SEQ_NUM_POLY_TRACKS 4
#define STEP_SEQ_NUM_TRACKS \
  (STEP_SEQ_NUM_MONO_TRACKS + STEP_SEQ_NUM_POLY_TRACKS)
#define BPM_DEFAULT 120
#define BPM_MAX 240
#define BPM_MIN 30

/*
  by default, the timing resolution is a 1/384 of one bar
  (or 1/24 of a quarter note, same as Elektron)

  so 1 micro step = 2 seconds / 384 = 5.208 ms for BPM 120
  ideally tick interval should be shorter than 1/10 of a micro step (ticking
  rate over 2000Hz), otherwise timing precision suffers
*/

namespace Sequencer {

class E3Sequencer {
public:
  E3Sequencer(juce::MidiMessageCollector& midiCollector,
              double bpm = BPM_DEFAULT);

  E3Sequencer(const E3Sequencer&) = delete;
  E3Sequencer& operator=(const E3Sequencer&) = delete;
  ~E3Sequencer() = default;

  void start(double startTime);

  void stop() {
    running_ = false;
    panic();
  }

  bool neverStarted() const { return startTime_ == 0.0; }

  void resume() { running_ = true; }
  void setBpm(double BPM) { bpm_ = BPM; }
  double getBpm() const { return bpm_; }

  // send allNoteOff to all tracks immediately
  // note: this is probably not the right way to do it, will refactor later
  void panic() {
    for (int i = 0; i < STEP_SEQ_NUM_TRACKS; ++i) {
      auto msg = juce::MidiMessage::allNotesOff(i + 1);
      msg.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
      midiCollector_.addMessageToQueue(msg);
    }
  }

  void setEnableSmartOverdub(bool should) {
    for (auto& polytrack : polyTracks_)
    {
      polytrack.setEnableSmartOverdub(should);
    }
  }

  bool isRunning() const { return running_; }
  bool isArmed() const { return armed_; }

  void setArmed(bool armed) { armed_ = armed; }
  void setQuantizeRec(bool shouldQuantize) { quantizeRec_ = shouldQuantize; }
  void handleNoteOn(juce::MidiMessage noteOn);
  void handleNoteOff(juce::MidiMessage noteOff);

  Track& getTrackByChannel(int channel) {
    if (channel <= STEP_SEQ_NUM_MONO_TRACKS) {
      return getMonoTrack(channel - 1);
    } else {
      return getPolyTrack(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS);
    }
  }

  // sequencer programming interface
  MonoTrack& getMonoTrack(int index) { return monoTracks_[index]; }

  PolyTrack& getPolyTrack(int index) { return polyTracks_[index]; }

  // deltaTime is in seconds, call this frequenctly, preferably over 1kHz
  void process(double deltaTime);

  std::function<void(int track_index, int step_index, MonoStep step)>
      notifyProcessorMonoStepUpdate;

  std::function<void(int track_index, int step_index, PolyStep step)>
      notifyProcessorPolyStepUpdate;

  // tick-based timekeeping for MIDI clock sync
  // void tick(juce::MidiMessageCollector& collector);
private:
  MonoTrack monoTracks_[STEP_SEQ_NUM_MONO_TRACKS];
  PolyTrack polyTracks_[STEP_SEQ_NUM_POLY_TRACKS];
  double bpm_;
  // TODO: alternative time signatures
  double getOneTickTime() const { return 15.0 / bpm_ / TICKS_PER_STEP; }

  double getOneStepTime() const { return TICKS_PER_STEP * getOneTickTime(); }

  // function-related variables
  bool running_;
  bool armed_;
  bool quantizeRec_;

  double timeSinceStart_;
  double startTime_;

  // timestamp in (fractional) steps
  Note calculateNoteFromNoteOnAndOff(juce::MidiMessage noteOn,
                                     juce::MidiMessage noteOff);

  KeyboardMonitor keyboardMonitor_;

  // std::optional<juce::MidiMessage> keyState_[STEP_SEQ_MAX_LENGTH];
  juce::MidiMessageCollector& midiCollector_;
};

}  // namespace Sequencer
