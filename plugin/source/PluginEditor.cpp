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
  panicButton.setButtonText("Panic!");
  panicButton.setTooltip("force all notes off");
  panicButton.onClick = [this] { processorRef.panic(); };
  addAndMakeVisible(panicButton);

  helpButton.setButtonText("?");
  helpButton.setTooltip("tips and shortcuts");
  helpButton.onClick = [this] { showHelpPopup(); };
  addAndMakeVisible(helpButton);

  recordButton.setButtonText(juce::String::fromUTF8("⏺Rec"));
  recordButton.setClickingTogglesState(true);
  recordButton.addShortcut(juce::KeyPress('r'));
  recordButton.setTooltip("toggle real-time recording (r)");
  recordButton.setColour(juce::TextButton::ColourIds::buttonOnColourId,
                         juce::Colours::orangered);
  recordButton.onClick = [this] {
    processorRef.sequencer.setArmed(recordButton.getToggleState());
  };
  addAndMakeVisible(recordButton);

  smartButton.setButtonText(juce::String::fromUTF8("Smart Overdub™"));
  smartButton.setClickingTogglesState(true);
  smartButton.setTooltip("toggle smart overdub");
  smartButton.setColour(juce::TextButton::ColourIds::buttonOnColourId,
                         juce::Colours::orangered);
  smartButton.onClick = [this] {
    processorRef.sequencer.setEnableSmartOverdub(smartButton.getToggleState());
  };
  addAndMakeVisible(smartButton);

  if (processorRef.wrapperType ==
      juce::AudioProcessor::WrapperType::wrapperType_Standalone) {
    playButton.setButtonText(juce::String::fromUTF8("⏯Play"));
    playButton.setClickingTogglesState(true);
    playButton.setColour(juce::TextButton::ColourIds::buttonOnColourId,
                         juce::Colours::orangered);
    playButton.addShortcut(juce::KeyPress(juce::KeyPress::spaceKey));
    playButton.setTooltip("toggle play and pause (space)");
    playButton.onClick = [this] {
      if (processorRef.sequencer.neverStarted()) {
        processorRef.sequencer.start(juce::Time::getMillisecondCounterHiRes() *
                                     0.001);
      } else if (processorRef.sequencer.isRunning()) {
        processorRef.sequencer.stop();
      } else {
        processorRef.sequencer.resume();
      }
    };
    addAndMakeVisible(playButton);

    stopButton.setButtonText(juce::String::fromUTF8("⏹Stop"));
    stopButton.addShortcut(juce::KeyPress('s'));
    stopButton.setTooltip("stop playback and move to start position (s)");
    stopButton.onClick = [this] {
      processorRef.sequencer.start(juce::Time::getMillisecondCounterHiRes() *
                                   0.001);
      processorRef.sequencer.stop();
      playButton.setToggleState(false,
                                juce::NotificationType::dontSendNotification);
    };
    addAndMakeVisible(stopButton);

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
    keyboardMidiChannelLabel.attachToComponent(&keyboardMidiChannelSlider,
                                               true);
    keyboardMidiChannelSlider.setSliderStyle(
        juce::Slider::SliderStyle::IncDecButtons);
    keyboardMidiChannelSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false,
                                              30, STEP_BUTTON_HEIGHT);
    keyboardMidiChannelSlider.setRange(1, 16, 1);
    keyboardMidiChannelSlider.setValue(1);
    keyboardMidiChannelSlider.onValueChange = [this] {
      onScreenKeyboard.setMidiChannel(
          (int)keyboardMidiChannelSlider.getValue());
    };
    addAndMakeVisible(keyboardMidiChannelSlider);
  }

  quantizeButton.setButtonText("Quantize");
  quantizeButton.setClickingTogglesState(true);
  quantizeButton.setTooltip(
      "quantize note-on timing for real-time recording (q)");
  quantizeButton.addShortcut(juce::KeyPress('q'));
  quantizeButton.setColour(juce::TextButton::ColourIds::buttonOnColourId,
                           juce::Colours::orangered);
  quantizeButton.onClick = [this] {
    processorRef.sequencer.setQuantizeRec(quantizeButton.getToggleState());
  };
  addAndMakeVisible(quantizeButton);

  addAndMakeVisible(sequencerViewport);
  sequencerViewport.setViewedComponent(&sequencerEditor, false);

  addAndMakeVisible(onScreenKeyboard);

  // TODO: use command system for save and load

  presetFolder =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile("E3Seq");
  if (!presetFolder.exists()) {
    presetFolder.createDirectory();
  }
  presetLoader =
      std::make_unique<juce::FileChooser>("Load Preset", presetFolder, "*.xml");
  presetSaver =
      std::make_unique<juce::FileChooser>("Save Preset", presetFolder, "*.xml");

  savePresetButton.setButtonText("Save");
  savePresetButton.setTooltip("save preset as a XML file (ctrl+s)");
  savePresetButton.onClick = [this]() { savePreset(); };
  addAndMakeVisible(savePresetButton);

  loadPresetButton.setButtonText("Load");
  loadPresetButton.setTooltip("load preset from a XML file (ctrl+l)");
  loadPresetButton.onClick = [this]() { loadPreset(); };
  addAndMakeVisible(loadPresetButton);

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

