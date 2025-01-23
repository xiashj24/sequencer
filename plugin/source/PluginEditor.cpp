#include "E3Seq/PluginEditor.h"
#include "E3Seq/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      trackOne(p, 0),
      onScreenKeyboard(p.keyboardState,
                       juce::MidiKeyboardComponent::horizontalKeyboard) {
  juce::ignoreUnused(processorRef);

  addAndMakeVisible(trackOne);
  addAndMakeVisible(onScreenKeyboard);

  // TODO: set up message box
  // addAndMakeVisible(midiMessagesBox);
  // midiMessagesBox.setMultiLine(true);
  // midiMessagesBox.setReturnKeyStartsNewLine(true);
  // midiMessagesBox.setReadOnly(true);
  // midiMessagesBox.setScrollbarsShown(true);
  // midiMessagesBox.setCaretVisible(false);
  // midiMessagesBox.setPopupMenuEnabled(true);
  // midiMessagesBox.setColour(juce::TextEditor::backgroundColourId,
  //                           juce::Colour(0x32ffffff));
  // midiMessagesBox.setColour(juce::TextEditor::outlineColourId,
  //                           juce::Colour(0x1c000000));
  // midiMessagesBox.setColour(juce::TextEditor::shadowColourId,
  //                           juce::Colour(0x16000000));

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.

  // MARK: Initialization
  setSize(1200, 600); // TODO: fix screen size
  setResizable(true, true);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized() {
  // MARK: GUI layout
  trackOne.setBounds(10, 10, trackOne.getWidth(), trackOne.getHeight());

  onScreenKeyboard.setBounds(10, getHeight() - 90, getWidth() - 20, 80);

  // midiMessagesBox.setBounds(getLocalBounds()
  //                               .withWidth(halfWidth)
  //                               .withX(halfWidth)
  //                               .reduced(10)
  //                               .withTrimmedBottom(100));
}

}  // namespace audio_plugin
