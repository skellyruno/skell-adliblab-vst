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
#include "ui/PresetMenuView.h"
#include <SkellCore/GuidedTour.h>
class SkellAdlibLabEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit SkellAdlibLabEditor(SkellAdlibLabProcessor&);
 ~SkellAdlibLabEditor()override;
 void paint(juce::Graphics&)override;
 void resized()override;
 void applyLookAndFeelPalette(const Theme::Palette&);
 void setPresetMenuForScreenshot(bool);
 juce::StringArray runUIInteractionTests();
 bool showGuidedTourForScreenshot();
 void dismissGuidedTourForScreenshot();
private:
 class Canvas final:public juce::Component,private juce::Slider::Listener {
 public:
  Canvas(SkellAdlibLabProcessor&,SkellAdlibLabEditor&);
  ~Canvas()override{width.removeListener(this);gain.removeListener(this);}
  void paint(juce::Graphics&)override;
  void reorder(int id,int displacement);
  void layoutModules();
  void refreshMeters(bool clipsChanged);
  void sliderValueChanged(juce::Slider*)override;
  void applyTheme(int);
  void showPresetMenu(bool);
  juce::StringArray runControlChecks();
  std::vector<skellcore::ui::GuidedTourStep> tourSteps();
  skellcore::ui::GuidedTourPalette tourPalette()const;
  int getThemeIndex()const{return themeIndex;}
  const Theme::Palette&getPalette()const{return palette;}
 private:
  SkellAdlibLabProcessor&processor;
  SkellAdlibLabEditor&editor;
  Theme::Palette palette;
  int themeIndex=0;
  int lastDisplayedNote=-1;
  HeaderBar header;
  StackPanel stack;
  CharacterPanel character;
  ThrowPanel throwPanel;
  MeltPanel melt;
  ChaosKnob chaos;
  ClipBankView bank;
  MiniKeyboard keyboard;
  PresetMenuView presetMenu;
  juce::Slider width,gain;
  juce::Label widthReadout,gainReadout;
  juce::ToggleButton limiter,formant,velIntensity,velTail,reverse;
  juce::AudioProcessorValueTreeState::SliderAttachment widthAtt,gainAtt;
  juce::AudioProcessorValueTreeState::ButtonAttachment limiterAtt,formantAtt,velIntensityAtt,velTailAtt,reverseAtt;
  std::array<ModulePanel*,4>modules;
 }canvas;
 void timerCallback()override;
 SkellAdlibLabProcessor&processor;
 LabLookAndFeel lookAndFeel;
 std::unique_ptr<skellcore::ui::GuidedTourController> guidedTour;
 void showGuidedTour(bool firstRun);
 juce::TooltipWindow tooltips{this,700};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellAdlibLabEditor)
};
