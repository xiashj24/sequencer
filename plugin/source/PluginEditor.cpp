#include "E3Seq/PluginEditor.h"
#include "E3Seq/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  juce::ignoreUnused(processorRef);

  // MARK: GUI callbacks
  addAndMakeVisible(bassDrumButton);
  bassDrumButton.setButtonText("Bass Drum (36)");
  bassDrumButton.onClick = [this] { addMessageToList(processorRef.triggerNote(36)); };

  addAndMakeVisible(snareDrumButton);
  snareDrumButton.setButtonText("Snare Drum (38)");
  snareDrumButton.onClick = [this] { addMessageToList(processorRef.triggerNote(38)); };

  addAndMakeVisible(closedHiHatButton);
  closedHiHatButton.setButtonText("Closed HH (42)");
  closedHiHatButton.onClick = [this] { addMessageToList(processorRef.triggerNote(42)); };

  addAndMakeVisible(openHiHatButton);
  openHiHatButton.setButtonText("Open HH (46)");
  openHiHatButton.onClick = [this] { addMessageToList(processorRef.triggerNote(46)); };

  addAndMakeVisible(midiMessagesBox);
  midiMessagesBox.setMultiLine(true);
  midiMessagesBox.setReturnKeyStartsNewLine(true);
  midiMessagesBox.setReadOnly(true);
  midiMessagesBox.setScrollbarsShown(true);
  midiMessagesBox.setCaretVisible(false);
  midiMessagesBox.setPopupMenuEnabled(true);
  midiMessagesBox.setColour(juce::TextEditor::backgroundColourId,
                            juce::Colour(0x32ffffff));
  midiMessagesBox.setColour(juce::TextEditor::outlineColourId,
                            juce::Colour(0x1c000000));
  midiMessagesBox.setColour(juce::TextEditor::shadowColourId,
                            juce::Colour(0x16000000));

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.

  // MARK: Initialization
  logMessage(juce::String::formatted("Samplerate: %d Hz", (int)processorRef.getSampleRate()));
  logMessage(juce::String::formatted("Block Size: %d samples", processorRef.getBlockSize()));
  // TODO: what happens when host changes samplerate/block size?
  setSize(1000, 500);
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
  auto halfWidth = getWidth() / 2;
  auto buttonsBounds = getLocalBounds().withWidth(halfWidth).reduced(10);

  bassDrumButton.setBounds(buttonsBounds.getX(), 10, buttonsBounds.getWidth(),
                           40);
  snareDrumButton.setBounds(buttonsBounds.getX(), 60, buttonsBounds.getWidth(),
                            40);
  closedHiHatButton.setBounds(buttonsBounds.getX(), 110,
                              buttonsBounds.getWidth(), 40);
  openHiHatButton.setBounds(buttonsBounds.getX(), 160, buttonsBounds.getWidth(),
                            40);
  midiMessagesBox.setBounds(
      getLocalBounds().withWidth(halfWidth).withX(halfWidth).reduced(10));
}

}  // namespace audio_plugin
