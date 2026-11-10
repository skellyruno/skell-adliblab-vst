#pragma once
#include "Theme.h"
#include "SkullThemeButton.h"
#include "../PluginProcessor.h"
#include <memory>
class HeaderBar final:public juce::Component {
public:
 explicit HeaderBar(SkellAdlibLabProcessor&p,const Theme::Palette&pal)
 :processor(p),palette(pal),skull(pal),mixAtt(p.parameters,"mix",mix),outputAtt(p.parameters,"outputGain",output){
  addAndMakeVisible(skull);skull.setBounds(11,9,48,48);
  skull.onClick=[this]{if(onThemeCycle)onThemeCycle();};
  skull.onThemeChosen=[this](int theme){if(onThemeSelect)onThemeSelect(theme);};
  setup(mix,palette.stack);setup(output,palette.melt);
  addAndMakeVisible(mix);mix.setBounds(906,6,64,55);
  addAndMakeVisible(output);output.setBounds(980,6,64,55);
  preset.setButtonText("INIT PATCH");preset.setBounds(353,18,326,34);preset.onClick=[this]{showPresets();};addAndMakeVisible(preset);
  shuffle.setButtonText("SHUFFLE");shuffle.setBounds(690,18,100,34);shuffle.onClick=[&p]{p.reRoll();};addAndMakeVisible(shuffle);
  settings.setButtonText("MENU");settings.setBounds(1177,18,62,34);settings.onClick=[this]{showSettings();};addAndMakeVisible(settings);
 }
 std::function<void()> onThemeCycle;
 std::function<void(int)> onThemeSelect;
 void refreshTheme(){setup(mix,palette.stack);setup(output,palette.melt);skull.repaint();repaint();}
 void paint(juce::Graphics&g)override{
  const auto r=getLocalBounds().toFloat();
  g.setGradientFill(juce::ColourGradient(palette.raised.withAlpha(.82f),0.f,0.f,
                                        palette.surface,r.getRight(),r.getBottom(),false));
  g.fillRoundedRectangle(r.reduced(.5f),11.f);
  g.setColour(palette.border.withAlpha(.85f));g.drawRoundedRectangle(r.reduced(.9f),10.5f,1.f);
  g.setColour(palette.stack.withAlpha(.88f));g.fillRoundedRectangle(69.f,58.f,180.f,2.f,1.f);
  g.setColour(palette.muted);g.setFont(Theme::mono(10));
  g.drawText("SKELL AUDIO   /   VOCAL DESIGN",72,7,270,17,juce::Justification::left);
  g.setColour(palette.text);g.setFont(Theme::display(28));
  g.drawText("ADLIB",70,23,110,34,juce::Justification::left);
  g.setColour(palette.stack);g.drawText("LAB",178,23,73,34,juce::Justification::left);
  g.setColour(palette.muted);g.setFont(Theme::mono(9));
  g.drawText(palette.name,268,36,77,16,juce::Justification::left);
  g.setColour(palette.border.withAlpha(.85f));
  g.drawVerticalLine(347,15.f,55.f);g.drawVerticalLine(802,15.f,55.f);g.drawVerticalLine(1064,15.f,55.f);
  g.setColour(palette.muted);g.setFont(Theme::mono(10));
  g.drawText("MIX",911,53,54,12,juce::Justification::centred);
  g.drawText("OUTPUT",982,53,60,12,juce::Justification::centred);
  g.drawText("INPUT",1076,9,72,15,juce::Justification::centredLeft);
  const int lit=juce::roundToInt(12.f*juce::jlimit(0.f,1.f,processor.getInputLevel()));
  for(int i=0;i<12;++i){
   g.setColour((i<lit?(i>9?palette.chaos:palette.melt):palette.border).withAlpha(i<lit?1.f:.5f));
   g.fillRoundedRectangle(1076.f+i*7.25f,31.f,5.2f,7.f,1.4f);
  }
 }

private:
 static void setup(juce::Slider&s,juce::Colour colour){s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);s.setColour(juce::Slider::rotarySliderFillColourId,colour);}
 void parameter(const char*id,float value){
  if(auto*r=dynamic_cast<juce::RangedAudioParameter*>(processor.parameters.getParameter(id))){r->beginChangeGesture();r->setValueNotifyingHost(r->convertTo0to1(value));r->endChangeGesture();}
 }
 // Restore the actual APVTS defaults, not just a few cosmetic controls.
 // Keep samples and the chosen visual skin, which belong to project/UI state.
 void resetFactoryPatch(){
  for(auto* base:processor.getParameters()){
   if(auto* parameter=dynamic_cast<juce::RangedAudioParameter*>(base)){
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->getDefaultValue());
    parameter->endChangeGesture();
   }
  }
  processor.setModuleOrder({{0,1,2,3}});
 }
 void showPresets(){
  juce::PopupMenu menu;menu.addItem(1,"Init Patch");menu.addItem(2,"Wide Adlib Stack");menu.addItem(3,"Robot Incoming");menu.addItem(4,"Ghost Throws");menu.addSeparator();menu.addItem(5,"Save patch file...");menu.addItem(6,"Load patch file...");
  juce::Component::SafePointer<HeaderBar>safe(this);
  menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&preset),[safe](int result){if(!safe)return;
   switch(result){
    case 1:safe->resetFactoryPatch();safe->preset.setButtonText("INIT PATCH");break;
    case 2:safe->parameter("stackVoices",1);safe->parameter("stackSpread",160);safe->parameter("mix",.77f);safe->preset.setButtonText("WIDE ADLIB STACK");break;
    case 3:safe->parameter("charType",1);safe->parameter("charIntensity",.83f);safe->parameter("charCrush",7);safe->preset.setButtonText("ROBOT INCOMING");break;
    case 4:safe->parameter("charType",4);safe->parameter("throwFeedback",.8f);safe->parameter("throwRepeats",8);safe->preset.setButtonText("GHOST THROWS");break;
    case 5:safe->savePreset();break;
    case 6:safe->loadPreset();break;
   }
  });
 }
 void savePreset(){
  chooser=std::make_unique<juce::FileChooser>("Save AdlibLab patch",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("AdlibLab.skad"),"*.skad");
  juce::Component::SafePointer<HeaderBar>safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::warnAboutOverwriting,[safe](const juce::FileChooser&f){if(!safe)return;auto file=f.getResult();if(file==juce::File{})return;
   juce::MemoryBlock state;safe->processor.getStateInformation(state);auto stream=file.createOutputStream();if(stream){stream->setPosition(0);stream->truncate();stream->write(state.getData(),state.getSize());}
  });
 }
 void loadPreset(){
  chooser=std::make_unique<juce::FileChooser>("Load AdlibLab patch",juce::File{},"*.skad");
  juce::Component::SafePointer<HeaderBar>safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser&f){if(!safe)return;auto file=f.getResult();if(!file.existsAsFile()||file.getSize()>10000000)return;juce::MemoryBlock data;if(file.loadFileAsData(data)){safe->processor.setStateInformation(data.getData(),(int)data.getSize());safe->preset.setButtonText("LOADED PATCH");}});
 }
 void showSettings(){juce::PopupMenu menu;menu.addItem(1,"About Skell AdlibLab");menu.addItem(2,"Low quality mode");menu.addItem(3,"Normal quality mode");menu.addItem(4,"High quality mode");
  juce::Component::SafePointer<HeaderBar>safe(this);
  menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&settings),[safe](int result){if(!safe)return;if(result==1)juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Skell AdlibLab","Windows VST3 - experimental audio engine");else if(result>=2&&result<=4)safe->parameter("latencyMode",(float)(result-2));});
 }
 SkellAdlibLabProcessor&processor;const Theme::Palette&palette;SkullThemeButton skull;juce::Slider mix,output;
 juce::AudioProcessorValueTreeState::SliderAttachment mixAtt,outputAtt;
 juce::TextButton preset,shuffle,settings;std::unique_ptr<juce::FileChooser>chooser;
};
