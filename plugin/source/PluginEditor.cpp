#include "E3Seq/PluginEditor.h"
#include "E3Seq/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      sequencerEditor(p),
      onScreenKeyboard(p.keyboardState,
                       juce::MidiKeyboardComponent::horizontalKeyboard) {
  juce::ignoreUnused(processorRef);

  bpmLabel.setText("BPM: ", juce::NotificationType::dontSendNotification);
  bpmLabel.attachToComponent(&bpmSlider, true);
  bpmSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
  bpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40,
                            STEP_BUTTON_HEIGHT);
  bpmSlider.setRange(BPM_MIN, BPM_MAX, 1);
  bpmSlider.setValue(BPM_DEFAULT);
  bpmSlider.setDoubleClickReturnValue(true, BPM_DEFAULT);
  bpmSlider.onValueChange = [this] {
    processorRef.sequencer.setBpm(bpmSlider.getValue());
  };
  addAndMakeVisible(bpmSlider);

  keyboardMidiChannelLabel.setText(
      "Keyboard trigger MIDI channel: ",
      juce::NotificationType::dontSendNotification);
  keyboardMidiChannelLabel.attachToComponent(&keyboardMidiChannelSlider, true);
  keyboardMidiChannelSlider.setSliderStyle(
      juce::Slider::SliderStyle::IncDecButtons);
  keyboardMidiChannelSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false,
                                            30, STEP_BUTTON_HEIGHT);
  keyboardMidiChannelSlider.setRange(1, 16, 1);
  keyboardMidiChannelSlider.setValue(1);
  keyboardMidiChannelSlider.onValueChange = [this] {
    onScreenKeyboard.setMidiChannel((int)keyboardMidiChannelSlider.getValue());
  };
  addAndMakeVisible(keyboardMidiChannelSlider);

  panicButton.setButtonText("Panic!");
  panicButton.setTooltip("force all notes off");
  panicButton.onClick = [this] { processorRef.panic(); };
  addAndMakeVisible(panicButton);

  helpButton.setButtonText("?");
  helpButton.setTooltip("tips and shortcuts");
  helpButton.onClick = [this] { showHelpPopup(); };
  addAndMakeVisible(helpButton);

  if (processorRef.wrapperType ==
      juce::AudioProcessor::WrapperType::wrapperType_Standalone) {
    startButton.setButtonText(juce::String::fromUTF8("▶Start"));
    startButton.onClick = [this] {
      processorRef.sequencer.start(juce::Time::getMillisecondCounterHiRes() *
                                   0.001);
    };
    addAndMakeVisible(startButton);

    stopButton.setButtonText(juce::String::fromUTF8("⏹Stop"));
    stopButton.onClick = [this] { processorRef.sequencer.stop(); };
    addAndMakeVisible(stopButton);

    continueButton.setButtonText(juce::String::fromUTF8("⏭Cont"));
    continueButton.onClick = [this] { processorRef.sequencer.resume(); };
    addAndMakeVisible(continueButton);
  }

  addAndMakeVisible(sequencerViewport);
  // addAndMakeVisible(sequencerEditor);
  sequencerViewport.setViewedComponent(&sequencerEditor, false);

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
  setSize(1220, 700);
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
  auto bounds = getBounds();

  onScreenKeyboard.setBounds(bounds.removeFromBottom(90));

  auto utility_bar =
      bounds.removeFromBottom(STEP_BUTTON_HEIGHT + 20).reduced(10);

  startButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH));
  utility_bar.removeFromLeft(10);
  stopButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH));
  utility_bar.removeFromLeft(10);
  continueButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH));
  utility_bar.removeFromLeft(50);
  bpmSlider.setBounds(utility_bar.removeFromLeft(240));
  utility_bar.removeFromLeft(10);

  helpButton.setBounds(utility_bar.removeFromRight(STEP_BUTTON_HEIGHT));
  utility_bar.removeFromRight(10);
  panicButton.setBounds(utility_bar.removeFromRight(STEP_BUTTON_WIDTH));
  utility_bar.removeFromRight(10);
  keyboardMidiChannelSlider.setBounds(utility_bar.removeFromRight(80));
  utility_bar.removeFromRight(10);

  sequencerViewport.setBounds(bounds.reduced(10));
}

}  // namespace audio_plugin
