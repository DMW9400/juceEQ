/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JuceEQAudioProcessorEditor::JuceEQAudioProcessorEditor (JuceEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Add and make visible each EQBand component
    for (int bandNum = 0; bandNum < 10; ++bandNum)
    {
        addAndMakeVisible(audioProcessor.eqBands[bandNum]);
    }
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 500);
}

JuceEQAudioProcessorEditor::~JuceEQAudioProcessorEditor()
{
}

//==============================================================================
void JuceEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));

}

void JuceEQAudioProcessorEditor::resized()
{
    // Define layout parameters
    int spacing = 60;               // Space between sliders
    int sliderWidth = 40;           // Width of each slider
    int sliderHeight = getHeight() - 60; // Height of sliders (accounting for margins)

    int startX = 20;                // Starting x position
    int startY = 40;                // Starting y position (adjusted for title label)

    for (int i = 0; i < 10; ++i)
    {
        audioProcessor.eqBands[i].setBounds(
            startX + i * spacing,    // x position
            startY,                  // y position
            sliderWidth,             // width
            sliderHeight             // height
        );
    }
}
