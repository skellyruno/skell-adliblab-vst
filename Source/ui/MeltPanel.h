#pragma once
#include "ModulePanel.h"
class MeltPanel final:public ModulePanel,public juce::FileDragAndDropTarget {
public:
 explicit MeltPanel(SkellAdlibLabProcessor&p,const Theme::Palette&pal):ModulePanel(p,pal,3,"MELT","meltOn"){
  combo("meltMode",{"VOCODER","PITCH FOLLOW","MORPH"},20,65,249);
  load.setButtonText("LOAD / CHANGE SAMPLE");load.setBounds(30,134,229,45);load.onClick=[this]{choose();};addAndMakeVisible(load);
  knob("meltAmount","MELT",17,252);knob("meltFollowPitch","PITCH",84,252);knob("meltFollowLoudness","LOUDNESS",151,252);knob("meltSmooth","SMOOTH",218,252);knob("meltRoot","ROOT",84,348);
 }
 bool isInterestedInFileDrag(const juce::StringArray&paths)override{return paths.size()>0;}
 void filesDropped(const juce::StringArray&paths,int,int)override{if(paths.size())processor.loadWav(juce::File(paths[0]),-1);repaint();}
 void paint(juce::Graphics&g)override{
  ModulePanel::paint(g);
  const auto box=juce::Rectangle<float>(20,109,249,120);
  g.setColour(palette.melt.withAlpha(.07f));g.fillRoundedRectangle(box,8.f);
  g.setColour(palette.melt.withAlpha(.32f));g.drawRoundedRectangle(box,8.f,1.f);
  g.setColour(palette.muted);g.setFont(Theme::mono(10));
  g.drawText("SAMPLE / CARRIER",30,113,200,15,juce::Justification::left);
  auto*clip=processor.getMeltClip();
  if(clip && !clip->mono.empty()){
   g.setColour(palette.melt.withAlpha(.84f));
   const float middle=198.f;
   const auto length=clip->mono.size();
   for(int x=0;x<217;++x){
    const auto start=(size_t)x*length/217;
    const auto stop=juce::jmin(length,(size_t)(x+1)*length/217);
    float peak=0;
    for(size_t i=start;i<stop;i+=juce::jmax((size_t)1,(stop-start)/8))
     peak=juce::jmax(peak,std::abs(clip->mono[i]));
    const float h=peak*13.f;
    g.drawVerticalLine(36+x,middle-h,middle+h);
   }
   g.setColour(palette.text);g.setFont(Theme::mono(10));
   g.drawFittedText(clip->name,30,212,228,16,juce::Justification::centred,1);
  }else{
   g.setColour(palette.muted.withAlpha(.75f));g.setFont(Theme::mono(10));
   g.drawText("DROP WAV / AIFF / FLAC",31,191,228,19,juce::Justification::centred);
  }
 }
private:
 void choose(){
  chooser=std::make_unique<juce::FileChooser>("Choose a voice or sample",juce::File{},"*.wav;*.aiff;*.flac");
  juce::Component::SafePointer<MeltPanel>safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser&f){if(safe){auto file=f.getResult();if(file.existsAsFile())safe->processor.loadWav(file,-1);safe->repaint();}});
 }
 juce::TextButton load;std::unique_ptr<juce::FileChooser>chooser;
};
