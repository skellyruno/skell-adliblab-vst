#pragma once
#include "ModulePanel.h"
class MeltPanel final:public ModulePanel,public juce::FileDragAndDropTarget {
public:
 explicit MeltPanel(SkellAdlibLabProcessor&p):ModulePanel(p,3,"MELT",Theme::melt,"meltOn"){
  combo("meltMode",{"VOCODER","PITCH FOLLOW","MORPH"},20,65,249);
  load.setButtonText("DROP WAV HERE  /  LOAD...");load.setBounds(25,128,240,85);load.onClick=[this]{choose();};addAndMakeVisible(load);
  knob("meltAmount","MELT",17,252);knob("meltFollowPitch","PITCH",84,252);knob("meltFollowLoudness","LOUDNESS",151,252);knob("meltSmooth","SMOOTH",218,252);knob("meltRoot","ROOT",84,348);
 }
 bool isInterestedInFileDrag(const juce::StringArray&paths)override{return paths.size()>0;}
 void filesDropped(const juce::StringArray&paths,int,int)override{if(paths.size())processor.loadWav(juce::File(paths[0]),-1);repaint();}
 void paint(juce::Graphics&g)override{
  ModulePanel::paint(g);g.setColour(Theme::melt.withAlpha(.12f));g.fillRoundedRectangle(juce::Rectangle<float>(20,109,249,120),7);
  auto*clip=processor.getMeltClip();if(clip){g.setColour(Theme::melt);g.setFont(Theme::mono(11));g.drawFittedText(clip->name,30,213,220,17,juce::Justification::centred,1);}
 }
private:
 void choose(){
  chooser=std::make_unique<juce::FileChooser>("Choose a voice or sample",juce::File{},"*.wav;*.aiff;*.flac");
  juce::Component::SafePointer<MeltPanel>safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser&f){if(safe){auto file=f.getResult();if(file.existsAsFile())safe->processor.loadWav(file,-1);safe->repaint();}});
 }
 juce::TextButton load;std::unique_ptr<juce::FileChooser>chooser;
};
