#include "PluginEditor.h"

DeNoiseAudioProcessorEditor::DeNoiseAudioProcessorEditor (DeNoiseAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    title.setText ("De-Noise", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (26.0f, juce::Font::bold)));
    title.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (title);

    presetLabel.setText ("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (presetLabel);

    presetBox.addItemList (juce::StringArray { "Bas", "Normal", "Haut" }, 1);
    addAndMakeVisible (presetBox);

    mixLabel.setText ("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (mixLabel);

    mixSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    mixSlider.setTextValueSuffix (" %");
    addAndMakeVisible (mixSlider);

    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, "preset", presetBox);
    mixAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "mix", mixSlider);

    setSize (340, 190);
}

void DeNoiseAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1b1f27));
}

void DeNoiseAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);

    title.setBounds (area.removeFromTop (40));
    area.removeFromTop (10);

    auto row1 = area.removeFromTop (30);
    presetLabel.setBounds (row1.removeFromLeft (70));
    presetBox.setBounds (row1);

    area.removeFromTop (14);

    auto row2 = area.removeFromTop (30);
    mixLabel.setBounds (row2.removeFromLeft (70));
    mixSlider.setBounds (row2);
}
