/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FROGGAudioProcessorEditor::FROGGAudioProcessorEditor (FROGGAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // Background set up:
    auto backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::FROGGBG_png, BinaryData::FROGGBG_pngSize);
    background.setImage(backgroundImage);
    background.setImagePlacement(juce::RectanglePlacement::centred);
    addAndMakeVisible(background);

    // Array of parameters:
    auto& params = processor.getParameters();

    // Dry / Wet Slider set up:
    AudioParameterFloat* dryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    mDryWetSlider.setBounds(110, 30, 60, 60);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    mDryWetSlider.onValueChange = [this, dryWetParameter] {*dryWetParameter = mDryWetSlider.getValue(); };
    mDryWetSlider.onDragStart = [dryWetParameter] { dryWetParameter->beginChangeGesture(); };
    mDryWetSlider.onDragEnd = [dryWetParameter] { dryWetParameter->endChangeGesture(); };
    addAndMakeVisible(mDryWetSlider);

    // Depth Slider set up:
    AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);
    mDepthSlider.setBounds(227, 30, 60, 60);
    mDepthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDepthSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDepthSlider.setRange(depthParameter->range.start, depthParameter->range.end);
    mDepthSlider.setValue(*depthParameter);
    mDepthSlider.onValueChange = [this, depthParameter] {*depthParameter = mDepthSlider.getValue(); };
    mDepthSlider.onDragStart = [depthParameter] { depthParameter->beginChangeGesture(); };
    mDepthSlider.onDragEnd = [depthParameter] { depthParameter->endChangeGesture(); };
    addAndMakeVisible(mDepthSlider);

    // Rate Slider set up:
    AudioParameterFloat* rateParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);
    mRateSlider.setBounds(103, 115, 60, 60);
    mRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mRateSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mRateSlider.setRange(rateParameter->range.start, rateParameter->range.end);
    mRateSlider.setValue(*rateParameter);
    mRateSlider.onValueChange = [this, rateParameter] {*rateParameter = mRateSlider.getValue(); };
    mRateSlider.onDragStart = [rateParameter] { rateParameter->beginChangeGesture(); };
    mRateSlider.onDragEnd = [rateParameter] { rateParameter->endChangeGesture(); };
    addAndMakeVisible(mRateSlider);

    // Phase Offset Slider set up:
    AudioParameterFloat* phaseParameter = (juce::AudioParameterFloat*)params.getUnchecked(3);
    mPhaseOffsetSlider.setBounds(173.5f, 113, 50, 50);
    mPhaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mPhaseOffsetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mPhaseOffsetSlider.setRange(phaseParameter->range.start, phaseParameter->range.end);
    mPhaseOffsetSlider.setValue(*phaseParameter);
    mPhaseOffsetSlider.onValueChange = [this, phaseParameter] {*phaseParameter = mPhaseOffsetSlider.getValue(); };
    mPhaseOffsetSlider.onDragStart = [phaseParameter] { phaseParameter->beginChangeGesture(); };
    mPhaseOffsetSlider.onDragEnd = [phaseParameter] { phaseParameter->endChangeGesture(); };
    addAndMakeVisible(mPhaseOffsetSlider);

    // Feedback Slider set up:
    AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);
    mFeedbackSlider.setBounds(236, 115, 60, 60);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    mFeedbackSlider.onValueChange = [this, feedbackParameter] {*feedbackParameter = mFeedbackSlider.getValue(); };
    mFeedbackSlider.onDragStart = [feedbackParameter] { feedbackParameter->beginChangeGesture(); };
    mFeedbackSlider.onDragEnd = [feedbackParameter] { feedbackParameter->endChangeGesture(); };
    addAndMakeVisible(mFeedbackSlider);

    // Type ComboBox set up:
    AudioParameterInt* typeParameter = (juce::AudioParameterInt*)params.getUnchecked(5);
    mType.setBounds(15, 23, 80, 20);
    mType.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    mType.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    mType.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff401C04));
    mType.setColour(juce::ComboBox::textColourId, juce::Colour(0xff401C04));
    mType.addItem("Chorus", 1);
    mType.addItem("Flanger", 2);
    addAndMakeVisible(mType);

    mType.onChange = [this, typeParameter] 
    {
        typeParameter->beginChangeGesture();
        *typeParameter = mType.getSelectedItemIndex();
        typeParameter->endChangeGesture();
    };

    mType.setSelectedItemIndex(*typeParameter);

    // Slider colors:
    mDryWetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::ghostwhite);
    mDryWetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    mDryWetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::darkslategrey);

    mDepthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::ghostwhite);
    mDepthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    mDepthSlider.setColour(juce::Slider::thumbColourId, juce::Colours::darkslategrey);

    mRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::mediumpurple);
    mRateSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::rebeccapurple);
    mRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::mediumseagreen);

    mPhaseOffsetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::mediumpurple);
    mPhaseOffsetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::rebeccapurple);
    mPhaseOffsetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::darkseagreen);

    mFeedbackSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::mediumpurple);
    mFeedbackSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::rebeccapurple);
    mFeedbackSlider.setColour(juce::Slider::thumbColourId, juce::Colours::lightseagreen);

    setSize(400, 300);
}

FROGGAudioProcessorEditor::~FROGGAudioProcessorEditor()
{

}

//==============================================================================
void FROGGAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
 /*   g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);*/
}

void FROGGAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    background.setBounds(getLocalBounds());
}
