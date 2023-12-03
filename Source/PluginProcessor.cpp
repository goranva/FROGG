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
FROGGAudioProcessor::FROGGAudioProcessor()
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

    // Parameters initialization
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

    // Circular buffer and related variables initialization
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLenght = 0;

    // Feedback variables initialization
    mFeedbackLeft = 0;
    mFeedbackRight = 0;

    // LFO phase initialization
    mLFOPhase = 0;
}

FROGGAudioProcessor::~FROGGAudioProcessor()
{
}

//==============================================================================
const juce::String FROGGAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FROGGAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FROGGAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FROGGAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FROGGAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FROGGAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FROGGAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FROGGAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FROGGAudioProcessor::getProgramName (int index)
{
    return {};
}

void FROGGAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FROGGAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    mDelayTimeSmoothed = 1;

    // Reset Phase:
    mLFOPhase = 0;

    // Circular buffer length calculation based on maximum delay time (2):
    mCircularBufferLenght = sampleRate * MAX_DELAY_TIME;

    // Release existing memory for circular buffers if any:
    if (mCircularBufferLeft != nullptr) {
        delete[] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }

    if (mCircularBufferRight != nullptr) {
        delete[] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }

    // Allocate memory for circular buffers:
    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[mCircularBufferLenght];
    }

    // Initialize circular buffer with zeros:
    zeromem(mCircularBufferLeft, mCircularBufferLenght * sizeof(float));


    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[mCircularBufferLenght];
    }

    zeromem(mCircularBufferRight, mCircularBufferLenght * sizeof(float));
    
    // Circular buffer write head reset:
    mCircularBufferWriteHead = 0;
}

void FROGGAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FROGGAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FROGGAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // ScopedNoDenormals ensures that denormalized numbers won't cause performance issues
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get pointers to the left and right channels of the audio buffer
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    // Process each sample in the buffer
    for (int i = 0; i < buffer.getNumSamples(); i++) {

        // Circular buffer write operation with feedback
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;

        // LFO phase calculation and normalization
        float lfoOutLeft = sin(2 * M_PI * mLFOPhase);
        float lfoPhaseRight = mLFOPhase + *mPhaseOffsetParameter;

        if (lfoPhaseRight > 1) {
            lfoPhaseRight -= 1;
        }

        float lfoOutRight = sin(2 * M_PI * lfoPhaseRight);

        // Update LFO phase
        mLFOPhase += *mRateParameter / getSampleRate();

        if (mLFOPhase > 1) {
            mLFOPhase -= 1;
        }

        // Apply depth parameter to LFO output
        lfoOutLeft *= *mDepthParameter;
        lfoOutRight *= *mDepthParameter;

        // Map LFO output to a specified range based on the type parameter
        float lfoOutMappedLeft = 0;
        float lfoOutMappedRight = 0;

        if (*mTypeParameter == 0) {
            // Chorus:
            lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
            lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
        }
        else {
            // Flanger:
            lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
            lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
        }

        // Calculate delay time in samples based on LFO output
        float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
        float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;

        // Calculate read heads for left and right channels
        float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
        float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;

        // Make sure they never get to less than 0:
        if (delayReadHeadLeft < 0) {
            delayReadHeadLeft += mCircularBufferLenght;
        }

        if (delayReadHeadRight < 0) {
            delayReadHeadRight += mCircularBufferLenght;
        }

        // Convert read head positions to integer and fractional parts
        int readHeadLeft_x = (int)delayReadHeadLeft;
        int readHeadLeft_x1 = readHeadLeft_x + 1;
        float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;

        int readHeadRight_x = (int)delayReadHeadRight;
        int readHeadRight_x1 = readHeadRight_x + 1;
        float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;

        if (readHeadLeft_x1 >= mCircularBufferLenght) {
            readHeadLeft_x1 -= mCircularBufferLenght;
        }

        if (readHeadRight_x1 >= mCircularBufferLenght) {
            readHeadRight_x1 -= mCircularBufferLenght;
        }

        // Perform linear interpolation to get delayed samples
        float delay_sample_left = lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);

        // Apply feedback to the delayed samples
        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;

        // Increment circular buffer write head with wrap-around
        mCircularBufferWriteHead++;

        if (mCircularBufferWriteHead >= mCircularBufferLenght) {
            mCircularBufferWriteHead = 0;
        }

        // Calculate dry and wet amounts based on the dry/wet parameter
        float dryAmount = 1 - *mDryWetParameter;
        float wetAmount = *mDryWetParameter;

        // Apply the delay effect to the output buffer
        buffer.setSample(0, i, buffer.getSample(0, i) * dryAmount + delay_sample_left * wetAmount);
        buffer.setSample(1, i, buffer.getSample(1, i) * dryAmount + delay_sample_right * wetAmount);
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool FROGGAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FROGGAudioProcessor::createEditor()
{
    return new FROGGAudioProcessorEditor (*this);
}

//==============================================================================
void FROGGAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<XmlElement> xml(new XmlElement("FlangerChorus"));
    xml->setAttribute("DryWet", *mDryWetParameter);
    xml->setAttribute("Depth", *mDepthParameter);
    xml->setAttribute("Rate", *mRateParameter);
    xml->setAttribute("PhaseOffset", *mPhaseOffsetParameter);
    xml->setAttribute("Feedback", *mFeedbackParameter);
    xml->setAttribute("Type", *mTypeParameter);
    copyXmlToBinary(*xml, destData);
}

void FROGGAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr && xml->hasTagName("FlangerChorus")) {
        *mDryWetParameter = xml->getDoubleAttribute("DryWet");
        *mDepthParameter = xml->getDoubleAttribute("Depth");
        *mRateParameter = xml->getDoubleAttribute("Rate");
        *mPhaseOffsetParameter = xml->getDoubleAttribute("PhaseOffset");
        *mFeedbackParameter = xml->getDoubleAttribute("Feedback");
        *mTypeParameter = xml->getIntAttribute("Type");
    }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FROGGAudioProcessor();
}

float FROGGAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}
