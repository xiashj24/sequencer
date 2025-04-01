// poly track reuse most of the implementations for time-keeping from mono
// track, but renders the step differently

// note: maybe it makes sense to inherit from Track to create mono track and
// make track Abstract

#include "E3Seq/Step.h"
#include "E3Seq/Track.h"

namespace Sequencer {
class MonoTrack : public Track {
public:
  MonoTrack(int channel,
            const KeyboardMonitor& keyboard,
            int length = STEP_SEQ_DEFAULT_LENGTH,
            PlayMode mode = PlayMode::Forward)
      : Track(channel, keyboard, length, mode) {}

  // sequencer programmer interface
  void setStepAtIndex(int index,
                      MonoStep step,
                      bool ignore_alternate_count = false) {
    if (ignore_alternate_count) {
      step.count = steps_[index].count;
    }

    steps_[index] = step;
  }

  MonoStep getStepAtIndex(int index) const { return steps_[index]; }

private:
  MonoStep steps_[STEP_SEQ_MAX_LENGTH];

  int getStepNoteOnTick(int index) const {
    return static_cast<int>((index + steps_[index].note.offset) *
                            TICKS_PER_STEP);
  }

  int getStepNoteOffTick(int index) const {
    return static_cast<int>(
        (index + steps_[index].note.offset + steps_[index].note.length) *
        TICKS_PER_STEP);
  }

  int getStepRenderTick(int index) const override final {
    return getStepNoteOnTick(index);
  }

  // TODO: refactor this to use renderNote instead of renderMidiMessage
  // need to implement a separate midi effect (retrigger) to process outcoming
  // midi messages and incorporate that into the step parameter
  // after that, make renderMidiMessage private instead of protected
  void renderStep(int index) override final {
    auto& step = steps_[index];
    if (step.enabled) {
      // alternate check
      if ((step.count++) % step.alternate != 0) {
        return;
      }

      // probability check
      if (juce::Random::getSystemRandom().nextFloat() >= step.probability) {
        return;
      }

      int note_on_tick = getStepNoteOnTick(index);
      int note_off_tick = getStepNoteOffTick(index);

      // force note off before the next active step
      // TODO: this still doesn't feel like the right thing to do
      // maybe it makes more sense to reconsider this code from the perspective
      // of polytrack note stealing behaviour
      // i.e make the code for mono & poly tracks more unified
      int next_active_step_index = (index + 1) % getLength();
      while (!getStepAtIndex(next_active_step_index).enabled) {
        next_active_step_index = (next_active_step_index + 1) % getLength();
      }

      if (next_active_step_index > index) {
        note_off_tick =
            std::min(note_off_tick, getStepNoteOnTick(next_active_step_index));
      } else if (next_active_step_index < index) {
        note_off_tick = std::min(
            note_off_tick, getStepNoteOnTick(next_active_step_index) +
                               TICKS_PER_STEP * getLength());  // is this ok?
      }

      // note on
      juce::MidiMessage note_on_message = juce::MidiMessage::noteOn(
          getChannel(), step.note.number, (juce::uint8)step.note.velocity);
      note_on_message.setTimeStamp(note_on_tick);
      renderMidiMessage(note_on_message);

      // retrigger
      if (step.retrigger_rate > 0.0) {
        int retrigger_interval_in_ticks =
            static_cast<int>(step.retrigger_rate * TICKS_PER_STEP);
        for (int tick = note_on_tick + retrigger_interval_in_ticks;
             tick < note_off_tick; tick += retrigger_interval_in_ticks) {
          juce::MidiMessage retrigger_note_off_message =
              juce::MidiMessage::noteOff(getChannel(), step.note.number,
                                         (juce::uint8)step.note.velocity);
          retrigger_note_off_message.setTimeStamp(tick);
          juce::MidiMessage retrigger_note_on_message =
              juce::MidiMessage::noteOn(getChannel(), step.note.number,
                                        (juce::uint8)step.note.velocity);
          retrigger_note_on_message.setTimeStamp(tick);
          renderMidiMessage(retrigger_note_off_message);
          renderMidiMessage(retrigger_note_on_message);
        }
      }

      // note off
      juce::MidiMessage note_off_message = juce::MidiMessage::noteOff(
          getChannel(), step.note.number, (juce::uint8)step.note.velocity);
      note_off_message.setTimeStamp(note_off_tick);
      renderMidiMessage(note_off_message);
    }
  }
};
}  // namespace Sequencer
