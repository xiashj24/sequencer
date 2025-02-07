#include "E3Seq/Track.h"

namespace Sequencer {

// char* OffsetLabels[]{
//     "1/384", "1/192", "1/96",  "5/384",  "1/64", "7/384",
//     "1/48",  "3/128", "5/192", "11/384", "1/32"};  // TODO: migrate from using
//                                                    // double offset to integer
//                                                    // ticks and offset labels

void Track::setStepAtIndex(int index, Step step_after) {
  Step& step_before = steps_[index];

  if (step_before.enabled) {
    // delete existing NoteOn and NoteOff in oneLoopMidiSeq_
    auto step_start_time = ((double)index + OFFSET_MIN) * STEP_RESOLUTION;
    // note: nagative time stamps should work
    oneLoopMidiSeq_.deleteEvent(
        oneLoopMidiSeq_.getNextIndexAtTime(step_start_time), true);
  }

  if (step_after.enabled) {
    // add NoteOn and NoteOff to oneLoopMidiSeq_
    juce::MidiMessage note_on_message = juce::MidiMessage::noteOn(
        channel_, step_after.note, (juce::uint8)step_after.velocity);
    note_on_message.setTimeStamp(
        ((double)index + step_after.offset) *
        STEP_RESOLUTION);  // TODO: make sure rounding behave nicely

    juce::MidiMessage note_off_message = juce::MidiMessage::noteOff(
        channel_, step_after.note, (juce::uint8)step_after.velocity);
    note_off_message.setTimeStamp(((double)index + step_after.offset + step_after.gate) *
                                  STEP_RESOLUTION);

    oneLoopMidiSeq_.addEvent(note_on_message);
    oneLoopMidiSeq_.addEvent(note_off_message);
  }

  oneLoopMidiSeq_.updateMatchedPairs();
  // oneLoopMidiSeq_.sort();

  step_before = step_after;
}

}  // namespace Sequencer