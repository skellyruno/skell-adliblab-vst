#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
class ChaosKnob final:public juce::Component {
public:
 explicit ChaosKnob(SkellAdlibLabProcessor&p,const Theme::Palette&pal):palette(pal),attach(p.parameters,"chaos",dial),lockAttach(p.parameters,"chaosLock",lock){
  dial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  dial.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,22);
  dial.setColour(juce::Slider::rotarySliderFillColourId,palette.chaos);
  dial.setColour(juce::Slider::textBoxTextColourId,palette.text);
  dial.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  dial.textFromValueFunction=[](double v){return juce::String((int)std::round(v*100.))+"%";};
  dial.setBounds(30,25,170,158);addAndMakeVisible(dial);
  lock.setButtonText("LOCK");lock.setBounds(12,184,100,29);addAndMakeVisible(lock);
  roll.setButtonText("RE-ROLL");roll.setBounds(127,184,94,29);roll.onClick=[&p]{p.reRoll();};addAndMakeVisible(roll);
  refreshTheme();
 }
 void refreshTheme(){dial.setColour(juce::Slider::rotarySliderFillColourId,palette.chaos);dial.setColour(juce::Slider::textBoxTextColourId,palette.text);lock.setColour(juce::ToggleButton::textColourId,palette.text);lock.setColour(juce::ToggleButton::tickColourId,palette.chaos);repaint();}
 void paint(juce::Graphics&g)override{
  g.setColour(palette.surface);g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(1),10);
  g.setColour(palette.border);g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1),10,1.f);
  g.setColour(palette.chaos);g.setFont(Theme::display(16));g.drawText("CHAOS",17,5,116,23,juce::Justification::left);
  g.setColour(palette.muted);g.setFont(Theme::mono(10));g.drawText("HIT MUTATOR",118,7,103,19,juce::Justification::right);
 }
private:
 const Theme::Palette&palette;
 juce::Slider dial;juce::ToggleButton lock;juce::TextButton roll;
 juce::AudioProcessorValueTreeState::SliderAttachment attach;
 juce::AudioProcessorValueTreeState::ButtonAttachment lockAttach;
};
