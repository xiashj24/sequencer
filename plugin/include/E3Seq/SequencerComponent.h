#pragma once
#include "E3Seq/PluginProcessor.h"
#include "E3Seq/TrackComponent.h"
#include "E3Seq/E3Sequencer.h"

namespace audio_plugin {

static const juce::Colour ColourPalette[8] = {
    {juce::Colours::crimson}, {juce::Colours::darkorange},
    {juce::Colours::gold}, {juce::Colours::limegreen},
    {juce::Colours::deepskyblue}, {juce::Colours::royalblue},
    {juce::Colours::darkviolet}, {juce::Colours::magenta}};

class SequencerComponent : public juce::Component {
public:
  SequencerComponent(AudioPluginAudioProcessor& p)
      : sequencerRef(p.sequencer),
        tracks{{p, 0, ColourPalette[0]}, {p, 1, ColourPalette[1]},
               {p, 2, ColourPalette[2]}, {p, 3, ColourPalette[3]},
               {p, 4, ColourPalette[4]}, {p, 5, ColourPalette[5]},
               {p, 6, ColourPalette[6]}, {p, 7, ColourPalette[7]}} {
    for (int i = 0; i < 8; i++) {
      addAndMakeVisible(tracks[i]);
    }
    updateSize();
  }

  void resized() override final {
    int lastX = 0, lastY = 0;
    for (int i = 0; i < 8; i++) {
      tracks[i].setBounds(lastX, lastY, tracks[i].getWidth(),
                          tracks[i].getHeight());
      lastY += tracks[i].getHeight() + 10;
    }
    updateSize();
  }

  void updateSize() {
    int width = tracks[0].getWidth();
    int height = 0;
    for (int i = 0; i < 8; ++i) {
      height += tracks[i].getHeight() + 10;
    }
    setSize(width, height);
  }

private:
  [[maybe_unused]] Sequencer::E3Sequencer& sequencerRef;
  TrackComponent tracks[8];
};
}  // namespace audio_plugin