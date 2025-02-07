#include "E3Seq/E3Sequencer.h"

#include <cmath>

namespace Sequencer {

void E3Sequencer::process(double deltaTime,
                          juce::MidiMessageCollector& collector) {
  if (!running_)
    return;

  // time update
  // note: deltaTime should be much smaller than oneTickTime or precision will
  // be bad
  time_ += deltaTime;
  double one_tick_time = getOneTickTime();

  if (time_ > one_tick_time) {
    for (auto track : tracks_) {
      track.tick(collector);  // tick and collect midi events from each track
    }
  }

  return;
}

}  // namespace Sequencer
