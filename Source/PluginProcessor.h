/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define MAX_DELAY_TIME 2

//==============================================================================
/**
*/
class FROGGAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FROGGAudioProcessor();
    ~FROGGAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    float lin_interp(float sample_x, float sample_x1, float inPhase);

private:

    float mDelayTimeSmoothed;

    // Flanger / Chorus Parameters:

    // Parameter to control mix between dry signal and wet:
    AudioParameterFloat* mDryWetParameter;

    // Parameter to control the depth of the modulator, how much delay is applied to the signal:
    AudioParameterFloat* mDepthParameter;

    // Parameter to control the rate of the LFO modulator, how quick the delay is applied to the signal over time:
    AudioParameterFloat* mRateParameter;

    // Parameter to control the phase:
    AudioParameterFloat* mPhaseOffsetParameter;

    // Parameter to control the amount of feedback applied to the delayed signal:
    AudioParameterFloat* mFeedbackParameter;

    // Parameter to control the  type of modulation effect: Chorus or Flanger:
    AudioParameterInt* mTypeParameter;

    // Circular buffer for delay of each channel:
    float* mCircularBufferLeft;
    float* mCircularBufferRight;

    int mCircularBufferWriteHead;
    int mCircularBufferLenght;

    // Feedback to send to input:
    float mFeedbackLeft;
    float mFeedbackRight;

    float mLFOPhase;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FROGGAudioProcessor)
};
