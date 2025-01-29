#include "E3Seq/PluginEditor.h"
#include "E3Seq/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      tracks{{p, 0}, {p, 1}, {p, 2}, {p, 3}, {p, 4}, {p, 5}, {p, 6}, {p, 7}},
      onScreenKeyboard(p.keyboardState,
                       juce::MidiKeyboardComponent::horizontalKeyboard) {
  juce::ignoreUnused(processorRef);

  bpmLabel.setText("BPM: ", juce::NotificationType::dontSendNotification);
  bpmLabel.attachToComponent(&bpmSlider, true);
  bpmSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
  bpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40,
                            STEP_BUTTON_HEIGHT);
  bpmSlider.setRange(30, 240, 1);
  bpmSlider.setValue(120);
  bpmSlider.onValueChange = [this] {
    processorRef.getSequencer().setBpm(bpmSlider.getValue());
  };
  addAndMakeVisible(bpmSlider);

  keyboardMidiChannelLabel.setText(
      "Keyboard trigger MIDI channel: ",
      juce::NotificationType::dontSendNotification);
  keyboardMidiChannelLabel.attachToComponent(&keyboardMidiChannelSlider, true);
  keyboardMidiChannelSlider.setSliderStyle(
      juce::Slider::SliderStyle::IncDecButtons);
  keyboardMidiChannelSlider.setTextBoxStyle(
      juce::Slider::TextBoxLeft, false, 30, STEP_BUTTON_HEIGHT);
  keyboardMidiChannelSlider.setRange(1, 16, 1);
  keyboardMidiChannelSlider.setValue(1);
  keyboardMidiChannelSlider.onValueChange = [this] {
    onScreenKeyboard.setMidiChannel((int)keyboardMidiChannelSlider.getValue());
  };
  addAndMakeVisible(keyboardMidiChannelSlider);

  allNotesOffButton.setButtonText("All Notes Off");
  allNotesOffButton.onClick = [this] { processorRef.allNotesOff(); };
  addAndMakeVisible(allNotesOffButton);

  if (processorRef.wrapperType ==
      juce::AudioProcessor::WrapperType::wrapperType_Standalone) {
    startButton.setButtonText(juce::String::fromUTF8("Start 🎵"));
    startButton.onClick = [this] { processorRef.getSequencer().start(); };
    addAndMakeVisible(startButton);

    stopButton.setButtonText(juce::String::fromUTF8("Stop ⏹"));
    stopButton.onClick = [this] { processorRef.getSequencer().stop(); };
    addAndMakeVisible(stopButton);

    continueButton.setButtonText(juce::String::fromUTF8("Cont ⏭"));
    continueButton.onClick = [this] { processorRef.getSequencer().resume(); };
    addAndMakeVisible(continueButton);
  }

  for (int i = 0; i < 8; i++) {
    addAndMakeVisible(tracks[i]);
  }
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
  setSize(1200, 900);
  setResizable(true, true);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with
  // a solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized() {
  // MARK: GUI layout

  int lastX = 10, lastY = 10;
  for (int i = 0; i < 8; i++) {
    tracks[i].setBounds(lastX, lastY, tracks[i].getWidth(),
                        tracks[i].getHeight());
    lastY += tracks[i].getHeight() + 10;
  }

  int util_buttons_height = getHeight() - 130;
  startButton.setBounds(10, util_buttons_height, STEP_BUTTON_WIDTH,
                        STEP_BUTTON_HEIGHT);
  stopButton.setBounds(80, util_buttons_height, STEP_BUTTON_WIDTH,
                       STEP_BUTTON_HEIGHT);
  continueButton.setBounds(150, util_buttons_height, STEP_BUTTON_WIDTH,
                           STEP_BUTTON_HEIGHT);
  bpmSlider.setBounds(260, util_buttons_height, 200, STEP_BUTTON_HEIGHT);

  keyboardMidiChannelSlider.setBounds(getWidth() - 200, util_buttons_height, 80,
                                      STEP_BUTTON_HEIGHT);
  allNotesOffButton.setBounds(getWidth() - 100, getHeight() - 130, 90,
                              STEP_BUTTON_HEIGHT);
  onScreenKeyboard.setBounds(10, getHeight() - 90, getWidth() - 20, 80);

  // midiMessagesBox.setBounds(getLocalBounds()
  //                               .withWidth(halfWidth)
  //                               .withX(halfWidth)
  //                               .reduced(10)
  //                               .withTrimmedBottom(100));
}

}  // namespace audio_plugin
