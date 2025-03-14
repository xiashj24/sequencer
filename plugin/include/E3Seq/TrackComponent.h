#pragma once
#include "E3Seq/PluginProcessor.h"

namespace audio_plugin {

#define STEP_BUTTON_WIDTH 60
#define STEP_BUTTON_HEIGHT 30
#define STEP_BUTTON_SPACING 10

#define KNOB_HEIGHT 90
#define KNOB_TEXT_HEIGHT 20

// (since C++17) shared const across different translation units
inline juce::String OffsetText[TICKS_PER_STEP] = {
    "-1/2", "-11/24", "-5/12", "-3/8",  "-1/3", "-7/24", "-1/4", "-5/24",
    "-1/6", "-1/8",   "-1/12", "-1/24", "0",    "1/24",  "1/12", "1/8",
    "1/6",  "5/24",   "1/4",   "7/24",  "1/3",  "3/8",   "5/12", "11/24"};

inline juce::String RetriggerText[15] = {"Off", "0.17", "0.25", "0.33", "0.42",
                                         "0.5", "0.58", "0.67", "0.75", "0.83",
                                         "1",   "1.17", "1.25", "1.33", "1.5"};

inline float RetriggerValue[15] = {0.f,  // retrigger off
                                   4.f / TICKS_PER_STEP,
                                   6.f / TICKS_PER_STEP,
                                   8.f / TICKS_PER_STEP,
                                   10.f / TICKS_PER_STEP,
                                   12.f / TICKS_PER_STEP,
                                   14.f / TICKS_PER_STEP,
                                   16.f / TICKS_PER_STEP,
                                   18.f / TICKS_PER_STEP,
                                   20.f / TICKS_PER_STEP,
                                   24.f / TICKS_PER_STEP,
                                   28.f / TICKS_PER_STEP,
                                   30.f / TICKS_PER_STEP,
                                   32.f / TICKS_PER_STEP,
                                   36.f / TICKS_PER_STEP};

class TrackComponent : public juce::Component, private juce::Timer {
public:
  TrackComponent(AudioPluginAudioProcessor& p,
                 int trackNumber,
                 juce::Colour trackColor)
      : trackRef(p.sequencer.getTrack(trackNumber)), trackNumber_(trackNumber) {
    // look and feel
    // TODO: add page switching if STEP_SEQ_DEFAULT_LENGTH exceed 16

    startTimer(10);

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
                               trackColor);

      stepButtons[i].onClick = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        step.enabled = stepButtons[i].getToggleState();
        // stepButtons[i].setAlpha(step.enabled ? 1.f : DISABLED_ALPHA);
        noteKnobs[i].setVisible(step.enabled);
        lengthKnobs[i].setVisible(step.enabled);
        velocityKnobs[i].setVisible(step.enabled);
        offsetKnobs[i].setVisible(step.enabled);
        probabilityKnobs[i].setVisible(step.enabled);
        retriggerKnobs[i].setVisible(step.enabled);
        alternateKnobs[i].setVisible(step.enabled);
        trackRef.setStepAtIndex(i, step);
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
      noteKnobs[i].textFromValueFunction = [](double value) {
        return juce::MidiMessage::getMidiNoteName(static_cast<int>(value), true,
                                                  true, 4);
      };
      noteKnobs[i].setRange(21, 127, 1);
      noteKnobs[i].setDoubleClickReturnValue(true, DEFAULT_NOTE);
      noteKnobs[i].setValue(DEFAULT_NOTE);

      // TODO: text show note name instead of number

