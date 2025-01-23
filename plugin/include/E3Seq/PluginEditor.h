#pragma once

#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h> // MidiKeyboard component
#include "TrackComponent.h"

// TODO: add version display

namespace audio_plugin {

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor& processorRef;

  // GUI elements
  TrackComponent trackOne;

  juce::MidiKeyboardComponent onScreenKeyboard;
  
  // void logMessage(const juce::String& m) {
  //   midiMessagesBox.moveCaretToEnd();
  //   midiMessagesBox.insertTextAtCaret(m + juce::newLine);
  // }

  // void addMessageToList(const juce::MidiMessage& message) {
  //   auto time = message.getTimeStamp();

  //   auto hours = ((int)(time / 3600.0)) % 24;
  //   auto minutes = ((int)(time / 60.0)) % 60;
  //   auto seconds = ((int)time) % 60;
  //   auto millis = ((int)(time * 1000.0)) % 1000;

  //   auto timecode = juce::String::formatted("%02d:%02d:%02d.%03d", hours,
  //                                           minutes, seconds, millis);

  //   logMessage(timecode + "  -  " + message.getDescription());
  // }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
}  // namespace audio_plugin