void AudioPluginAudioProcessorEditor::savePreset() {
  presetSaver->launchAsync(juce::FileBrowserComponent::saveMode |
                               juce::FileBrowserComponent::canSelectFiles,
                           [this](const juce::FileChooser& chooser) {
                             juce::File file = chooser.getResult();
                             if (file != juce::File{}) {
                               processorRef.savePreset(file);
                             }
                           });
}

void AudioPluginAudioProcessorEditor::loadPreset() {
  presetLoader->launchAsync(juce::FileBrowserComponent::openMode |
                                juce::FileBrowserComponent::canSelectFiles,
                            [this](const juce::FileChooser& chooser) {
                              processorRef.loadPreset(chooser.getResult());
                            });
  stopButton.triggerClick();
}

void AudioPluginAudioProcessorEditor::resized() {
  // MARK: GUI layout
  auto bounds = getBounds();

  onScreenKeyboard.setBounds(bounds.removeFromBottom(90));

  auto utility_bar =
      bounds.removeFromBottom(STEP_BUTTON_HEIGHT + 20).reduced(10);

  quantizeButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH));
  utility_bar.removeFromLeft(10);
  recordButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH));
  utility_bar.removeFromLeft(10);
  playButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH));
  utility_bar.removeFromLeft(10);
  stopButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH));
  utility_bar.removeFromLeft(10);
  smartButton.setBounds(utility_bar.removeFromLeft(STEP_BUTTON_WIDTH*2));
  utility_bar.removeFromLeft(10);

  utility_bar.removeFromLeft(40);
  bpmSlider.setBounds(utility_bar.removeFromLeft(200));
  utility_bar.removeFromLeft(10);

  helpButton.setBounds(utility_bar.removeFromRight(STEP_BUTTON_HEIGHT));
  utility_bar.removeFromRight(10);
  panicButton.setBounds(utility_bar.removeFromRight(STEP_BUTTON_WIDTH));
  utility_bar.removeFromRight(10);
  loadPresetButton.setBounds(utility_bar.removeFromRight(STEP_BUTTON_WIDTH));
  utility_bar.removeFromRight(10);
  savePresetButton.setBounds(utility_bar.removeFromRight(STEP_BUTTON_WIDTH));
  utility_bar.removeFromRight(10);
  keyboardMidiChannelSlider.setBounds(utility_bar.removeFromRight(80));
  utility_bar.removeFromRight(10);

  sequencerViewport.setBounds(bounds.reduced(10));
}

}  // namespace audio_plugin
