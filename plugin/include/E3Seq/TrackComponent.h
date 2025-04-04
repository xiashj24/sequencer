#pragma once
#include "E3Seq/PluginProcessor.h"

namespace audio_plugin {

#define STEP_BUTTON_WIDTH 60
#define STEP_BUTTON_HEIGHT 30
#define STEP_BUTTON_SPACING 10

#define KNOB_HEIGHT 90
#define KNOB_TEXT_HEIGHT 20

// TODO: refactor to reduce code duplication

class MonoTrackComponent : public juce::Component, private juce::Timer {
public:
  MonoTrackComponent(AudioPluginAudioProcessor& p,
                     int trackIndex,
                     juce::Colour trackColor)
      : processorRef(p),
        trackRef(p.sequencer.getMonoTrack(trackIndex)),
        trackIndex_(trackIndex) {
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
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      stepButtons[i].setButtonText(juce::String(i + 1));
      stepButtons[i].setClickingTogglesState(true);
      stepButtons[i].setColour(juce::TextButton::ColourIds::buttonOnColourId,
                               trackColor);

      stepButtons[i].onStateChange = [this, i] {
        bool visible = stepButtons[i].getToggleState();
        noteKnobs[i].setVisible(visible);
        velocityKnobs[i].setVisible(visible);
        offsetKnobs[i].setVisible(visible);
        lengthKnobs[i].setVisible(visible);
        probabilityKnobs[i].setVisible(visible);
        retriggerKnobs[i].setVisible(visible);
        alternateKnobs[i].setVisible(visible);
      };

      addAndMakeVisible(stepButtons[i]);
    }

    // note
    noteLabel.setText("note", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(noteLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      noteKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      noteKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                   STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(noteKnobs[i]);
    }

    // velocity
    velocityLabel.setText("velocity",
                          juce::NotificationType::dontSendNotification);
    addAndMakeVisible(velocityLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      velocityKnobs[i].setSliderStyle(juce::Slider::LinearVertical);
      velocityKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                       STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(velocityKnobs[i]);
    }

    // offset
    offsetLabel.setText("offset", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(offsetLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      offsetKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      offsetKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                     STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(offsetKnobs[i]);
    }

    // length
    lengthLabel.setText("length", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(lengthLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      lengthKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      lengthKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                     STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(lengthKnobs[i]);
    }

    // retrigger
    retriggerLabel.setText("retrigger",
                           juce::NotificationType::dontSendNotification);
    addAndMakeVisible(retriggerLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      retriggerKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      retriggerKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                        STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(retriggerKnobs[i]);
    }

    // probability
    probabilityLabel.setText("probability",
                             juce::NotificationType::dontSendNotification);
    addAndMakeVisible(probabilityLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      probabilityKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      probabilityKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                          STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(probabilityKnobs[i]);
    }

    // alternate
    alternateLabel.setText("alternate",
                           juce::NotificationType::dontSendNotification);
    addAndMakeVisible(alternateLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      alternateKnobs[i].setSliderStyle(juce::Slider::LinearVertical);
      alternateKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                        STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);

      addChildComponent(alternateKnobs[i]);
    }

    // attach buttons and sliders to processor parameters
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      juce::String prefix =
          "T" + juce::String(trackIndex_) + "_S" + juce::String(i) + "_";

      enableAttachments[i] = std::make_unique<ButtonAttachment>(
          processorRef.parameters, prefix + "ENABLED", stepButtons[i]);

      noteAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "NOTE", noteKnobs[i]);

      velocityAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "VELOCITY", velocityKnobs[i]);

      offsetAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "OFFSET", offsetKnobs[i]);

      lengthAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "LENGTH", lengthKnobs[i]);

      retriggerAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "RETRIGGER", retriggerKnobs[i]);

      probabilityAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "PROBABILITY", probabilityKnobs[i]);

      alternateAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "ALTERNATE", alternateKnobs[i]);
    }
  }

  // TODO: try some async animation stuff?
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
    velocityLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 1,
                            STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    offsetLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                          STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    lengthLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
                          STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    retriggerLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 4,
                             STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    probabilityLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 5,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    alternateLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 6,
                             STEP_BUTTON_WIDTH, KNOB_HEIGHT);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      int x = (i + 1) * (STEP_BUTTON_WIDTH + STEP_BUTTON_SPACING);
      stepButtons[i].setBounds(x, 0, STEP_BUTTON_WIDTH, STEP_BUTTON_HEIGHT);
      noteKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT, STEP_BUTTON_WIDTH,
                             KNOB_HEIGHT);
      velocityKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT,
                                 STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      offsetKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      lengthKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
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
  AudioPluginAudioProcessor& processorRef;
  Sequencer::Track& trackRef;
  int trackIndex_;
  bool collapsed_;

  void setCollapsed(bool collapsed) {
    collapsed_ = collapsed;
    if (collapsed) {
      setSize(STEP_BUTTON_WIDTH * (STEP_SEQ_DEFAULT_LENGTH + 1) +
                  STEP_BUTTON_SPACING * STEP_SEQ_DEFAULT_LENGTH,
              STEP_BUTTON_HEIGHT);
      trackCollapseButton.setButtonText("Track " +
                                        juce::String(trackIndex_ + 1) + " ▶");
    } else {
      setSize(STEP_BUTTON_WIDTH * (STEP_SEQ_DEFAULT_LENGTH + 1) +
                  STEP_BUTTON_SPACING * STEP_SEQ_DEFAULT_LENGTH,
              STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 7);
      trackCollapseButton.setButtonText("Track " +
                                        juce::String(trackIndex_ + 1) + " ▼");
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

  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
  using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

  // Parameter attachments
  std::unique_ptr<ButtonAttachment> enableAttachments[STEP_SEQ_DEFAULT_LENGTH];
  std::unique_ptr<SliderAttachment> noteAttachments[STEP_SEQ_DEFAULT_LENGTH],
      velocityAttachments[STEP_SEQ_DEFAULT_LENGTH],
      offsetAttachments[STEP_SEQ_DEFAULT_LENGTH],
      lengthAttachments[STEP_SEQ_DEFAULT_LENGTH],
      retriggerAttachments[STEP_SEQ_DEFAULT_LENGTH],
      probabilityAttachments[STEP_SEQ_DEFAULT_LENGTH],
      alternateAttachments[STEP_SEQ_DEFAULT_LENGTH];
};

// MARK: poly track
class PolyTrackComponent : public juce::Component, private juce::Timer {
public:
  PolyTrackComponent(AudioPluginAudioProcessor& p,
                     int trackIndex,
                     juce::Colour trackColor)
      : processorRef(p),
        trackRef(p.sequencer.getPolyTrack(trackIndex-STEP_SEQ_NUM_MONO_TRACKS)),
        trackIndex_(trackIndex) {
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
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      stepButtons[i].setButtonText(juce::String(i + 1));
      stepButtons[i].setClickingTogglesState(true);
      stepButtons[i].setColour(juce::TextButton::ColourIds::buttonOnColourId,
                               trackColor);

      stepButtons[i].onStateChange = [this, i] {
        bool visible = stepButtons[i].getToggleState();
        noteOneKnobs[i].setVisible(visible);
        noteTwoKnobs[i].setVisible(visible);
        noteThreeKnobs[i].setVisible(visible);
        noteFourKnobs[i].setVisible(visible);
        velocityKnobs[i].setVisible(visible);
        offsetKnobs[i].setVisible(visible);
        lengthKnobs[i].setVisible(visible);
        probabilityKnobs[i].setVisible(visible);
      };

      addAndMakeVisible(stepButtons[i]);
    }

    // note 1
    noteOneLabel.setText("note 1",
                         juce::NotificationType::dontSendNotification);
    addAndMakeVisible(noteOneLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      noteOneKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      noteOneKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                      STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(noteOneKnobs[i]);
    }

    // note 2
    noteTwoLabel.setText("note 2",
                         juce::NotificationType::dontSendNotification);
    addAndMakeVisible(noteTwoLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      noteTwoKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      noteTwoKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                      STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(noteTwoKnobs[i]);
    }

    // note 3
    noteThreeLabel.setText("note 3",
                           juce::NotificationType::dontSendNotification);
    addAndMakeVisible(noteThreeLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      noteThreeKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      noteThreeKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                        STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(noteThreeKnobs[i]);
    }

    // note 4
    noteFourLabel.setText("note 4",
                          juce::NotificationType::dontSendNotification);
    addAndMakeVisible(noteFourLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      noteFourKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      noteFourKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                       STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(noteFourKnobs[i]);
    }

    // velocity
    velocityLabel.setText("velocity",
                          juce::NotificationType::dontSendNotification);
    addAndMakeVisible(velocityLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      velocityKnobs[i].setSliderStyle(juce::Slider::LinearVertical);
      velocityKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                       STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      // set velocity of all notes inside the step
      velocityKnobs[i].onValueChange = [this, i]() {
        juce::String prefix =
            "T" + juce::String(trackIndex_) + "_S" + juce::String(i) + "_";

        for (int j = 1; j < POLYPHONY; ++j) {
          juce::String note_signifier = "N" + juce::String(j) + "_";
          auto other_velocity = processorRef.parameters.getParameter(
              prefix + note_signifier + "VELOCITY");
          other_velocity->setValueNotifyingHost(other_velocity->convertTo0to1(
              static_cast<float>(velocityKnobs[i].getValue())));
        }
      };
      addChildComponent(velocityKnobs[i]);
    }

    // offset
    offsetLabel.setText("offset", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(offsetLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      offsetKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      offsetKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                     STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      offsetKnobs[i].onValueChange = [this, i]() {
        juce::String prefix =
            "T" + juce::String(trackIndex_) + "_S" + juce::String(i) + "_";

        for (int j = 1; j < POLYPHONY; ++j) {
          juce::String note_signifier = "N" + juce::String(j) + "_";
          auto other_offset = processorRef.parameters.getParameter(
              prefix + note_signifier + "OFFSET");
          other_offset->setValueNotifyingHost(other_offset->convertTo0to1(
              static_cast<float>(offsetKnobs[i].getValue())));
        }
      };

      addChildComponent(offsetKnobs[i]);
    }

    // length
    lengthLabel.setText("length", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(lengthLabel);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      lengthKnobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
      lengthKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                     STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);

      lengthKnobs[i].onValueChange = [this, i]() {
        juce::String prefix =
            "T" + juce::String(trackIndex_) + "_S" + juce::String(i) + "_";

        for (int j = 1; j < POLYPHONY; ++j) {
          juce::String note_signifier = "N" + juce::String(j) + "_";
          auto other_offset = processorRef.parameters.getParameter(
              prefix + note_signifier + "LENGTH");
          other_offset->setValueNotifyingHost(other_offset->convertTo0to1(
              static_cast<float>(lengthKnobs[i].getValue())));
        }
      };
      addChildComponent(lengthKnobs[i]);
    }

    // probability
    probabilityLabel.setText("probability",
                             juce::NotificationType::dontSendNotification);
    addAndMakeVisible(probabilityLabel);
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      probabilityKnobs[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
      probabilityKnobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                          STEP_BUTTON_WIDTH, KNOB_TEXT_HEIGHT);
      addChildComponent(probabilityKnobs[i]);
    }

    // attach buttons and sliders to processor parameters
    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      juce::String prefix =
          "T" + juce::String(trackIndex_) + "_S" + juce::String(i) + "_";

      enableAttachments[i] = std::make_unique<ButtonAttachment>(
          processorRef.parameters, prefix + "ENABLED", stepButtons[i]);

      noteOneAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "N0_NOTE", noteOneKnobs[i]);

      noteTwoAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "N1_NOTE", noteTwoKnobs[i]);

      noteThreeAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "N2_NOTE", noteThreeKnobs[i]);

      noteFourAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "N3_NOTE", noteFourKnobs[i]);

      velocityAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "N0_VELOCITY", velocityKnobs[i]);

      offsetAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "N0_OFFSET", offsetKnobs[i]);

      lengthAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "N0_LENGTH", lengthKnobs[i]);

      probabilityAttachments[i] = std::make_unique<SliderAttachment>(
          processorRef.parameters, prefix + "PROBABILITY", probabilityKnobs[i]);
    }
  }

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
    noteOneLabel.setBounds(0, STEP_BUTTON_HEIGHT, STEP_BUTTON_WIDTH,
                           KNOB_HEIGHT);
    noteTwoLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT,
                           STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    noteThreeLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                             STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    noteFourLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
                            STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    velocityLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 4,
                            STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    offsetLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 5,
                          STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    lengthLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 6,
                          STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    probabilityLabel.setBounds(0, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 7,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);

    for (int i = 0; i < STEP_SEQ_DEFAULT_LENGTH; ++i) {
      int x = (i + 1) * (STEP_BUTTON_WIDTH + STEP_BUTTON_SPACING);
      stepButtons[i].setBounds(x, 0, STEP_BUTTON_WIDTH, STEP_BUTTON_HEIGHT);
      noteOneKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT, STEP_BUTTON_WIDTH,
                                KNOB_HEIGHT);
      noteTwoKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT,
                                STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      noteThreeKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 2,
                                  STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      noteFourKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 3,
                                 STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      velocityKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 4,
                                 STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      offsetKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 5,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      lengthKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 6,
                               STEP_BUTTON_WIDTH, KNOB_HEIGHT);
      probabilityKnobs[i].setBounds(x, STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 7,
                                    STEP_BUTTON_WIDTH, KNOB_HEIGHT);
    }
  }

  void toggleCollapsed() {
    setCollapsed(!collapsed_);
    getParentComponent()->resized();
  }

