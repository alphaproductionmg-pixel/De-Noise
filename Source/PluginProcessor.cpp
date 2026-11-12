#include "PluginProcessor.h"
#include "PluginEditor.h"

DeNoiseAudioProcessor::DeNoiseAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    presetParam = apvts.getRawParameterValue ("preset");
    mixParam    = apvts.getRawParameterValue ("mix");
}

juce::AudioProcessorValueTreeState::ParameterLayout DeNoiseAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "preset", 1 }, "Preset",
        juce::StringArray { "Bas", "Normal", "Haut" }, 1));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    return layout;
}

bool DeNoiseAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void DeNoiseAudioProcessor::prepareToPlay (double sampleRate, int)
{
    const int numChannels = std::max (getTotalNumInputChannels(), 1);

    reducers.clear();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto r = std::make_unique<NoiseReducer>();
        r->prepare (sampleRate);
        reducers.push_back (std::move (r));
    }

    setLatencySamples (NoiseReducer::fftSize);
}

void DeNoiseAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = std::min (buffer.getNumChannels(), (int) reducers.size());
    const int numSamples = buffer.getNumSamples();

    const int presetIndex = juce::jlimit (0, 2, (int) std::lround (presetParam->load()));
    const float mix = mixParam->load() / 100.0f;
    const auto& p = presets[presetIndex];

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto& reducer = *reducers[(size_t) ch];
        reducer.setParameters (p.overSubtraction, p.floorDb);

        float* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float wet = 0.0f, dry = 0.0f;
            reducer.processSample (data[i], wet, dry);
            data[i] = dry + mix * (wet - dry);
        }
    }

    for (int ch = numChannels; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

juce::AudioProcessorEditor* DeNoiseAudioProcessor::createEditor()
{
    return new DeNoiseAudioProcessorEditor (*this);
}

void DeNoiseAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void DeNoiseAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DeNoiseAudioProcessor();
}
