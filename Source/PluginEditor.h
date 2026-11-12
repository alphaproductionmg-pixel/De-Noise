#pragma once

#include "PluginProcessor.h"

class DeNoiseAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DeNoiseAudioProcessorEditor (DeNoiseAudioProcessor&);
    ~DeNoiseAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    DeNoiseAudioProcessor& processor;

    juce::Label title, presetLabel, mixLabel;
    juce::ComboBox presetBox;
    juce::Slider mixSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeNoiseAudioProcessorEditor)
};
