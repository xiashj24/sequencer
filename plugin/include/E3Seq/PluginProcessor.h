﻿#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "E3Sequencer.h"

namespace audio_plugin {
class AudioPluginAudioProcessor : public juce::AudioProcessor {
public:
  AudioPluginAudioProcessor();
  ~AudioPluginAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  juce::MidiMessage triggerNote(int noteNumber);
  juce::MidiMessage allNotesOff();

  E3Sequencer& getSequencer() { return sequencer_; }

  juce::MidiKeyboardState keyboardState;  // visible to the editor
private:
  // MARK: member var
  const int midiChannel = 1;

  E3Sequencer sequencer_;

  juce::MidiMessageCollector midiCollector_;
  juce::MidiBuffer seqMidiBuffer_;
  // std::unique_ptr<juce::MidiInput> midiInput; // TODO: implement MIDI input
  // via MidiMessageCollector std::unique_ptr<juce::MidiOutput> midiOutput;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
}  // namespace audio_plugin
