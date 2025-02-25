#pragma once

#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>  // MidiKeyboard component
#include "TrackComponent.h"

// TODO: add version display

namespace audio_plugin {

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  bool keyPressed(const juce::KeyPress& key) override {
    if (key == juce::KeyPress::spaceKey) {
      auto& seq = processorRef.sequencer;
      if (seq.isRunning()) {
        seq.stop();
      } else {
        if (seq.neverStarted()) {
          seq.start(juce::Time::getMillisecondCounterHiRes());
        } else {
          seq.resume();
        }
      }
      return true;  // Key handled
    }
    return false;  // Pass key event to parent
  }

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor& processorRef;

  // GUI elements
  TrackComponent tracks[8];

  juce::TextButton startButton;
  juce::TextButton stopButton;
  juce::TextButton continueButton;

  juce::TextButton panicButton;

  juce::Label bpmLabel;
  juce::Slider bpmSlider;

  juce::Label keyboardMidiChannelLabel;
  juce::Slider keyboardMidiChannelSlider;

  juce::TextButton helpButton;

  void showHelpPopup() {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon, "Help",
        "Space key: toggle stop/continue\n"
        "Double click or alt+click on any parameter to reset to its default "
        "value.\n");
  }

  juce::MidiKeyboardComponent onScreenKeyboard;

  juce::TooltipWindow tooltip;

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
