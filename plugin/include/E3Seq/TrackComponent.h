#pragma once
#include "PluginProcessor.h"

namespace audio_plugin {

#define STEP_BUTTON_WIDTH 60
#define STEP_BUTTON_HEIGHT 30
#define STEP_BUTTON_SPACING 10

#define KNOB_HEIGHT 90
#define KNOB_TEXT_HEIGHT 20

class TrackComponent : public juce::Component {
public:
  TrackComponent(AudioPluginAudioProcessor& p, int trackNumber)
      : sequencerRef(p.getSequencer()), trackNumber_(trackNumber) {
    // look and feel
    // TODO: add page switching if STEP_SEQ_DEFAULT_LENGTH exceed 16

    setCollapsed(true);
    addAndMakeVisible(trackCollapseButton);
    trackCollapseButton.onClick = [this] {
      toggleCollapsed();
      if (auto* parent = findParentComponentOfClass<juce::Component>()) {
        parent->resized();
      }
    };

    // step buttons
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      stepButtons[i].setButtonText(juce::String(i + 1));
      stepButtons[i].setClickingTogglesState(true);
      stepButtons[i].setColour(juce::TextButton::ColourIds::buttonOnColourId,
                               juce::Colours::orangered);

      stepButtons[i].onClick = [this, i] {
        sequencerRef.getTrack(trackNumber_)[i].enabled =
            stepButtons[i].getToggleState();
      };
      addAndMakeVisible(stepButtons[i]);
    }

    // note
    noteLabel.setText("note", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(noteLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      noteKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      noteKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                   STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      noteKnobs[i].setRange(21, 127, 1);
      noteKnobs[i].setValue(DEFAULT_NOTE);
      // TODO: text show note name instead of number

      noteKnobs[i].onValueChange = [this, i] {
        sequencerRef.getTrack(trackNumber_)[i].note =
            (int)noteKnobs[i].getValue();
      };
      addAndMakeVisible(noteKnobs[i]);
    }

    // gate
    gateLabel.setText("gate", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(gateLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      gateKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      gateKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                   STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      gateKnobs[i].setRange(0.05, 1.0, 0.01);
      gateKnobs[i].setValue(DEFAULT_GATE);

      gateKnobs[i].onValueChange = [this, i] {
        sequencerRef.getTrack(trackNumber_)[i].gate = gateKnobs[i].getValue();
      };

      addAndMakeVisible(gateKnobs[i]);
    }

    // velocity
    velocityLabel.setText("velocity",
                          juce::NotificationType::dontSendNotification);
    addAndMakeVisible(velocityLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      velocityKnobs[i].setSliderStyle(juce::Slider::LinearVertical);
      velocityKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                       STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      velocityKnobs[i].setRange(1, 127, 1);
      velocityKnobs[i].setValue(DEFAULT_VELOCITY);

      velocityKnobs[i].onValueChange = [this, i] {
        sequencerRef.getTrack(trackNumber_)[i].velocity =
            (int)velocityKnobs[i].getValue();
      };

      addAndMakeVisible(velocityKnobs[i]);
    }

    // offset
    offsetLabel.setText("offset", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(offsetLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      offsetKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      offsetKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                     STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      offsetKnobs[i].setRange(-0.5, 0.49, 0.01);
      offsetKnobs[i].setValue(0.0);

      offsetKnobs[i].onValueChange = [this, i] {
        sequencerRef.getTrack(trackNumber_)[i].offset =
            offsetKnobs[i].getValue();
      };  // bug: step 1 minus offset don't work
      addAndMakeVisible(offsetKnobs[i]);
    }

    // overall component size
  }

  void resized() override {
    // layout
    trackCollapseButton.setBounds(0, 0, STEP_BUTTON_WIDTH, STEP_BUTTON_HEIGHT);
    noteLabel.setBounds(0, STEP_BUTTON_HEIGHT, STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    gateLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT, STEP_BUTTON_WIDTH,
                        KNOB_HEIGHT);
    velocityLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                            STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    offsetLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
                          STEP_BUTTON_WIDTH, KNOB_HEIGHT);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      int x = (i + 1) * (STEP_BUTTON_WIDTH + STEP_BUTTON_SPACING);
      stepButtons[i].setBounds(x, 0, STEP_BUTTON_WIDTH, STEP_BUTTON_HEIGHT);
      noteKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT, STEP_BUTTON_WIDTH,
                             KNOB_HEIGHT);
      gateKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT,
                             STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      velocityKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                                 STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      offsetKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    }
  }

  void toggleCollapsed() { setCollapsed(!collapsed_); }

private:
  Sequencer::E3Sequencer& sequencerRef;
  int trackNumber_;
  bool collapsed_;

  void setCollapsed(bool collapsed) {
    collapsed_ = collapsed;
    if (collapsed) {
      setSize(STEP_BUTTON_WIDTH * (STEP_SEQ_DEFAULT_LENGTH + 1) +
                  STEP_BUTTON_SPACING * STEP_SEQ_DEFAULT_LENGTH,
              STEP_BUTTON_HEIGHT);
      trackCollapseButton.setButtonText("Track " +
                                        juce::String(trackNumber_ + 1) + " ▶");
    } else {
      setSize(STEP_BUTTON_WIDTH * (STEP_SEQ_DEFAULT_LENGTH + 1) +
                  STEP_BUTTON_SPACING * STEP_SEQ_DEFAULT_LENGTH,
              STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 4);
      trackCollapseButton.setButtonText("Track " +
                                        juce::String(trackNumber_ + 1) + " ▼");
    }
  }

  juce::TextButton trackCollapseButton;
  juce::Label noteLabel;
  juce::Label gateLabel;
  juce::Label velocityLabel;
  juce::Label offsetLabel;

  juce::TextButton stepButtons[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider noteKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider gateKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider velocityKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider offsetKnobs[STEP_SEQ_DEFAULT_LENGTH];
};

}  // namespace audio_plugin