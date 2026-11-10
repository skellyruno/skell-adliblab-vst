#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
class ClipBankView final:public juce::Component,public juce::FileDragAndDropTarget {
public:
 explicit ClipBankView(SkellAdlibLabProcessor&p,const Theme::Palette&pal):processor(p),palette(pal){}
 bool isInterestedInFileDrag(const juce::StringArray&paths)override{return paths.size()>0;}
 void filesDropped(const juce::StringArray&paths,int x,int)override{if(paths.size())processor.loadWav(juce::File(paths[0]),padAt(x));repaint();}
 void paint(juce::Graphics&g)override{
  auto w=(float)getWidth()/8.f;
  for(int i=0;i<8;++i){auto r=juce::Rectangle<float>(i*w+2,2,w-5,(float)getHeight()-4);
   g.setColour(palette.surface);g.fillRoundedRectangle(r,6);
   g.setColour(i%2?palette.character.withAlpha(.35f):palette.stack.withAlpha(.35f));g.drawRoundedRectangle(r,6,1.f);
   g.setColour(palette.text);g.setFont(Theme::mono(10));auto*clip=processor.clips.getClip(i);
   g.drawFittedText(clip?clip->name:juce::String("PAD ")+juce::String(i+1),r.toNearestInt().reduced(6).withHeight(18),juce::Justification::centred,1);
   auto mid=r.getCentreY()+8;
   if(clip&&clip->mono.size()>12){
    g.setColour(palette.melt.withAlpha(.75f));
    for(int j=0;j<34;++j){auto idx=(size_t)j*clip->mono.size()/34;auto a=std::abs(clip->mono[idx]);g.drawVerticalLine((int)(r.getX()+7+j*(r.getWidth()-14)/34),mid-a*14,mid+a*14);}
   }else{g.setColour(palette.muted);g.drawLine(r.getX()+10,mid,r.getRight()-10,mid,1);}
  }
 }
 void mouseUp(const juce::MouseEvent&e)override{
  int pad=padAt(e.x);if(e.mods.isPopupMenu()){
   juce::PopupMenu menu;menu.addItem(1,"Load audio...");menu.addItem(2,"Clear pad");menu.addItem(3,"Audition");menu.addItem(4,"Record next phrase");
   juce::Component::SafePointer<ClipBankView>safe(this);
   menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),[safe,pad](int choice){
    if(safe==nullptr)return;
    if(choice==1)safe->openPad(pad);else if(choice==4)safe->processor.armRecording(pad);else if(choice==2)safe->processor.clearPad(pad);else if(choice==3)safe->processor.triggerPad(pad);
    safe->repaint();
   });
  }else if(e.mods.isShiftDown())processor.armRecording(pad);else processor.triggerPad(pad);
 }
private:
 int padAt(int x)const{return juce::jlimit(0,7,x*8/juce::jmax(1,getWidth()));}
 void openPad(int pad){
  chooser=std::make_unique<juce::FileChooser>("Load an adlib clip",juce::File{},"*.wav;*.aiff;*.flac");
  juce::Component::SafePointer<ClipBankView>safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe,pad](const juce::FileChooser&f){
   if(safe){auto file=f.getResult();if(file.existsAsFile())safe->processor.loadWav(file,pad);safe->repaint();}
  });
 }
 SkellAdlibLabProcessor&processor;const Theme::Palette&palette;std::unique_ptr<juce::FileChooser>chooser;
};
