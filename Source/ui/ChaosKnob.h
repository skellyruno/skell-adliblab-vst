#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
class ChaosKnob final:public juce::Component {
public:
 explicit ChaosKnob(SkellAdlibLabProcessor&p):processor(p),attach(p.parameters,"chaos",dial),lockAttach(p.parameters,"chaosLock",lock){
  dial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  dial.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,20);
  dial.setColour(juce::Slider::rotarySliderFillColourId,Theme::chaos);
  dial.setColour(juce::Slider::textBoxTextColourId,Theme::text);
  dial.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  dial.setBounds(14,28,198,158);addAndMakeVisible(dial);
  lock.setButtonText("LOCK SEED");lock.setColour(juce::ToggleButton::textColourId,Theme::text);lock.setBounds(25,183,125,27);addAndMakeVisible(lock);
  roll.setButtonText("REROLL");roll.setBounds(128,185,85,26);roll.onClick=[&p]{p.reRoll();};addAndMakeVisible(roll);
 }
 void paint(juce::Graphics&g)override{
  g.setColour(Theme::chaos);g.setFont(Theme::display(20));g.drawText("CHAOS",23,0,190,32,juce::Justification::centred);
 }
private:SkellAdlibLabProcessor&processor;juce::Slider dial;juce::ToggleButton lock;juce::TextButton roll;
 juce::AudioProcessorValueTreeState::SliderAttachment attach;
 juce::AudioProcessorValueTreeState::ButtonAttachment lockAttach;
};
