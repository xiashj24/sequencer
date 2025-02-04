//******************************************************************************
//	E3Sequencer.h
//	(c)2025 KORG Inc. / written by Shijie Xia
//
//	platform-agonistic polyphonic MIDI step sequencer
//******************************************************************************
// note : JUCE API and std::vector should be avoided

#pragma once
#include "Step.h"
#include "Track.h"
#include "NoteEvents.h"
#include "ControlChangeEvents.h"

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
#define DEFAULT_BPM 120
#define RESOLUTION 24  // divide 1 step into 24 micro-steps
// NoteOn (and NoteOff?) events are quantized to the closest microsteps
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
  E3Sequencer(int bpm = DEFAULT_BPM)
      : bpm_(bpm), running_(false), time_(0.0), oneTickTime_(0.001) {
    // MARK: channel setup
    for (int i = 0; i < STEP_SEQ_NUM_TRACKS; i++) {
      getTrack(i).setChannel(i + 1);
    }
  }

  E3Sequencer(const E3Sequencer&) = delete;
  E3Sequencer& operator=(const E3Sequencer&) = delete;
  ~E3Sequencer() = default;

  /*
    set up tick rate before using
    for example, if a system runs on sample rate 48k and block size 64
    samples and ticks once per audio processing block the tick rate is
    48k/64 = 750
  */
  void setTickRate(double tickRate) { oneTickTime_ = 1.0 / tickRate; }

  void start() {
    running_ = true;
    time_ = 0.0;
  }
  void stop() { running_ = false; } // TODO: send NoteOff to all playing notes (not AllNoteOff!)
  void resume() { running_ = true; }
  void setBpm(double BPM) { bpm_ = BPM; }

  bool isRunning() const { return running_; }

  // for sequence editing
  Track& getTrack(int index) { return tracks_[index]; }

  void process(NoteEvent* noteOn,
            NoteEvent* noteOff,
            ControlChangeEvent* cc);  // this should be called once per tick

  // TODO: API to export the sequencer data as some text format (JSON or
  // protobuf)

private:
  // seqencer parameters here
  double bpm_;

  // function-related variables
  bool running_;
  double time_;

  double oneTickTime_;

  Track tracks_[STEP_SEQ_NUM_TRACKS];
};

}  // namespace Sequencer
