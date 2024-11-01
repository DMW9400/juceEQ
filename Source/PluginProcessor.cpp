/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout JuceEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Initialize center frequencies
    centerFrequencies = { 32.0f, 63.0f, 125.0f, 250.0f, 500.0f,
                          1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };

    for (int i = 0; i < 10; ++i)
    {
        juce::String paramID = "GainParam_" + juce::String(i);
        juce::String paramName = juce::String(centerFrequencies[i]) + "Hz Gain";

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            paramID,                                    // Parameter ID
            paramName,                                  // Parameter name
            juce::NormalisableRange<float>(0.1f, 2.0f), // Range
            1.0f                                        // Default value
        ));
    }

    return layout;
}

JuceEQAudioProcessor::JuceEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    numChannels = getTotalNumOutputChannels();
    
    for (int i = 0; i < 10; ++i)
    {
        juce::String paramID = "GainParam_" + juce::String(i);
        float currentGain = parameters.getRawParameterValue(paramID)->load();

        // Initialize each EQBand with its center frequency
        eqBands[i].prepare(centerFrequencies[i], 44100.0, currentGain, numChannels); // sampleRate will be updated in prepareToPlay

        // Attach each EQBand's slider to its corresponding parameter
        eqBands[i].initializeVTS(parameters, paramID);
    }
}

JuceEQAudioProcessor::~JuceEQAudioProcessor()
{
}

//==============================================================================
const juce::String JuceEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JuceEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JuceEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JuceEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JuceEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JuceEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JuceEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JuceEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String JuceEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void JuceEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void JuceEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    numChannels = getTotalNumOutputChannels();

        for (int i = 0; i < 10; ++i)
        {
            juce::String paramID = "GainParam_" + juce::String(i);
            float currentGain = parameters.getRawParameterValue(paramID)->load();

            // Update EQBand with new sample rate and current gain
            eqBands[i].prepare(centerFrequencies[i], sampleRate, currentGain, numChannels);
        }
}

void JuceEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JuceEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void JuceEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (int i = 0; i < 10; ++i)
    {
        juce::String paramID = "GainParam_" + juce::String(i);
        float gainValue = parameters.getRawParameterValue(paramID)->load();
        eqBands[i].setGain(gainValue);
    }
    // Create an AudioBlock from the buffer
    juce::dsp::AudioBlock<float> audioBlock(buffer);

    // Process each EQBand
    for (int i = 0; i < 10; ++i)
    {
        eqBands[i].process(audioBlock);
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
//    for (int channel = 0; channel < totalNumInputChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer (channel);
//
//        // ..do something to the data...
//    }
}

//==============================================================================
bool JuceEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JuceEQAudioProcessor::createEditor()
{
    return new JuceEQAudioProcessorEditor (*this);
}

//==============================================================================
void JuceEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the parameters state
    if (auto xmlState = parameters.copyState().createXml())
    {
        copyXmlToBinary(*xmlState, destData);
    }
}

void JuceEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the parameters state
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JuceEQAudioProcessor();
}
