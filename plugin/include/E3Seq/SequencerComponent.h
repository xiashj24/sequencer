#pragma once
#include "E3Seq/PluginProcessor.h"
#include "E3Seq/TrackComponent.h"
#include "E3Seq/E3Sequencer.h"

namespace audio_plugin {

static const juce::Colour ColourPalette[8] = {
    {juce::Colours::crimson},     {juce::Colours::darkorange},
    {juce::Colours::gold},        {juce::Colours::limegreen},
    {juce::Colours::deepskyblue}, {juce::Colours::royalblue},
    {juce::Colours::darkviolet},  {juce::Colours::magenta}};
// TODO: polytrack colours

class SequencerComponent : public juce::Component {
public:
  SequencerComponent(AudioPluginAudioProcessor& p)
      : processorRef(p),
        monoTracks{{p, 0, ColourPalette[0]}, {p, 1, ColourPalette[1]},
                   {p, 2, ColourPalette[2]}, {p, 3, ColourPalette[3]},
                   {p, 4, ColourPalette[4]}, {p, 5, ColourPalette[5]},
                   {p, 6, ColourPalette[6]}, {p, 7, ColourPalette[7]}},
        polyTracks{{p, 8, ColourPalette[0]},
                   {p, 9, ColourPalette[1]},
                   {p, 10, ColourPalette[2]},
                   {p, 11, ColourPalette[3]}} {
    if (processorRef.wrapperType ==
        juce::AudioProcessor::WrapperType::wrapperType_VST3) {
      addAndMakeVisible(polyTracks[0]);
    } else {
      for (int i = 0; i < STEP_SEQ_NUM_MONO_TRACKS; ++i) {
        addAndMakeVisible(monoTracks[i]);
      }
      for (int i = 0; i < STEP_SEQ_NUM_POLY_TRACKS; ++i) {
        addAndMakeVisible(polyTracks[i]);
      }
    }

    updateSize();
  }

  void resized() override final {
    if (processorRef.wrapperType ==
        juce::AudioProcessor::WrapperType::wrapperType_VST3) {
      polyTracks[0].setBounds(0, 0, polyTracks[0].getWidth(),
                              polyTracks[0].getHeight());

    } else {
      int lastX = 0, lastY = 0;
      for (int i = 0; i < STEP_SEQ_NUM_MONO_TRACKS; ++i) {
        monoTracks[i].setBounds(lastX, lastY, monoTracks[i].getWidth(),
                                monoTracks[i].getHeight());
        lastY += monoTracks[i].getHeight() + 10;
      }
      for (int i = 0; i < STEP_SEQ_NUM_POLY_TRACKS; ++i) {
        polyTracks[i].setBounds(lastX, lastY, polyTracks[i].getWidth(),
                                polyTracks[i].getHeight());
        lastY += polyTracks[i].getHeight() + 10;
      }
      updateSize();
    }
  }

  void updateSize() {
    int width = monoTracks[0].getWidth();
    int height = 0;
    for (int i = 0; i < STEP_SEQ_NUM_MONO_TRACKS; ++i) {
      height += monoTracks[i].getHeight() + 10;
    }
    for (int i = 0; i < STEP_SEQ_NUM_POLY_TRACKS; ++i) {
      height += polyTracks[i].getHeight() + 10;
    }
    setSize(width, height);
  }

private:
  AudioPluginAudioProcessor& processorRef;
  MonoTrackComponent monoTracks[STEP_SEQ_NUM_MONO_TRACKS];
  PolyTrackComponent polyTracks[STEP_SEQ_NUM_POLY_TRACKS];
};
}  // namespace audio_plugin