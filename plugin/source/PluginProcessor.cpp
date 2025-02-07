#include "E3Seq/PluginProcessor.h"
#include "E3Seq/PluginEditor.h"

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
      ) {
// create virtual MIDI port (Mac only)
#if JUCE_MAC
  virtualMidiOut = juce::MidiOutput::createNewDevice("E3 Sequencer MIDI Out");
  if (virtualMidiOut) {
    virtualMidiOut->startBackgroundThread();
  }
#endif
}

void AudioPluginAudioProcessor::allNotesOff() {
  for (int i = 0; i < 16; i++) {
    auto message = juce::MidiMessage::allNotesOff(i + 1);
    message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    guiMidiCollector.addMessageToQueue(message);
  }
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

const juce::String AudioPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
  // #if JucePlugin_WantsMidiInput
  //   return true;
  // #else
  //   return false;
  // #endif
  return true;
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
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
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
  resetSeqMidiCollector(sampleRate);
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
  // live recording: read MIDI input from MIDI buffer and program sequencer

  // receive MIDI start/stop/continue messages
  for (const auto metadata : midiMessages) {
    auto message = metadata.getMessage();

    if (message.isMidiStart()) {
      sequencer.start();
    } else if (message.isMidiStop()) {
      sequencer.stop();
    } else if (message.isMidiContinue()) {
      sequencer.resume();
    }
  }

  midiMessages.clear();  // discard input MIDI messages
  // generate MIDI start/stop/continue messages by querying DAW transport
  // also set bpm
  if (this->wrapperType ==
      juce::AudioProcessor::WrapperType::wrapperType_VST3) {
    if (auto dawPlayHead = getPlayHead()) {
      if (auto positionInfo = dawPlayHead->getPosition()) {
        sequencer.setBpm(positionInfo->getBpm().orFallback(120.0));

        if (positionInfo->getIsPlaying()) {
          if (!sequencer.isRunning())
            sequencer.start();
        } else {
          if (sequencer.isRunning())
            sequencer.stop();
          // note: the host environment is responsible to send allNoteOff for
          // all channels
        }
      }
    }
  }

  // ..sequencer logic here..
  resetSeqMidiCollector(getSampleRate());
  sequencer.process(getBlockSize()/getSampleRate(), seqMidiCollector);

  // overwrite MIDI buffer
  seqMidiCollector.removeNextBlockOfMessages(midiMessages, getBlockSize());
  guiMidiCollector.removeNextBlockOfMessages(midiMessages, getBlockSize());
  keyboardState.processNextMidiBuffer(midiMessages, 0, getBlockSize(), true);

  // send the same MIDI messages to virtual MIDI out
#ifdef JUCE_MAC
  if (virtualMidiOut) {
    virtualMidiOut->sendBlockOfMessages(
        midiMessages, juce::Time::getMillisecondCounterHiRes(),
        getSampleRate());
  }
#endif
}

bool AudioPluginAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data,
                                                    int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  juce::ignoreUnused(data, sizeInBytes);
}

}  // namespace audio_plugin

// This creates new instances of the plugin.
// This function definition must be in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new audio_plugin::AudioPluginAudioProcessor();
}