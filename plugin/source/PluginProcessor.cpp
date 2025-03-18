#include "E3Seq/PluginProcessor.h"
#include "E3Seq/PluginEditor.h"

#define TIMER_INTERVAL_MS 1

namespace audio_plugin {
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      sequencer(seqMidiCollector),
      parameters(*this,
                 &undoManager,
                 "E3Seq",
                 createParameterLayout()),  // TODO: undoManager
      lastCallbackTime(0.0) {
#if JUCE_MAC
  // create virtual MIDI out (Mac only)
  virtualMidiOut = juce::MidiOutput::createNewDevice("E3 Sequencer MIDI Out");
  if (virtualMidiOut) {
    virtualMidiOut->startBackgroundThread();
  }
#endif
  // store parameter pointers which can be safely accessed in timer thread
  for (int track = 0; track < STEP_SEQ_NUM_TRACKS; ++track) {
    for (int step = 0; step < STEP_SEQ_MAX_LENGTH; ++step) {
      juce::String prefix =
          "T" + juce::String(track) + "_S" + juce::String(step) + "_";
      enabled_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "ENABLED");
      note_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "NOTE");
      velocity_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "VELOCITY");
      offset_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "OFFSET");
      length_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "LENGTH");
      retrigger_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "RETRIGGER");
      probability_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "PROBABILITY");
      alternate_pointers[track][step] =
          parameters.getRawParameterValue(prefix + "ALTERNATE");
    }
  }

  sequencer.notifyProcessor = [this](int track_index, int step_index,
                                     Sequencer::Step step) {
    juce::String prefix =
        "T" + juce::String(track_index) + "_S" + juce::String(step_index) + "_";
    parameters.getParameter(prefix + "ENABLED")
        ->setValueNotifyingHost(static_cast<float>(step.enabled));

    auto p = parameters.getParameter(prefix + "NOTE");
    p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(step.note)));

    p = parameters.getParameter(prefix + "VELOCITY");
    p->setValueNotifyingHost(
        p->convertTo0to1(static_cast<float>(step.velocity)));

    p = parameters.getParameter(prefix + "OFFSET");
    p->setValueNotifyingHost(p->convertTo0to1(step.offset));

    p = parameters.getParameter(prefix + "LENGTH");
    p->setValueNotifyingHost(p->convertTo0to1(step.length));

    p = parameters.getParameter(prefix + "RETRIGGER");
    p->setValueNotifyingHost(p->convertTo0to1(step.retrigger_rate));

    p = parameters.getParameter(prefix + "PROBABILITY");
    p->setValueNotifyingHost(p->convertTo0to1(step.probability));

    p = parameters.getParameter(prefix + "ALTERNATE");
    p->setValueNotifyingHost(
        p->convertTo0to1(static_cast<float>(step.alternate)));
  };
  startTimer(TIMER_INTERVAL_MS);
}

const juce::String OffsetText[] = {
    "-1/2", "-11/24", "-5/12", "-3/8",  "-1/3", "-7/24", "-1/4", "-5/24",
    "-1/6", "-1/8",   "-1/12", "-1/24", "0",    "1/24",  "1/12", "1/8",
    "1/6",  "5/24",   "1/4",   "7/24",  "1/3",  "3/8",   "5/12", "11/24"};

const juce::String RetriggerText[] = {
    "Off", "1/12",  "1/6", "1/4",   "1/3", "5/12", "1/2", "7/12",  "2/3", "3/4",
    "5/6", "11/12", "1",   "13/12", "7/6", "5/4",  "4/3", "17/12", "3/2"};

juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout() {
  using namespace juce;
  AudioProcessorValueTreeState::ParameterLayout layout;
  for (int track = 0; track < STEP_SEQ_NUM_TRACKS; ++track) {
    for (int step = 0; step < STEP_SEQ_MAX_LENGTH; ++step) {
      String prefix = "T" + String(track) + "_S" + String(step) + "_";
      // MARK: parameter layout
      layout.add(std::make_unique<AudioParameterBool>(prefix + "ENABLED",
                                                      "Enabled", false));

      auto note_attributes =
          juce::AudioParameterIntAttributes{}.withStringFromValueFunction(
              [](int value, int maximumStringLength) {
                juce::ignoreUnused(maximumStringLength);
                return juce::MidiMessage::getMidiNoteName(value, true, true, 4);
              });
      layout.add(std::make_unique<AudioParameterInt>(
          prefix + "NOTE", "Note", 21, 127, DEFAULT_NOTE, note_attributes));

      layout.add(std::make_unique<AudioParameterInt>(
          prefix + "VELOCITY", "Velocity", 1, 127, DEFAULT_VELOCITY));

      auto offset_attributes =
          juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(
              [](float value, int maximumStringLength) {
                juce::ignoreUnused(maximumStringLength);
                int index = static_cast<int>(value * 24) + 12;
                return OffsetText[index];
              });
      layout.add(std::make_unique<AudioParameterFloat>(
          prefix + "OFFSET", "Offset",
          NormalisableRange<float>(-0.5f, 0.49f, 0.01f), 0.0f,
          offset_attributes));

      layout.add(std::make_unique<AudioParameterFloat>(
          prefix + "LENGTH", "Length",
          NormalisableRange<float>(0.08f, STEP_SEQ_MAX_LENGTH, 0.01f, 0.5f),
          static_cast<float>(DEFAULT_LENGTH)));

      auto retrigger_attributes =
          juce::AudioParameterFloatAttributes{}.withStringFromValueFunction(
              [](float value, int maximumStringLength) {
                juce::ignoreUnused(maximumStringLength);
                int index = static_cast<int>(value * 12);
                return RetriggerText[index];
              });
      layout.add(std::make_unique<AudioParameterFloat>(
          prefix + "RETRIGGER", "Retrigger Rate",
          NormalisableRange<float>(0.f, 1.5f, 1.f / 12.f), 0.f,
          retrigger_attributes));

      layout.add(std::make_unique<AudioParameterFloat>(
          prefix + "PROBABILITY", "Probability",
          NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

      layout.add(std::make_unique<AudioParameterInt>(prefix + "ALTERNATE",
                                                     "Alternate", 1, 4, 1));
    }
  }
  return layout;
}

void AudioPluginAudioProcessor::hiResTimerCallback() {
  // ..sequencer logic here..
  constexpr double deltaTime = TIMER_INTERVAL_MS / (double)1000;

  for (int i = 0; i < STEP_SEQ_NUM_TRACKS; ++i) {
    for (int j = 0; j < STEP_SEQ_MAX_LENGTH; ++j) {
      Sequencer::Step step{
          .enabled = static_cast<bool>(*(enabled_pointers[i][j])),
          .note = static_cast<int>(*(note_pointers[i][j])),
          .velocity = static_cast<int>(*(velocity_pointers[i][j])),
          .offset = *(offset_pointers[i][j]),
          .length = *(length_pointers[i][j]),
          .retrigger_rate = *(retrigger_pointers[i][j]),
          .probability = *(probability_pointers[i][j]),
          .alternate = static_cast<int>(*(alternate_pointers[i][j])),
      };
      sequencer.getTrack(i).setStepAtIndex(j, step, true);
    }
  }
  sequencer.process(deltaTime);
}

void AudioPluginAudioProcessor::panic() {
  for (int i = 0; i < 16; i++) {
    auto message = juce::MidiMessage::allNotesOff(i + 1);
    message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    guiMidiCollector.addMessageToQueue(message);
  }
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
  stopTimer();
}

const juce::String AudioPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are
             // 0 programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() {
  return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  juce::ignoreUnused(sampleRate, samplesPerBlock);

  // MARK: initialization
  seqMidiCollector.reset(sampleRate);
  guiMidiCollector.reset(sampleRate);
}

void AudioPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
  // juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // This is the place where you'd normally do the guts of your plugin's
  // audio processing...
  // Make sure to reset the state if your inner loop is processing
  // the samples and the outer loop is handling the channels.
  // Alternatively, you can process the samples with the channels
  // interleaved by keeping the same state.
  for (int channel = 0; channel < totalNumInputChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    juce::ignoreUnused(channelData);
    // ..do something to the audio data...
  }
  // MARK: process MIDI

  // process MIDI start/stop/continue messages
  for (const auto metadata : midiMessages) {
    auto message = metadata.getMessage();

    // auto current_time = juce::Time::getMillisecondCounter() * 0.001;
    auto time_stamp_in_seconds =
        message.getTimeStamp() / getSampleRate() + lastCallbackTime;

    if (message.isMidiStart()) {
      sequencer.start(time_stamp_in_seconds);
    } else if (message.isMidiStop()) {
      sequencer.stop();
    } else if (message.isMidiContinue()) {
      sequencer.resume();
    }
    // TODO: midi clock sync
    else if (message.isNoteOn()) {
      message.setTimeStamp(time_stamp_in_seconds);
      sequencer.handleNoteOn(message);
    } else if (message.isNoteOff()) {
      message.setTimeStamp(time_stamp_in_seconds);
      sequencer.handleNoteOff(message);
    }
  }

  lastCallbackTime = juce::Time::getMillisecondCounterHiRes() * 0.001;

  // generate MIDI start/stop/continue messages by querying DAW transport
  // also set bpm
  if (this->wrapperType ==
      juce::AudioProcessor::WrapperType::wrapperType_VST3) {
    if (auto dawPlayHead = getPlayHead()) {
      if (auto positionInfo = dawPlayHead->getPosition()) {
        sequencer.setBpm(positionInfo->getBpm().orFallback(120.0));

        if (positionInfo->getIsPlaying()) {
          if (!sequencer.isRunning())
            sequencer.start(juce::Time::getMillisecondCounterHiRes() * 0.001);
        } else {
          if (sequencer.isRunning())
            sequencer.stop();
        }
      }
    }
  }

  // midiMessages.clear();  // discard input MIDI messages

  // overwrite MIDI buffer
  seqMidiCollector.removeNextBlockOfMessages(midiMessages, getBlockSize());
  guiMidiCollector.removeNextBlockOfMessages(midiMessages, getBlockSize());
  // visualize MIDI in all channels and manual trigger
  keyboardState.processNextMidiBuffer(midiMessages, 0, getBlockSize(), true);

  // send the same MIDI messages to virtual MIDI out (Mac only)
#ifdef JUCE_MAC
  if (virtualMidiOut) {
    virtualMidiOut->sendBlockOfMessages(
        midiMessages, juce::Time::getMillisecondCounterHiRes(),
        getSampleRate());
  }
#endif
}

bool AudioPluginAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an
                // editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.

  if (this->wrapperType ==
      juce::AudioProcessor::WrapperType::wrapperType_Standalone)
    return;  // only recall parameters if run inside a DAW
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data,
                                                    int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr) {
    if (xmlState->hasTagName(parameters.state.getType())) {
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
  }
}

void AudioPluginAudioProcessor::savePreset(const juce::File& file) {
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  xml->writeTo(file);
}

void AudioPluginAudioProcessor::loadPreset(const juce::File& file) {
  std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(file);
  if (xml != nullptr && xml->hasTagName(parameters.state.getType())) {
    parameters.replaceState(juce::ValueTree::fromXml(*xml));
  }
}

void AudioPluginAudioProcessor::resetToDefaultState() {
  parameters.replaceState(juce::ValueTree(parameters.state.getType()));
}

}  // namespace audio_plugin

// This creates new instances of the plugin.
// This function definition must be in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new audio_plugin::AudioPluginAudioProcessor();
}