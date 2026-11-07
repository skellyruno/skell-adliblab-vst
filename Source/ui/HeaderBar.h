#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
#include <memory>
class HeaderBar final:public juce::Component {
public:
 explicit HeaderBar(SkellAdlibLabProcessor&p):processor(p),mixAtt(p.parameters,"mix",mix),outputAtt(p.parameters,"outputGain",output){
  setup(mix,Theme::stack);setup(output,Theme::melt);
  addAndMakeVisible(mix);mix.setBounds(946,6,63,55);
  addAndMakeVisible(output);output.setBounds(1022,6,63,55);
  preset.setButtonText("INIT PATCH   ▾");preset.setBounds(370,20,310,31);preset.onClick=[this]{showPresets();};addAndMakeVisible(preset);
  shuffle.setButtonText("SHUFFLE");shuffle.setBounds(691,20,85,31);shuffle.onClick=[&p]{p.reRoll();};addAndMakeVisible(shuffle);
  settings.setButtonText("☰");settings.setBounds(1194,20,55,31);settings.onClick=[this]{showSettings();};addAndMakeVisible(settings);
 }
 void paint(juce::Graphics&g)override{
  g.setColour(juce::Colour(0xff101019));g.fillRoundedRectangle(getLocalBounds().toFloat(),9.f);
  g.setColour(Theme::melt);g.setFont(Theme::display(29));g.drawText("ADLIB",24,13,134,42,juce::Justification::left);
  g.setColour(Theme::text);g.drawText("LAB",163,13,88,42,juce::Justification::left);
  g.setColour(Theme::muted);g.setFont(Theme::mono(10));g.drawText("SKELL AUDIO",256,30,102,17,juce::Justification::left);
  g.setColour(Theme::muted);g.setFont(Theme::mono(10));g.drawText("MIX",945,53,65,12,juce::Justification::centred);
  g.drawText("OUTPUT",1023,53,65,12,juce::Justification::centred);
  g.drawText("IN",1109,12,65,12,juce::Justification::left);
  g.setColour(Theme::panel.brighter(.4f));g.fillRoundedRectangle(juce::Rectangle<float>(1109,30,71,8),2);
  g.setColour(Theme::character);g.fillRoundedRectangle(juce::Rectangle<float>(1110,31,69*juce::jlimit(0.f,1.f,processor.getInputLevel()),6),1);
 }
private:
 static void setup(juce::Slider&s,juce::Colour colour){s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);s.setColour(juce::Slider::rotarySliderFillColourId,colour);}
 void parameter(const char*id,float value){
  if(auto*r=dynamic_cast<juce::RangedAudioParameter*>(processor.parameters.getParameter(id))){r->beginChangeGesture();r->setValueNotifyingHost(r->convertTo0to1(value));r->endChangeGesture();}
 }
 void showPresets(){
  juce::PopupMenu menu;menu.addItem(1,"Init Patch");menu.addItem(2,"Wide Adlib Stack");menu.addItem(3,"Robot Incoming");menu.addItem(4,"Ghost Throws");menu.addSeparator();menu.addItem(5,"Save patch file...");menu.addItem(6,"Load patch file...");
  juce::Component::SafePointer<HeaderBar>safe(this);
  menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&preset),[safe](int result){if(!safe)return;
   switch(result){
    case 1:safe->parameter("chaos",0);safe->parameter("charType",0);safe->parameter("charIntensity",.35f);safe->preset.setButtonText("INIT PATCH   ▾");break;
    case 2:safe->parameter("stackVoices",1);safe->parameter("stackSpread",160);safe->parameter("mix",.77f);safe->preset.setButtonText("WIDE ADLIB STACK  ▾");break;
    case 3:safe->parameter("charType",1);safe->parameter("charIntensity",.83f);safe->parameter("charCrush",7);safe->preset.setButtonText("ROBOT INCOMING  ▾");break;
    case 4:safe->parameter("charType",4);safe->parameter("throwFeedback",.8f);safe->parameter("throwRepeats",8);safe->preset.setButtonText("GHOST THROWS  ▾");break;
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
  chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser&f){if(!safe)return;auto file=f.getResult();if(!file.existsAsFile()||file.getSize()>10000000)return;juce::MemoryBlock data;if(file.loadFileAsData(data))safe->processor.setStateInformation(data.getData(),(int)data.getSize());});
 }
 void showSettings(){juce::PopupMenu menu;menu.addItem(1,"About Skell AdlibLab");menu.addItem(2,"Low quality mode");menu.addItem(3,"Normal quality mode");menu.addItem(4,"High quality mode");
  juce::Component::SafePointer<HeaderBar>safe(this);
  menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&settings),[safe](int result){if(!safe)return;if(result==1)juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Skell AdlibLab","VST3 / Standalone • experimental audio engine");else if(result>=2&&result<=4)safe->parameter("latencyMode",(float)(result-2));});
 }
 SkellAdlibLabProcessor&processor;juce::Slider mix,output;
 juce::AudioProcessorValueTreeState::SliderAttachment mixAtt,outputAtt;
 juce::TextButton preset,shuffle,settings;std::unique_ptr<juce::FileChooser>chooser;
};
