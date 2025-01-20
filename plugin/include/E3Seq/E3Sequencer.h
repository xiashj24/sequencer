//******************************************************************************
//	E3Sequencer.h
//	(c)2025 KORG Inc. / written by Shijie Xia
//
//	general-purpose MIDI step sequencer
//******************************************************************************

// philosophy
// platform-agonistic: JUCE API and std::vector should be avoided
// flexiblility: usable for a lot of products
// easy to tweak from user interface

#pragma once

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

#define STEP_SEQ_MAX_LENGTH 128  // as defined by the product specs
#define STEP_SEQ_NUM_TRACKS 16   // as defined by the product specs

#define STEP_SEQ_DEFAULT_LENGTH 16
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

#define DEFAULT_NOTE 84       // C5
#define DEFAULT_VELOCITY 100  // MAX 127
#define DEFAULT_GATE 0.75

// TODO: add example code

class E3Sequencer {
public:
  // TODO: ControlChangeEvent
  struct ControlChangeEvent {};

  // MARK: Step
  struct Step {
    // step parameters as seen by the user
    int note = DEFAULT_NOTE;
    float gate = DEFAULT_GATE;  // note: gate can be greater than 1 but should
                                // be smaller than track length
    int velocity = DEFAULT_VELOCITY;
    float offset = 0.f;

    float pitchbend = 0.f;
    int roll = 1;
    float probability = 1.f;
    int alternate = 1;

    // function-related variables
    bool enabled = false;
    bool tie = false;  // or use something like gate > MAX_GATE ?
  };

  class Track;

  // MARK: NoteEvent
  struct NoteEvent {
    bool enabled;
    int note;
    int velocity;
    int channel;
    float time_since_last_tick;

    NoteEvent()
        : enabled(false),
          note(DEFAULT_NOTE),
          velocity(DEFAULT_VELOCITY),
          channel(1),
          time_since_last_tick(0.f) {}

    NoteEvent(Step step)
        : enabled(true),
          note(step.note),
          velocity(step.velocity),
          channel(1),
          time_since_last_tick(0.f) {}

    NoteEvent(Track track, Step step)
        : enabled(true),
          note(step.note),
          velocity(step.velocity),
          channel(track.getChannel()),
          time_since_last_tick(0.f) {}
  };

  // MARK: Track
  class Track {
  public:
    // TODO: polyphony and voice stealing...

    enum class PlayMode { Forward, Backward, Random, Bounce, Brownian };

    Track(int channel = 1,
          int len = STEP_SEQ_DEFAULT_LENGTH,
          PlayMode mode = PlayMode::Forward)
        : channel_(channel), length_(len), playMode_(mode), enabled_(false) {}

    void enable() { enabled_ = true; }  // need to be called before use
    void disable() { enabled_ = false; }
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
    [[maybe_unused]] float swing;  // TODO: implement swing
    [[maybe_unused]] bool resync_to_longest_track;

    // function-related variables
    bool enabled_;
    Step steps_[STEP_SEQ_MAX_LENGTH];
  };

  E3Sequencer(int bpm = DEFAULT_BPM)
      : bpm_(bpm), running_(false), time_(0.f), tickTime_(0.001f) {
    for (int i = 0; i < STEP_SEQ_NUM_TRACKS; ++i) {
      getTrack(i).setChannel(i + 1);
    }
  }
  // TODO: make this class copyable/movable?
  ~E3Sequencer() = default;

  /*
    set up tick rate before using
    for embedded systems, this is most likely to be 1000 Hz (1 ms per tick)
    for desktop/RPI systems running on sample rate 48k and block size 64
    samples and ticking once per audio processing block the tick rate is 48k/64
    = 750
  */
  void setTickRate(float tickRate) { tickTime_ = 1.f / tickRate; }

  // transport-related
  void start() {
    running_ = true;
    time_ = 0.f;
  }
  void stop() { running_ = false; }
  void resume() { running_ = true; }
  void setBpm(int BPM) { bpm_ = BPM; }

  // sequence editing functions
  // TODO: provide more high level functions
  Track& getTrack(int index) { return tracks_[index]; }

  void tick(NoteEvent* noteOn,
            NoteEvent* noteOff,
            ControlChangeEvent* cc);  // this should be called once per tick

  // TODO: API to export the sequencer data as some text format (JSON)

private:
  // seqencer parameters here
  int bpm_;

  // function-related variables
  bool running_;
  float time_;

  float tickTime_;

  Track tracks_[STEP_SEQ_NUM_TRACKS];
};
