#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/StackPanel.h"
#include "ui/CharacterPanel.h"
#include "ui/ThrowPanel.h"
#include "ui/MeltPanel.h"
#include "ui/RackExtraPanel.h"
#include "ui/EffectPickerView.h"
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
 void setEffectPickerForScreenshot(bool);
 void refreshRackForScreenshot();
 void setDragPreviewForScreenshot(bool);
 juce::StringArray runUIInteractionTests();
 bool showGuidedTourForScreenshot();
 void dismissGuidedTourForScreenshot();
 bool keyPressed(const juce::KeyPress&)override;
 bool keyStateChanged(bool)override;
 void focusLost(FocusChangeType)override;
 void releaseComputerNotes();
private:
 class Canvas final:public juce::Component,private juce::Slider::Listener {
 public:
  Canvas(SkellAdlibLabProcessor&,SkellAdlibLabEditor&);
  ~Canvas()override{width.removeListener(this);gain.removeListener(this);}
  void paint(juce::Graphics&)override;
  void paintOverChildren(juce::Graphics&)override;
  void reorder(int slot,int displacement);
  int slotForLegacyPanel(int id)const;
  void previewDrag(int slot,int delta,bool active);
  void openEffectPicker(int slot);
  void showEffectPickerForScreenshot(bool enabled);
  void ensureRackLayoutCurrent();
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
  std::array<std::unique_ptr<RackExtraPanel>,4> extraPanels;
  std::array<std::unique_ptr<EmptyRackSlot>,4> emptySlots;
  EffectPickerView effectPicker;
  int pickerSlot=-1,draggingSlot=-1,draggingDelta=0;
  adlib::RackTypes displayedTypes{};
  std::array<int,4>displayedOrder{{-1,-1,-1,-1}};
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
 int keyboardOctave=0;
 std::array<bool,24>computerHeld{};
 LabLookAndFeel lookAndFeel;
 std::unique_ptr<skellcore::ui::GuidedTourController> guidedTour;
 void showGuidedTour(bool firstRun);
 juce::TooltipWindow tooltips{this,700};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellAdlibLabEditor)
};
