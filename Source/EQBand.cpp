/*
  ==============================================================================

    EQBand.cpp
    Created: 31 Oct 2024 9:42:40pm
    Author:  David Matthew Welch

  ==============================================================================
*/

#include "EQBand.h"
#include <JuceHeader.h>

EQBand::EQBand()
    : numChannels(0), sampleRate(44100), frequency(1000.0f), gain(1.0f)
{
    addAndMakeVisible(bandSlider);
    
    bandSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    
    bandSlider.setRange(.01, 2.0, .01);
    bandSlider.setValue(1.0);
    
    bandSlider.addListener(this);
    
    gain = 1.0f;
    
    addAndMakeVisible(bandLabel);
    bandLabel.setText("Gain", juce::dontSendNotification);
    bandLabel.attachToComponent(&bandSlider, false);
    
}

EQBand::~EQBand()
{
    // Destructor remains empty as smart pointers manage resources
}

void EQBand::resized()
{
    bandSlider.setBounds(getLocalBounds());
}

void EQBand::initializeVTS(juce::AudioProcessorValueTreeState& vts,  const juce::String& paramID){
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, paramID, bandSlider);
}

void EQBand::prepare (float frequency, int sampleRate, float gain, size_t numChannels){
//    set the arguments to each band's member variables
    this->frequency = frequency;
    this->sampleRate = sampleRate;
    this->gain = gain;
    this->numChannels = numChannels;
//    set the coefficients for each band's IIR filter
    auto coefficients = juce::IIRCoefficients::makePeakFilter(sampleRate, frequency, 1.0f, gain);
//    ensure we our number of filters matches the audio channels in the event of stereo/higher multichannel
    filters.resize(numChannels);
//    set coefficients for each filter within the vector
    for(auto& f : filters){
        f.setCoefficients(coefficients);
    }
}

void EQBand::process(juce::dsp::AudioBlock<float>& block)
{
    size_t currentNumChannels = block.getNumChannels();
    
    if (filters.size() != currentNumChannels){
        numChannels = currentNumChannels;
//        filter.resize(numChannels);
        prepare(frequency, sampleRate, gain, numChannels);
    }
        for (size_t channel = 0; channel < currentNumChannels; ++channel)
        {
            auto* samples = block.getChannelPointer(channel);
            filters[channel].processSamples(samples, static_cast<int>(block.getNumSamples()));
        }

}

void EQBand::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &bandSlider)
    {
        float newGain = static_cast<float>(bandSlider.getValue());
        
        // Update filter coefficients without reinitializing the entire filter setup
        prepare(frequency, sampleRate, newGain, numChannels);
//        setGain(newGain);
    }
}

void EQBand::setGain(float newGain)
{
    gain = newGain;
    
    // Update filter coefficients with the new gain
    auto coefficients = juce::IIRCoefficients::makePeakFilter(sampleRate, frequency, 1.0f, gain);
    
    for (auto& f : filters)
    {
        f.setCoefficients(coefficients);
    }
}

void EQBand::reset()
{
    for (auto& f : filters)
    {
        f.reset();
    }
}

// Added paint method definition
void EQBand::paint(juce::Graphics& g)
{
    // Fill the background with a light grey color
    g.fillAll(juce::Colours::lightgrey);
    
    // Optional: Add any custom painting here (e.g., border, indicators)
}
