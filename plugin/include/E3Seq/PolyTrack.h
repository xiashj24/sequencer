#include "E3Seq/Step.h"
#include "E3Seq/Track.h"

// does it even make sense to have a play mode for polytrack? should test and
// decide

namespace Sequencer {
class PolyTrack : public Track {
public:
  PolyTrack(int channel, int length = STEP_SEQ_DEFAULT_LENGTH)
      : Track(channel, length) {}

  // note: there is some code duplication but I can't think of a better way
  PolyStep getStepAtIndex(int index) const { return steps_[index]; }

  // side note: for embedded, it makes sense to begin a new transaction of undo
  // manager whenever stepStepAtIndex is called
  void setStepAtIndex(int index, PolyStep step) { steps_[index] = step; }

private:
  PolyStep steps_[STEP_SEQ_MAX_LENGTH];

  int getStepRenderTick(int index) const override final {
    // render at the start of the step time window
    // TODO: also try render at the ealiest note on tick
    return index * TICKS_PER_STEP - HALF_STEP_TICKS;
  }

  void renderStep(int index) override final {
    auto& step = steps_[index];
    if (step.enabled) {
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