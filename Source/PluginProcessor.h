#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>
#include "NoiseReducer.h"

class DeNoiseAudioProcessor : public juce::AudioProcessor
{
public:
    DeNoiseAudioProcessor();
    ~DeNoiseAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "De-Noise"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct PresetValues
    {
        float overSubtraction;
        float floorDb;
    };

    // Préréglages : Bas / Normal / Haut
    static constexpr PresetValues presets[3] = {
        { 1.0f,  -9.0f },   // Bas    : réduction légère, peu d'artefacts
        { 1.6f, -15.0f },   // Normal : compromis
        { 2.4f, -24.0f }    // Haut   : réduction forte
    };

    std::vector<std::unique_ptr<NoiseReducer>> reducers;

    std::atomic<float>* presetParam = nullptr;
    std::atomic<float>* mixParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeNoiseAudioProcessor)
};
