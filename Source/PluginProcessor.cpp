/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#define _USE_MATH_DEFINES
#include <cmath> 

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FinalProjectAudioProcessor::FinalProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("drywet",
        "Dry Wet",
        0.0,
        1.0,
        0.5));

    addParameter(mDepthParameter = new juce::AudioParameterFloat("depth",
        "Depth",
        0.0,
        1.0,
        0.5));

    addParameter(mRateParameter = new juce::AudioParameterFloat("rate",
        "Rate",
        0.1f,
        20.f,
        10.f));

    addParameter(mPhaseOffsetParameter = new juce::AudioParameterFloat("phaseoffset",
        "Phase Offset",
        0.0f,
        1.1f,
        0.5f));

    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback",
        "Feedback",
        0,
        0.98,
        0.5));

    addParameter(mTypeParameter = new juce::AudioParameterInt("type",
        "Type",
        0,
        1,
        1));

    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLenght = 0;

    mFeedbackLeft = 0;
    mFeedbackRight = 0;

    mLFOPhase = 0;


}

FinalProjectAudioProcessor::~FinalProjectAudioProcessor()
{
}

//==============================================================================
const juce::String FinalProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FinalProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FinalProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FinalProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FinalProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FinalProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FinalProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FinalProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FinalProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void FinalProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FinalProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    mDelayTimeSmoothed = 1;

    mLFOPhase = 0;

    mCircularBufferLenght = sampleRate * MAX_DELAY_TIME;

    if (mCircularBufferLeft != nullptr) {
        delete[] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }

    if (mCircularBufferRight != nullptr) {
        delete[] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }

    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[mCircularBufferLenght];
    }

    zeromem(mCircularBufferLeft, mCircularBufferLenght * sizeof(float));


    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[mCircularBufferLenght];
    }

    zeromem(mCircularBufferRight, mCircularBufferLenght * sizeof(float));

    mCircularBufferWriteHead = 0;

}

void FinalProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FinalProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FinalProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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


    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); i++) {

        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;

        float lfoOutLeft = sin(2 * M_PI * mLFOPhase);

        float lfoPhaseRight = mLFOPhase + *mPhaseOffsetParameter;

        if (lfoPhaseRight > 1) {
            lfoPhaseRight -= 1;
        }

        float lfoOutRight = sin(2 * M_PI * lfoPhaseRight);

        mLFOPhase += *mRateParameter / getSampleRate();

        if (mLFOPhase > 1) {
            mLFOPhase -= 1;
        }


        lfoOutLeft *= *mDepthParameter;
        lfoOutRight *= *mDepthParameter;

        float lfoOutMappedLeft = 0;
        float lfoOutMappedRight = 0;

        if (*mTypeParameter == 0) {
            float lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
            float lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
        }
        else {
            lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
            lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
        }

        float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
        float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;

        float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;

        if (delayReadHeadLeft < 0) {
            delayReadHeadLeft += mCircularBufferLenght;
        }

        float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;

        if (delayReadHeadRight < 0) {
            delayReadHeadRight += mCircularBufferLenght;
        }

        int readHeadLeft_x = (int)delayReadHeadLeft;
        int readHeadLeft_x1 = readHeadLeft_x + 1;
        float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;

        if (readHeadLeft_x1 >= mCircularBufferLenght) {
            readHeadLeft_x1 -= mCircularBufferLenght;
        }

        int readHeadRight_x = (int)delayReadHeadRight;
        int readHeadRight_x1 = readHeadRight_x + 1;
        float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;

        if (readHeadRight_x1 >= mCircularBufferLenght) {
            readHeadRight_x1 -= mCircularBufferLenght;
        }

        float delay_sample_left = lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);

        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;

        mCircularBufferWriteHead++;

        if (mCircularBufferWriteHead >= mCircularBufferLenght) {
            mCircularBufferWriteHead = 0;
        }

        float dryAmount = 1 - *mDryWetParameter;
        float wetAmout = *mDryWetParameter;

        buffer.setSample(0, i, buffer.getSample(0, i) * dryAmount + delay_sample_left * wetAmout);
        buffer.setSample(1, i, buffer.getSample(1, i) * dryAmount + delay_sample_right * wetAmout);
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool FinalProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FinalProjectAudioProcessor::createEditor()
{
    return new FinalProjectAudioProcessorEditor (*this);
}

//==============================================================================
void FinalProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void FinalProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FinalProjectAudioProcessor();
}

float FinalProjectAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}