private:
  AudioPluginAudioProcessor& processorRef;
  Sequencer::Track& trackRef;
  int trackIndex_;
  bool collapsed_;

  void setCollapsed(bool collapsed) {
    collapsed_ = collapsed;
    if (collapsed) {
      setSize(STEP_BUTTON_WIDTH * (STEP_SEQ_DEFAULT_LENGTH + 1) +
                  STEP_BUTTON_SPACING * STEP_SEQ_DEFAULT_LENGTH,
              STEP_BUTTON_HEIGHT);
      trackCollapseButton.setButtonText("Track " +
                                        juce::String(trackIndex_ + 1) + " ▶");
    } else {
      setSize(STEP_BUTTON_WIDTH * (STEP_SEQ_DEFAULT_LENGTH + 1) +
                  STEP_BUTTON_SPACING * STEP_SEQ_DEFAULT_LENGTH,
              STEP_BUTTON_HEIGHT + KNOB_HEIGHT * 8);
      trackCollapseButton.setButtonText("Track " +
                                        juce::String(trackIndex_ + 1) + " ▼");
    }
  }

  juce::TextButton trackCollapseButton;
  juce::Label noteOneLabel;
  juce::Label noteTwoLabel;
  juce::Label noteThreeLabel;
  juce::Label noteFourLabel;
  juce::Label velocityLabel;
  juce::Label offsetLabel;
  juce::Label lengthLabel;
  juce::Label probabilityLabel;

  juce::TextButton stepButtons[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider noteOneKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider noteTwoKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider noteThreeKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider noteFourKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider velocityKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider offsetKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider lengthKnobs[STEP_SEQ_DEFAULT_LENGTH];
  juce::Slider probabilityKnobs[STEP_SEQ_DEFAULT_LENGTH];

  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
  using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

  // Parameter attachments
  std::unique_ptr<ButtonAttachment> enableAttachments[STEP_SEQ_DEFAULT_LENGTH];
  std::unique_ptr<SliderAttachment> noteOneAttachments[STEP_SEQ_DEFAULT_LENGTH],
      noteTwoAttachments[STEP_SEQ_DEFAULT_LENGTH],
      noteThreeAttachments[STEP_SEQ_DEFAULT_LENGTH],
      noteFourAttachments[STEP_SEQ_DEFAULT_LENGTH],
      velocityAttachments[STEP_SEQ_DEFAULT_LENGTH],
      offsetAttachments[STEP_SEQ_DEFAULT_LENGTH],
      lengthAttachments[STEP_SEQ_DEFAULT_LENGTH],
      probabilityAttachments[STEP_SEQ_DEFAULT_LENGTH];
};

}  // namespace audio_plugin