      noteKnobs[i].onValueChange = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        step.note = static_cast<int>(noteKnobs[i].getValue());
        trackRef.setStepAtIndex(i, step);
      };
      addChildComponent(noteKnobs[i]);
    }

    // length
    lengthLabel.setText("length", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(lengthLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      lengthKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      lengthKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                     STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      lengthKnobs[i].setSkewFactorFromMidPoint(4.0);
      lengthKnobs[i].setRange(0.083, STEP_SEQ_MAX_LENGTH, 0.01);
      lengthKnobs[i].setValue(DEFAULT_LENGTH);
      lengthKnobs[i].setDoubleClickReturnValue(true, DEFAULT_LENGTH);

      lengthKnobs[i].onValueChange = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        step.length = static_cast<float>(lengthKnobs[i].getValue());
        trackRef.setStepAtIndex(i, step);
      };

      addChildComponent(lengthKnobs[i]);
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
      velocityKnobs[i].setDoubleClickReturnValue(true, DEFAULT_VELOCITY);

      velocityKnobs[i].onValueChange = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        step.velocity = static_cast<int>(velocityKnobs[i].getValue());
        trackRef.setStepAtIndex(i, step);
      };

      addChildComponent(velocityKnobs[i]);
    }

    // offset
    offsetLabel.setText("offset", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(offsetLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      offsetKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      offsetKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                     STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      offsetKnobs[i].textFromValueFunction = [](double value) {
        int index = static_cast<int>(value * TICKS_PER_STEP) + 12;
        return OffsetText[index];
      };

      offsetKnobs[i].setRange(-0.5, 0.49, 0.01);
      offsetKnobs[i].setValue(0.0);
      offsetKnobs[i].setDoubleClickReturnValue(true, 0.0);

      offsetKnobs[i].onValueChange = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        step.offset = static_cast<float>(offsetKnobs[i].getValue());
        trackRef.setStepAtIndex(i, step);
      };
      addChildComponent(offsetKnobs[i]);
    }

    // retrigger
    retriggerLabel.setText("retrigger",
                           juce::NotificationType::dontSendNotification);
    addAndMakeVisible(retriggerLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      retriggerKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      retriggerKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                        STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);

      retriggerKnobs[i].textFromValueFunction = [](double value) {
        int index = static_cast<int>(value);
        return RetriggerText[index];
      };
      retriggerKnobs[i].setRange(0, 14, 1);
      retriggerKnobs[i].setValue(0);
      retriggerKnobs[i].setDoubleClickReturnValue(true, 1);

      retriggerKnobs[i].onValueChange = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        int index = static_cast<int>(retriggerKnobs[i].getValue());
        step.retrigger_rate = RetriggerValue[index];
        trackRef.setStepAtIndex(i, step);
      };

      addChildComponent(retriggerKnobs[i]);
    }

    // probability
    probabilityLabel.setText("probability",
                             juce::NotificationType::dontSendNotification);
    addAndMakeVisible(probabilityLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      probabilityKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      probabilityKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                          STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      probabilityKnobs[i].setRange(0, 1, 0.01);
      probabilityKnobs[i].setValue(1);
      probabilityKnobs[i].setDoubleClickReturnValue(true, 1);

      probabilityKnobs[i].onValueChange = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        step.probability = static_cast<float>(probabilityKnobs[i].getValue());
        trackRef.setStepAtIndex(i, step);
      };
      addChildComponent(probabilityKnobs[i]);
    }

    // alternate
    alternateLabel.setText("alternate",
                           juce::NotificationType::dontSendNotification);
    addAndMakeVisible(alternateLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      alternateKnobs[i].setSliderStyle(juce::Slider::LinearVertical);
      alternateKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                        STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      alternateKnobs[i].setRange(1, 4, 1);
      alternateKnobs[i].setValue(1);
      alternateKnobs[i].setDoubleClickReturnValue(true, 1);

      alternateKnobs[i].onValueChange = [this, i] {
        auto step = trackRef.getStepAtIndex(i);
        step.alternate = static_cast<int>(alternateKnobs[i].getValue());
        trackRef.setStepAtIndex(i, step);
      };
      addChildComponent(alternateKnobs[i]);
    }
  }

  // put all dynamic GUI processing here?
  // maybe try some async animation stuff?
  void timerCallback() override final {
    int playhead_index = trackRef.getCurrentStepIndex();

    for (int i = 0; i < STEP_SEQ_MAX_LENGTH; ++i) {
      if (i == playhead_index) {
        stepButtons[i].setAlpha(1.f);
      } else {
        stepButtons[i].setAlpha(0.7f);
      }
    }
  }

  void resized() override final {
    // layout
    trackCollapseButton.setBounds(0, 0, STEP_BUTTON_WIDTH, STEP_BUTTON_HEIGHT);
    noteLabel.setBounds(0, STEP_BUTTON_HEIGHT, STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    lengthLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT,
                          STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    velocityLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                            STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    offsetLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
                          STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    retriggerLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 4,
                             STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    probabilityLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 5,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    alternateLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 6,
                             STEP_BUTTON_WIDTH, KNOB_HEIGHT);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; i++) {
      int x = (i + 1) * (STEP_BUTTON_WIDTH + STEP_BUTTON_SPACING);
      stepButtons[i].setBounds(x, 0, STEP_BUTTON_WIDTH, STEP_BUTTON_HEIGHT);
      noteKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT, STEP_BUTTON_WIDTH,
                             KNOB_HEIGHT);
      lengthKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      velocityKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                                 STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      offsetKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      retriggerKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 4,
                                  STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      probabilityKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 5,
                                    STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      alternateKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 6,
                                  STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    }
  }

  void toggleCollapsed() {
    setCollapsed(!collapsed_);
    getParentComponent()->resized();
  }

private:
  Sequencer::Track& trackRef;
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
              STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 7);
      trackCollapseButton.setButtonText("Track " +
                                        juce::String(trackNumber_ + 1) + " ▼");
    }
  }

  juce::TextButton trackCollapseButton;
  juce::Label noteLabel;
  juce::Label lengthLabel;
  juce::Label velocityLabel;
  juce::Label offsetLabel;
  juce::Label retriggerLabel;
  juce::Label probabilityLabel;
  juce::Label alternateLabel;

  juce::TextButton stepButtons[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider noteKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider lengthKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider velocityKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider offsetKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider retriggerKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider probabilityKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider alternateKnobs[STEP_SEQ_DEFAULT_LENGTH];
};

}  // namespace audio_plugin