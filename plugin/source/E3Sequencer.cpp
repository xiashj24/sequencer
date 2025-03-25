#include "E3Seq/E3Sequencer.h"
#include <cmath>

// this class is the interface between the underlying sequencer logic and the
// outside app framework. It uses information such as BPM and MIDI clock to
// translate real world time to the "ticks" used by the sequencer
// ideally users of this class should not need to know about ticks at all

namespace Sequencer {

E3Sequencer::E3Sequencer(juce::MidiMessageCollector& midiCollector, double bpm)
    : monoTracks_{1, 2, 3, 4, 5, 6, 7, 8},
      polyTracks_{9, 10, 11, 12},  // will this work?
      bpm_(bpm),
      running_(false),
      armed_(false),
      quantizeRec_(false),
      timeSinceStart_(0.0),
      startTime_(0.0),
      midiCollector_(midiCollector) {
  // MARK: track config
  for (int index = 0; index < STEP_SEQ_NUM_TRACKS; ++index) {
    Track& track = getTrack(index);
    track.sendMidiMessage = [this](juce::MidiMessage msg) {
      // time translation
      int tick = (int)msg.getTimeStamp();
      double real_time_stamp = startTime_ + getOneTickTime() * tick;
      // this time translation code is smelling....need some further thinking
      this->midiCollector_.addMessageToQueue(
          msg.withTimeStamp(real_time_stamp));
    };
  }

  for (int i = 0; i < 128; ++i) {
    lastNoteOn_[i].second = -1;
  }
}

// note: for time precision, deltaTime should be much smaller than OneTickTime
void E3Sequencer::process(double deltaTime) {
  if (!running_)
    return;

  timeSinceStart_ += deltaTime;
  double one_tick_time = getOneTickTime();

  if (timeSinceStart_ >= one_tick_time) {
    for (int index = 0; index < STEP_SEQ_NUM_TRACKS; ++index) {
      getTrack(index).tick();
    }
    timeSinceStart_ -= one_tick_time;
  }

  return;
}

void E3Sequencer::start(double startTime) {
  running_ = true;
  timeSinceStart_ = 0.0;
  startTime_ = startTime;
  for (int index = 0; index < STEP_SEQ_NUM_TRACKS; ++index) {
    getTrack(index).returnToStart();
  }
}

// TODO: test edge case
// what will hanppen when hold a note for more than one bar???
void E3Sequencer::handleNoteOn(juce::MidiMessage noteOn) {
  if (noteOn.getChannel() > STEP_SEQ_NUM_TRACKS)
    return;

  if (this->isArmed() && this->isRunning()) {
    int note = noteOn.getNoteNumber();
    int channel = noteOn.getChannel();
    int step_index = getTrack(channel).getCurrentStepIndex();

    lastNoteOn_[note] = {noteOn, step_index};
  }
}

Note E3Sequencer::calculateNoteFromNoteOnAndOff(juce::MidiMessage noteOn,
                                                juce::MidiMessage noteOff) {
#ifdef JUCE_DEBUG
  jassert(noteOn.getNoteNumber() == noteOff.getNoteNumber());
  jassert(noteOn.getChannel() == noteOff.getChannel());
#endif
  int note_number = noteOn.getNoteNumber();
  int velocity = noteOn.getVelocity();

  double offset = 0.0;
  if (!quantizeRec_) {
    offset = (noteOn.getTimeStamp() - startTime_) / getOneStepTime();
    offset -= std::round(offset);  // wrap in [-0.5, 0.5)
  }

  auto length =
      (noteOff.getTimeStamp() - noteOn.getTimeStamp()) / getOneStepTime();
  length = std::min(length, static_cast<double>(STEP_SEQ_MAX_LENGTH));

  return {.number = note_number,
          .velocity = velocity,
          .offset = static_cast<float>(offset),
          .length = static_cast<float>(length)};
}

// TODO: this function is getting too big, consider refactoring
void E3Sequencer::handleNoteOff(juce::MidiMessage noteOff) {
  int note_number = noteOff.getNoteNumber();
  int channel = noteOff.getChannel();

  if (channel > STEP_SEQ_NUM_TRACKS)
    return;

  if (channel <= STEP_SEQ_NUM_MONO_TRACKS) {
    // live rec for mono tracks
    int step_index = lastNoteOn_[note_number].second;
    if (step_index >= 0) {
      auto noteOn = lastNoteOn_[note_number].first;
      if (noteOn.getNoteNumber() == note_number &&
          noteOn.getChannel() == channel) {
        auto Note = calculateNoteFromNoteOnAndOff(noteOn, noteOff);

        MonoStep step{.enabled = true, .note = Note};
        // getMonoTrack(channel - 1).setStepAtIndex(index, step); // no need
        // notify AudioProcessor about parameter update
        notifyProcessorMonoStepUpdate(channel - 1, step_index, step);
        lastNoteOn_[note_number].second = -1;
      }
    }
  } else {
    // live rec for poly tracks
    int step_index = lastNoteOn_[note_number].second;

    if (step_index >= 0) {
      auto noteOn = lastNoteOn_[note_number].first;
      if (noteOn.getNoteNumber() == note_number &&
          noteOn.getChannel() == channel) {
        // calculate offset and length

        auto note = calculateNoteFromNoteOnAndOff(noteOn, noteOff);

        PolyStep step = getPolyTrack(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS)
                            .getStepAtIndex(step_index);
        step.addNote(note);
        // just to update note_index, TODO: make it clearer?
        getPolyTrack(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS)
            .setStepAtIndex(step_index, step);

        // notify AudioProcessor about parameter change
        notifyProcessorPolyStepUpdate(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS,
                                      step_index, step);
        lastNoteOn_[note_number].second = -1;
      }
    }
  }
}
}  // namespace Sequencer
