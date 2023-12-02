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
class FinalProjectAudioProcessorEditor  :public juce::AudioProcessorEditor
{
public:
    FinalProjectAudioProcessorEditor (FinalProjectAudioProcessor&);
    ~FinalProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FinalProjectAudioProcessor& audioProcessor;

    Slider mDryWetSlider;
    Slider mDepthSlider;
    Slider mRateSlider;
    Slider mPhaseOffsetSlider;
    Slider mFeedbackSlider;

    Label mPluginName;
    Label mDryWetLabel;
    Label mDepthLabel;
    Label mRateLabel;
    Label mPhaseOffsetLabel;
    Label mFeedbackLabel;

    ComboBox mType;

    // Background:
    juce::ImageComponent background;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FinalProjectAudioProcessorEditor)
};
