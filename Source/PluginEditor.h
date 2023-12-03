/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FROGGAudioProcessorEditor  :public juce::AudioProcessorEditor
{
public:
    FROGGAudioProcessorEditor (FROGGAudioProcessor&);
    ~FROGGAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FROGGAudioProcessor& audioProcessor;

    // Sliders:
    Slider mDryWetSlider;
    Slider mDepthSlider;
    Slider mRateSlider;
    Slider mPhaseOffsetSlider;
    Slider mFeedbackSlider;

    // Labels / Text:
    Label mPluginName;
    Label mDryWetLabel;
    Label mDepthLabel;
    Label mRateLabel;
    Label mPhaseOffsetLabel;
    Label mFeedbackLabel;

    // Combobox for Flanger / Chorus:
    ComboBox mType;

    // Background:
    juce::ImageComponent background;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FROGGAudioProcessorEditor)
};
