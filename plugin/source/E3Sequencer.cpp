#include "E3Seq/E3Sequencer.h"
#include <cmath>

// this class is the interface between the underlying sequencer logic and the
// outside app framework. It uses information such as BPM and MIDI clock to
// translate real world time to the "ticks" used by the sequencer
// ideally users of this class should not need to know about ticks at all

namespace Sequencer {

E3Sequencer::E3Sequencer(juce::MidiMessageCollector& midiCollector, double bpm)
    : monoTracks_{{1, keyboardMonitor_}, {2, keyboardMonitor_},
                  {3, keyboardMonitor_}, {4, keyboardMonitor_},
                  {5, keyboardMonitor_}, {6, keyboardMonitor_},
                  {7, keyboardMonitor_}, {8, keyboardMonitor_}},
      polyTracks_{{9, keyboardMonitor_},
                  {10, keyboardMonitor_},
                  {11, keyboardMonitor_},
                  {12, keyboardMonitor_}},
      bpm_(bpm),
      running_(false),
      armed_(false),
      quantizeRec_(false),
      timeSinceStart_(0.0),
      startTime_(0.0),
      midiCollector_(midiCollector) {
  // MARK: track config
  for (int channel = 1; channel <= STEP_SEQ_NUM_TRACKS; ++channel) {
    Track& track = getTrackByChannel(channel);
    track.sendMidiMessage = [this](juce::MidiMessage msg) {
      // time translation
      int tick = (int)msg.getTimeStamp();
      double real_time_stamp = startTime_ + getOneTickTime() * tick;
      // this time translation code is smelling....need some further thinking
      this->midiCollector_.addMessageToQueue(
          msg.withTimeStamp(real_time_stamp));
    };
  }
}

// note: for time precision, deltaTime should be much smaller than OneTickTime
void E3Sequencer::process(double deltaTime) {
  if (!running_)
    return;

  timeSinceStart_ += deltaTime;
  double one_tick_time = getOneTickTime();

  if (timeSinceStart_ >= one_tick_time) {
    for (int channel = 1; channel <= STEP_SEQ_NUM_TRACKS; ++channel) {
      int step_index = getTrackByChannel(channel).getCurrentStepIndex();

      getTrackByChannel(channel).tick();

      // in case there is a note stealing
      if (channel <= STEP_SEQ_NUM_MONO_TRACKS) {
        notifyProcessorMonoStepUpdate(
            channel - 1, step_index,
            getMonoTrack(channel - 1).getStepAtIndex(step_index));
      } else {
        notifyProcessorPolyStepUpdate(
            channel - 1 - STEP_SEQ_NUM_MONO_TRACKS, step_index,
            getPolyTrack(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS)
                .getStepAtIndex(step_index));
      }
    }
    timeSinceStart_ -= one_tick_time;
  }

  return;
}

void E3Sequencer::start(double startTime) {
  running_ = true;
  timeSinceStart_ = 0.0;
  startTime_ = startTime;
  for (int channel = 1; channel < STEP_SEQ_NUM_TRACKS; ++channel) {
    getTrackByChannel(channel).returnToStart();
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
  length = std::min(
      length, static_cast<double>(STEP_SEQ_MAX_LENGTH));  // clip to loop length

  return {.number = note_number,
          .velocity = velocity,
          .offset = static_cast<float>(offset),
          .length = static_cast<float>(length)};
}

void E3Sequencer::handleNoteOn(juce::MidiMessage noteOn) {
  if (noteOn.getChannel() > STEP_SEQ_NUM_TRACKS)
    return;

  int channel = noteOn.getChannel();
  int step_index = getTrackByChannel(channel).getCurrentStepIndex();

  keyboardMonitor_.processNoteOn(noteOn, step_index);
}

// TODO: this function is getting too big, consider refactoring
void E3Sequencer::handleNoteOff(juce::MidiMessage noteOff) {
  int note_number = noteOff.getNoteNumber();
  int channel = noteOff.getChannel();

  // ignore Midi channel > 12
  if (channel > STEP_SEQ_NUM_TRACKS)
    return;

  juce::MidiMessage note_on;
  int step_index;

  if (keyboardMonitor_.getNoteOn(note_number, note_on, step_index)) {
    if (note_on.getChannel() == channel) {
      keyboardMonitor_.processNoteOff(noteOff);
      // overdub
      if (this->isArmed() && this->isRunning()) {
        auto new_note = calculateNoteFromNoteOnAndOff(note_on, noteOff);

        if (channel <= STEP_SEQ_NUM_MONO_TRACKS) {
          // for mono tracks
          MonoStep step{.enabled = true, .note = new_note};
          // no need to call this for JUCE
          // getMonoTrack(channel - 1).setStepAtIndex(index, step);
          notifyProcessorMonoStepUpdate(channel - 1, step_index, step);
        } else {
          // for poly tracks
          PolyStep step = getPolyTrack(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS)
                              .getStepAtIndex(step_index);
          step.addNote(new_note);
          // this call is redundant for JUCE
          // getPolyTrack(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS)
          //     .setStepAtIndex(step_index, step);

          // notify AudioProcessor about parameter change
          notifyProcessorPolyStepUpdate(channel - 1 - STEP_SEQ_NUM_MONO_TRACKS,
                                        step_index, step);
        }
      }
    }
    // else channel has been changed in the middle of a note
  } else {
#ifdef JUCE_DEBUG
    jassertfalse;  // note on and note off mismatch!
#endif
  }
}
}  // namespace Sequencer
