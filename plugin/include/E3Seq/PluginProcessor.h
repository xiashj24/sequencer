#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "E3Seq/E3Sequencer.h"

namespace audio_plugin {
class AudioPluginAudioProcessor : public juce::AudioProcessor,
                                  private juce::HighResolutionTimer {
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

  virtual void hiResTimerCallback() override final;

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

  void panic();

  void savePreset(const juce::File& file);
  void loadPreset(const juce::File& file);
  void resetToDefaultState();

  // global sequencer instance
  Sequencer::E3Sequencer sequencer;

  juce::MidiKeyboardState keyboardState;

  // all recall-able sequencer parameters
  juce::AudioProcessorValueTreeState parameters;

  juce::UndoManager undoManager;

private:
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  std::atomic<float>* enabled_pointers[STEP_SEQ_NUM_TRACKS]
                                      [STEP_SEQ_MAX_LENGTH];
  std::atomic<float>* note_pointers[STEP_SEQ_NUM_TRACKS][STEP_SEQ_MAX_LENGTH];
  std::atomic<float>* velocity_pointers[STEP_SEQ_NUM_TRACKS]
                                       [STEP_SEQ_MAX_LENGTH];
  std::atomic<float>* offset_pointers[STEP_SEQ_NUM_TRACKS][STEP_SEQ_MAX_LENGTH];
  std::atomic<float>* length_pointers[STEP_SEQ_NUM_TRACKS][STEP_SEQ_MAX_LENGTH];
  std::atomic<float>* retrigger_pointers[STEP_SEQ_NUM_TRACKS]
                                        [STEP_SEQ_MAX_LENGTH];
  std::atomic<float>* probability_pointers[STEP_SEQ_NUM_TRACKS]
                                          [STEP_SEQ_MAX_LENGTH];
  std::atomic<float>* alternate_pointers[STEP_SEQ_NUM_TRACKS]
                                        [STEP_SEQ_MAX_LENGTH];

  juce::MidiMessageCollector guiMidiCollector;
  juce::MidiMessageCollector seqMidiCollector;

  std::unique_ptr<juce::MidiOutput> virtualMidiOut;

  double lastCallbackTime;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
}  // namespace audio_plugin
