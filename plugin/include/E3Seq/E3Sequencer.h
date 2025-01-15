#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
// since Spark uses the same MidiMessage class from JUCE
// we can expect porting to be easy...

// core sequencer API for Electribe 3
// copyright: Korg Inc.
// created by Shijie Xia on 2025/1/14

// TODO: Doxygene documentation

// PARAMETERS

// -- track parameters --
// Length: 1..16..16*8
// Playback mode: forward/backward/random/bounce/brownian

// -- step parameters --
// Gate: 0..100%..16..TIE
// Velocity: 0..100% (0..127? what about MIDI 2.0?)
// Offset: 0..100%
// Roll:  1/2/3/4 (Do we need more?)
// Note: 0..127 (note: below 20 is mostly unused)
// Pitchbend: -50%..50% (TODO: need to learn more about MIDI spec on this)
// Probablity: 0..100%
// Alternate: 1/2/3/4 (Do we need more?)

/*
set up tick time in seconds
for most embedded system, this is 0.001 (1 ms)
can also be tailored to desktop/RPI systems
for example, if a system runs on sample rate 48k and block size 64 samples
and tick once per audio processing block
the tick rate is 48k/64 = 750, thus tick time should be 1/750
*/

// TODO: add example code

class E3Sequencer {
public:
  E3Sequencer() = default;

  // TODO: make this copyable?

  ~E3Sequencer() = default;

  // time-keeping
  void reset(double tickTime) { tickTime_ = tickTime; } // need to be called before using

  // transport-related
  void start();  // from the 1st bar, to be specific
  void stop();
  void resume();
  void setBpm(int BPM) { bpm_ = BPM; }

  juce::MidiMessage getCurrentFrameOutput();  // this should be called once per tick
  // MIDI data output that happens before the next tick

private:
  // constants
  const int resolution = 384;

  /*
      by default, the timing resolution is a 1/384 of one bar
      (or 1/24 of a quarter note, same as Elektron)

      so 1 micro step = 2 seconds / 384 = 5.208 ms for BPM 120
      ideally tick interval should be shorter than 1/10 of a micro step (roughly
      over 2000Hz) otherwise timing precision suffers
  */

  // put all seqencer data here
  int bpm_ = 120;
  double tickTime_ = 0.001;

  int ticks_;

  // TODO: store the sequencer data in JSON
};