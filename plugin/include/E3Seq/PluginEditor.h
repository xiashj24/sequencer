#pragma once

#include "E3Seq/PluginProcessor.h"
#include "E3Seq/SequencerComponent.h"
#include "Utility.h"
#include <juce_audio_utils/juce_audio_utils.h>  // juce::MidiKeyboardComponent

namespace audio_plugin {

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  bool keyPressed(const juce::KeyPress& key) override {
    // todo: add more keyboard shortcuts
    juce::ignoreUnused(key);
    return false;  // Pass key event to parent
  }

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor& processorRef;

  // GUI elements
  juce::Viewport sequencerViewport;
  SequencerComponent sequencerEditor;
  // TrackComponent tracks[8];

  juce::TextButton recordButton;
  juce::TextButton playButton;
  juce::TextButton stopButton;

  juce::TextButton panicButton;

  juce::Label bpmLabel;
  juce::Slider bpmSlider;

  juce::Label keyboardMidiChannelLabel;
  juce::Slider keyboardMidiChannelSlider;

  juce::TextButton helpButton;

  void showHelpPopup() {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon, "Help",
        "Plugin version: " + juce::String{PLUGIN_VERSION.data()} +
            "\n"
            "Build date: " +
            juce::String{BUILD_DATE.data()} +
            "\n\n"
            "Tips:\n"
            "Space key: toggle play/pause\n"
            "R: toggle real-time recording\n"
            "S: stop playback and return to start postion\n"
            "Double click or alt+click on any parameter to reset to default\n"
            "\n");
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
