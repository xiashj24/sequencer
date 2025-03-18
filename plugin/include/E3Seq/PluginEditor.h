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

  // void paint(juce::Graphics&) override;
  void resized() override;

  bool keyPressed(const juce::KeyPress& key) override {
    if (key == juce::KeyPress('s', juce::ModifierKeys::commandModifier, 0)) {
      savePreset();
      return true;  // Indicates the key press was handled
    } else if (key ==
               juce::KeyPress('l', juce::ModifierKeys::commandModifier, 0)) {
      loadPreset();
      return true;  // Indicates the key press was handled
    } else if (key ==
               juce::KeyPress('z', juce::ModifierKeys::commandModifier, 0)) {
      processorRef.undoManager.undo();
      return true;  // Indicates the key press was handled
    } else if (key ==
               juce::KeyPress('y', juce::ModifierKeys::commandModifier, 0)) {
      processorRef.undoManager.redo();
      return true;  // Indicates the key press was handled
    }
    return false;  // Pass unhandled key presses to the parent
  }

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor& processorRef;

  // GUI elements
  juce::Viewport sequencerViewport;
  SequencerComponent sequencerEditor;

  juce::TextButton recordButton;
  juce::TextButton playButton;
  juce::TextButton stopButton;

  juce::TextButton panicButton;

  juce::Label bpmLabel;
  juce::Slider bpmSlider;

  juce::TextButton quantizeButton;

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
            "Ctrl+s to save current state as preset\n"
            "Ctrl+l to load preset\n"
            "Ctrl+z to undo\n"
            "Ctrl+y to redo\n"
            "\n");
  }

  juce::MidiKeyboardComponent onScreenKeyboard;

  juce::TooltipWindow tooltip;

  // preset related stuff

  void savePreset();
  void loadPreset();
  juce::TextButton savePresetButton;
  juce::TextButton loadPresetButton;
  std::unique_ptr<juce::FileChooser> presetLoader;
  std::unique_ptr<juce::FileChooser> presetSaver;

  juce::File presetFolder;

  // TODO: midi monitor in a separate window
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
