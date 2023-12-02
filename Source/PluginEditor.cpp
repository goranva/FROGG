/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FinalProjectAudioProcessorEditor::FinalProjectAudioProcessorEditor (FinalProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    auto backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::FlangerFrogBG_png, BinaryData::FlangerFrogBG_pngSize);
    background.setImage(backgroundImage);
    background.setImagePlacement(juce::RectanglePlacement::centred);
    addAndMakeVisible(background);


    auto& params = processor.getParameters();

    AudioParameterFloat* dryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);

    mDryWetSlider.setBounds(110, 30, 60, 60);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    mDryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    mDryWetLabel.attachToComponent(&mDryWetSlider, false);
    mDryWetLabel.setColour(juce::Label::textColourId, juce::Colours::purple);
    addAndMakeVisible(mDryWetSlider);

    mDryWetSlider.onValueChange = [this, dryWetParameter] {*dryWetParameter = mDryWetSlider.getValue(); };
    mDryWetSlider.onDragStart = [dryWetParameter] { dryWetParameter->beginChangeGesture(); };
    mDryWetSlider.onDragEnd = [dryWetParameter] { dryWetParameter->endChangeGesture(); };

    AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);

    mDepthSlider.setBounds(225, 30, 60, 60);
    mDepthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDepthSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDepthSlider.setRange(depthParameter->range.start, depthParameter->range.end);
    mDepthSlider.setValue(*depthParameter);
    addAndMakeVisible(mDepthSlider);
    mDepthLabel.setText("Depth", juce::dontSendNotification);
    mDepthLabel.attachToComponent(&mDepthSlider, false);
    mDepthLabel.setColour(juce::Label::textColourId, juce::Colours::purple);

    mDepthSlider.onValueChange = [this, depthParameter] {*depthParameter = mDepthSlider.getValue(); };
    mDepthSlider.onDragStart = [depthParameter] { depthParameter->beginChangeGesture(); };
    mDepthSlider.onDragEnd = [depthParameter] { depthParameter->endChangeGesture(); };

    AudioParameterFloat* rateParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);

    mRateSlider.setBounds(110, 120, 60, 60);
    mRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mRateSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mRateSlider.setRange(rateParameter->range.start, rateParameter->range.end);
    mRateSlider.setValue(*rateParameter);
    addAndMakeVisible(mRateSlider);
    mRateLabel.setText("Rate", juce::dontSendNotification);
    mRateLabel.attachToComponent(&mRateSlider, false);
    mRateLabel.setColour(juce::Label::textColourId, juce::Colours::purple);



    mRateSlider.onValueChange = [this, rateParameter] {*rateParameter = mRateSlider.getValue(); };
    mRateSlider.onDragStart = [rateParameter] { rateParameter->beginChangeGesture(); };
    mRateSlider.onDragEnd = [rateParameter] { rateParameter->endChangeGesture(); };

    AudioParameterFloat* phaseParameter = (juce::AudioParameterFloat*)params.getUnchecked(3);

    mPhaseOffsetSlider.setBounds(167, 105, 60, 60);
    mPhaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mPhaseOffsetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mPhaseOffsetSlider.setRange(phaseParameter->range.start, phaseParameter->range.end);
    mPhaseOffsetSlider.setValue(*phaseParameter);
    addAndMakeVisible(mPhaseOffsetSlider);
    mPhaseOffsetLabel.setText("Phase Offset", juce::dontSendNotification);
    mPhaseOffsetLabel.attachToComponent(&mPhaseOffsetSlider, false);
    mPhaseOffsetLabel.setColour(juce::Label::textColourId, juce::Colours::purple);


    mPhaseOffsetSlider.onValueChange = [this, phaseParameter] {*phaseParameter = mPhaseOffsetSlider.getValue(); };
    mPhaseOffsetSlider.onDragStart = [phaseParameter] { phaseParameter->beginChangeGesture(); };
    mPhaseOffsetSlider.onDragEnd = [phaseParameter] { phaseParameter->endChangeGesture(); };

    AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);
    mFeedbackSlider.setBounds(230, 120, 60, 60);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);
    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    mFeedbackLabel.attachToComponent(&mFeedbackSlider, false);
    mFeedbackLabel.setColour(juce::Label::textColourId, juce::Colours::purple);


    mFeedbackSlider.onValueChange = [this, feedbackParameter] {*feedbackParameter = mFeedbackSlider.getValue(); };
    mFeedbackSlider.onDragStart = [feedbackParameter] { feedbackParameter->beginChangeGesture(); };
    mFeedbackSlider.onDragEnd = [feedbackParameter] { feedbackParameter->endChangeGesture(); };

    AudioParameterInt* typeParameter = (juce::AudioParameterInt*)params.getUnchecked(5);

    mType.setBounds(2.5f, 30, 100, 30);
    mType.addItem("Chorus", 1);
    mType.addItem("Flanger", 2);
    addAndMakeVisible(mType);

    mType.onChange = [this, typeParameter] {
        typeParameter->beginChangeGesture();
        *typeParameter = mType.getSelectedItemIndex();
        typeParameter->endChangeGesture();
        };

    mType.setSelectedItemIndex(*typeParameter);

    mPluginName.setText("FlangerFrog", juce::dontSendNotification);
    mPluginName.attachToComponent(&mType, false);
    mPluginName.setColour(juce::Label::textColourId, juce::Colours::purple);
    mPluginName.setFont(juce::Font(25.0f));

    mDryWetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightseagreen);
    mDryWetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgreen);
    mDryWetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);    

    mDepthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightseagreen);
    mDepthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgreen);
    mDepthSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);

    mRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::hotpink);
    mRateSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    mRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);

    mPhaseOffsetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::hotpink);
    mPhaseOffsetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    mPhaseOffsetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);

    mFeedbackSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::hotpink);
    mFeedbackSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    mFeedbackSlider.setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);

    setSize(400, 300);
}

FinalProjectAudioProcessorEditor::~FinalProjectAudioProcessorEditor()
{

}

//==============================================================================
void FinalProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
 /*   g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);*/
}

void FinalProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    background.setBounds(getLocalBounds());

    mDryWetLabel.setBounds(110, 60, 60, 60);
    mDryWetLabel.setFont(20.0f);

    mDepthLabel.setBounds(225, 60, 60, 60);
    mDepthLabel.setFont(20.0f);

    mRateLabel.setBounds(110, 160, 60, 60);
    mRateLabel.setFont(20.0f);

    mPhaseOffsetLabel.setBounds(167, 160, 60, 60);
    mPhaseOffsetLabel.setFont(20.0f);

    mFeedbackLabel.setBounds(230, 160, 60, 60);
    mFeedbackLabel.setFont(20.0f);

}
