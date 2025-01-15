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
  // MARK: initialization
  // midiOutput = juce::MidiOutput::openDevice(
  //     juce::MidiOutput::getDefaultDevice().identifier); // TODO: make this
  //     configurable in menu in Standalone mode
  // startTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
}

void AudioPluginAudioProcessor::addMidiMessageToBuffer(
    const juce::MidiMessage& message) {
  auto timestamp = message.getTimeStamp();  // in seconds
  auto sampleNumber =
      (int)(timestamp *
            getSampleRate());  // Q: is it safe to call getSampleRate here?
  midiBuffer.addEvent(message, sampleNumber);
}

// MARK: trigger note
juce::MidiMessage AudioPluginAudioProcessor::triggerNote(int noteNumber) {
  // construct MIDI messages
  auto note_on =
      juce::MidiMessage::noteOn(midiChannel, noteNumber, (juce::uint8)100);
  note_on.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);

  auto note_off =
      juce::MidiMessage::noteOff(midiChannel, noteNumber, (juce::uint8)100);
  note_off.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001 +
                        1.0);  // gate last one second

  // add to local MIDI buffer
  addMidiMessageToBuffer(note_on);
  addMidiMessageToBuffer(note_off);
  return note_on;
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

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

  // MARK: MIDI processing
  // read MIDI input and program sequencer steps

  // sequencer logic here

  // write the messages in MIDI buffer / generated by sequencer to MIDI output
  int block_start_sample =
      juce::Time::getMillisecondCounterHiRes() * 0.001 *
      getSampleRate();  // TODO: this will probably overflow might have to
                        // implement some time updating mechanism
  for (const auto metadata : midiBuffer) {
    auto message = metadata.getMessage();
    // if it's NoteOn, trigger it immediately
    if (message.isNoteOn()) {
      int dt_in_samples = message.getTimeStamp() - block_start_sample;
      midiMessages.addEvent(message, std::max(dt_in_samples, 0));
    }
    // if it's NoteOff and inside the current audio block, schedule it
    if (message.isNoteOff()) {
      int dt_in_samples = message.getTimeStamp() - block_start_sample;
      if (dt_in_samples < getBlockSize()) {
        midiMessages.addEvent(
            message,
            std::max(dt_in_samples,
                     0));  // dt_in_samples won't be negative, just in case
      }
    }

    // clear obsolete MIDI messages
    midiBuffer.clear(0, block_start_sample);
    midiBuffer.clear(block_start_sample, getBlockSize());
  }
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