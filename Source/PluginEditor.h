#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/StackPanel.h"
#include "ui/CharacterPanel.h"
#include "ui/ThrowPanel.h"
#include "ui/MeltPanel.h"
#include "ui/ClipBankView.h"
#include "ui/MiniKeyboard.h"
#include "ui/ChaosKnob.h"
#include "ui/HeaderBar.h"
class SkellAdlibLabEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit SkellAdlibLabEditor(SkellAdlibLabProcessor&);
 ~SkellAdlibLabEditor()override;
 void paint(juce::Graphics&)override;
 void resized()override;
private:
 class Canvas final:public juce::Component {
 public:
  Canvas(SkellAdlibLabProcessor&,SkellAdlibLabEditor&);
  void paint(juce::Graphics&)override;
  void reorder(int id,int displacement);
  void layoutModules();
  void refreshMeters();
 private:
  SkellAdlibLabProcessor&processor;
  SkellAdlibLabEditor&editor;
  HeaderBar header;
  StackPanel stack;
  CharacterPanel character;
  ThrowPanel throwPanel;
  MeltPanel melt;
  ChaosKnob chaos;
  ClipBankView bank;
  MiniKeyboard keyboard;
  juce::Slider width,gain;
  juce::ToggleButton limiter,formant,velIntensity,velTail,reverse;
  juce::AudioProcessorValueTreeState::SliderAttachment widthAtt,gainAtt;
  juce::AudioProcessorValueTreeState::ButtonAttachment limiterAtt,formantAtt,velIntensityAtt,velTailAtt,reverseAtt;
  std::array<ModulePanel*,4> modules;
 } canvas;
 void timerCallback()override;
 SkellAdlibLabProcessor&processor;
 LabLookAndFeel lookAndFeel;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellAdlibLabEditor)
};
