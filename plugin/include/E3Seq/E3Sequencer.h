//******************************************************************************
//	E3Sequencer.h
//	(c)2025 KORG Inc. / written by Shijie Xia
//
//	platform-agonistic polyphonic MIDI step sequencer
//******************************************************************************
// note : avoid JUCE API and cpp STL inside this class

#pragma once
#include "Step.h"
#include "Track.h"
// #include "NoteEvent.h"
// #include "ControlChangeEvent.h"
#include <juce_audio_devices/juce_audio_devices.h>  // juce::MidiMessageCollector



// TODO: Doxygen documentation

// PARAMETERS

// --- track parameters ---
// Length: 1..16..128
// Playback mode: forward/backward/random/bounce/brownian

// --- step parameters ---
// Note: 0..127
// Gate: 0..100%..16..TIE  // quantized to 1/24 in (0,1)
// Velocity: 0..127
// Offset: 0..100%
// Roll:  1/2/3/4 (Do we need more?)
// Probablity: 0..100%
// Alternate: 1/2/3/4 (Do we need more?)

// (do not implement for now, as this is a keyboard centric parameter)
// Pitchbend: -50%..50% (TODO: need to learn more on MIDI spec on this)

#define STEP_SEQ_NUM_TRACKS 8  // as defined by the product specs
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

// TODO: add example code

namespace Sequencer {

class E3Sequencer {
public:
  E3Sequencer(juce::MidiMessageCollector& midiCollector,
              double bpm = BPM_DEFAULT);

  E3Sequencer(const E3Sequencer&) = delete;
  E3Sequencer& operator=(const E3Sequencer&) = delete;
  ~E3Sequencer() = default;

  void start(double startTime);

  void stop() { running_ = false; }
  // TODO:
  // go through the voiceBuffer of every track and
  // send NoteOff to all playing notes (not AllNoteOff!)
  // but the outside app is responsible for this action, callback?

  void resume() { running_ = true; }
  void setBpm(double BPM) { bpm_ = BPM; }
  double getBpm() const { return bpm_; }

  bool isRunning() const { return running_; }

  // sequencer programming interface
  Track& getTrack(int index) { return tracks_[index]; }
  // second-based timekeeping, call this frequenctly, preferably over 1kHz
  void process(double deltaTime);

  // tick-based timekeeping for MIDI clock sync
  // void tick(juce::MidiMessageCollector& collector);

  // TODO: presets based on JSON or Protobuf
private:
  // seqencer parameters here
  double bpm_;
  // TODO: alternative time signatures
  double getOneTickTime() const { return 15.0 / bpm_ / TICKS_PER_STEP; }

  // function-related variables
  bool running_;
  double timeSinceStart_;
  double startTime_;
  juce::MidiMessageCollector& midiCollector_;
  Track tracks_[STEP_SEQ_NUM_TRACKS];
};

}  // namespace Sequencer
