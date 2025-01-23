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
  // MARK: sequencer programming
  E3Sequencer::Track& track_1 = sequencer_.getTrack(0);
  track_1.setEnabled(true); // TODO: ennable button
  track_1.setChannel(1);
  track_1.setLength(16);  // TODO: the UI should set the length

  // sequencer_.start();
}

// MARK: trigger note
juce::MidiMessage AudioPluginAudioProcessor::triggerNote(int noteNumber) {
  // construct MIDI messages
  auto note_on =
      juce::MidiMessage::noteOn(midiChannel, noteNumber, (juce::uint8)100);
  note_on.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);

  // auto note_off =
  //     juce::MidiMessage::noteOff(midiChannel, noteNumber, (juce::uint8)100);
  //     // support note off velocity?
  // note_off.setTimeStamp(note_on_time + 1000);  // gate last 1 second

  // add to local MIDI buffer
  midiCollector_.addMessageToQueue(note_on);
  // midiCollector.addMessageToQueue(note_off); // this doesn't work, seems like
  // Juce only handles noteOff by callbacks
  return note_on;
}

juce::MidiMessage AudioPluginAudioProcessor::allNotesOff() {
  auto message = juce::MidiMessage::allNotesOff(midiChannel);
  message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
  midiCollector_.addMessageToQueue(message);
  return message;
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
  midiCollector_.reset(sampleRate);  // TODO: verify that this is called after
                                     // sample rate changes in DAW
  sequencer_.setTickRate(sampleRate /
                         (double)samplesPerBlock);  // tick once per audio block
  sequencer_.setBpm(120.0);
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
  // read MIDI input from MIDI buffer and program sequencer

  midiMessages.clear();  // discard input MIDI messages
  // generate MIDI start/stop/continue messages by querying DAW transport
  // also set bpm
  if (auto playHead = getPlayHead()) {
    if (auto positionInfo = playHead->getPosition()) {
      sequencer_.setBpm(positionInfo->getBpm().orFallback(120.0));
      if (positionInfo->getIsPlaying()) {
        if (!sequencer_.isRunning())
          sequencer_.start();
      } else {
        if (sequencer_.isRunning())
          sequencer_.stop();
          // note: the host environment is responsible to send allNoteOff for all channels
      }
    }
  }



  // for (const auto metadata : midiMessages) {
  //   auto message = metadata.getMessage();

  //   if (message.isMidiStart()) {
  //     sequencer_.start();
  //   } else if (message.isMidiStop()) {
  //     sequencer_.stop();
  //   } else if (message.isMidiContinue()) {
  //     sequencer_.resume();
  //   }
  // }

  // ..sequencer logic here..
  E3Sequencer::NoteEvent note_on, note_off;
  sequencer_.tick(&note_on, &note_off, nullptr);

  // add note event to midiBuffer_
  seqMidiBuffer_.clear();

  if (note_on.enabled) {
    auto note_on_message = juce::MidiMessage::noteOn(
        note_on.channel, note_on.note, (juce::uint8)note_on.velocity);
    seqMidiBuffer_.addEvent(
        note_on_message, (int)(note_on.time_since_last_tick * getSampleRate()));
  }

  if (note_off.enabled) {
    auto note_off_message = juce::MidiMessage::noteOff(
        note_off.channel, note_off.note, (juce::uint8)note_off.velocity);
    seqMidiBuffer_.addEvent(
        note_off_message,
        (int)(note_off.time_since_last_tick * getSampleRate()));
  }

  // overwrite MIDI buffer
  midiMessages.swapWith(seqMidiBuffer_);
  midiCollector_.removeNextBlockOfMessages(midiMessages,
                                           getBlockSize());  // live play
  keyboardState.processNextMidiBuffer(midiMessages, 0, getBlockSize(), true);
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