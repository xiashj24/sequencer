#include "E3Seq/Step.h"
#include "E3Seq/Track.h"
#include "E3Seq/KeyboardMonitor.h"

// this class serve as a data management layer between the core sequencer logic
// (Track.cpp) and global sequencer state (E3Sequencer)

namespace Sequencer {
class PolyTrack : public Track {
public:
  PolyTrack(int channel,
            const KeyboardMonitor& keyboard,
            int length = STEP_SEQ_DEFAULT_LENGTH)
      : Track(channel, keyboard, length) {}

  // note: there is some code duplication but I can't think of a better way
  PolyStep getStepAtIndex(int index) const { return steps_[index]; }

  void setStepAtIndex(int index, PolyStep step) { steps_[index] = step; }

private:
  PolyStep steps_[STEP_SEQ_MAX_LENGTH];

  int getStepRenderTick(int index) const override final {
    float offset_min = 0.0f;
    for (int i = 0; i < POLYPHONY; ++i) {
      offset_min = std::min(offset_min, steps_[index].notes[i].offset);
    }
    return static_cast<int>((index + offset_min) * TICKS_PER_STEP);
  }

  // TODO: rework this such that each note is rendered at their respective note
  // on timing
  void renderStep(int index) override final {
    auto& step = steps_[index];
    if (step.enabled) {
      // note stealing here
      // its behaviour should not be affected by probability
      if (keyboardRef.getActiveChannel() == this->getChannel()) {
        auto active_notes = keyboardRef.getActiveNotes(POLYPHONY);
        for (int note : active_notes) {
          step.stealNote(note);
        }
      }
      // However, the correct approach is probably note-wise stealing
      // i.e. each note should decide on its own

      // probability check
      if (juce::Random::getSystemRandom().nextFloat() >= step.probability) {
        return;
      }

      // render all notes in the step
      for (int j = 0; j < POLYPHONY; ++j) {
        // render note
        renderNote(index, steps_[index].notes[j]);
      }
    }
  }
};
}  // namespace Sequencer