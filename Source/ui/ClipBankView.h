#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
class ClipBankView final:public juce::Component,public juce::FileDragAndDropTarget {
public:
 explicit ClipBankView(SkellAdlibLabProcessor&p,const Theme::Palette&pal):processor(p),palette(pal){}
 bool isInterestedInFileDrag(const juce::StringArray&paths)override{return paths.size()>0;}
 void filesDropped(const juce::StringArray&paths,int x,int)override{if(paths.size())processor.loadWav(juce::File(paths[0]),padAt(x));repaint();}
 void paint(juce::Graphics&g)override{
  const float cell=(float)getWidth()/8.f;
  const int note=processor.getNote();
  const int root=(int)std::lround(processor.parameters.getRawParameterValue("kbRoot")->load());
  for(int i=0;i<8;++i){
   const auto r=juce::Rectangle<float>(i*cell+2.f,2.f,cell-5.f,(float)getHeight()-4.f);
   const bool active=note>=0&&((note-root)%8+8)%8==i;
   const bool hot=hoverPad==i;
   const auto accent=i%2?palette.character:palette.stack;
   g.setGradientFill(juce::ColourGradient(active?palette.raised:palette.surface.brighter(hot?.12f:0.f),
                                          r.getTopLeft(),palette.background,r.getBottomRight(),false));
   g.fillRoundedRectangle(r,6.f);
   g.setColour(active?palette.melt:(hot?accent:palette.border));
   g.drawRoundedRectangle(r.reduced(.5f),6.f,active?1.8f:1.f);
   if(active){g.setColour(palette.melt);g.fillRoundedRectangle(r.getX()+5.f,r.getY()+1.f,r.getWidth()-10.f,2.f,1.f);}
   const auto*clip=processor.clips.getClip(i);
   g.setColour(active?palette.melt:palette.muted);g.setFont(Theme::mono(9));
   g.drawText(juce::String(i+1).paddedLeft('0',2),(int)(r.getX()+6),(int)(r.getY()+5),16,12,juce::Justification::left);
   g.setColour(palette.text);g.setFont(Theme::mono(10));
   const auto name=clip?clip->name:juce::String("EMPTY");
   g.drawFittedText(name,(int)(r.getX()+23),(int)(r.getY()+4),
                    juce::jmax(12,(int)r.getWidth()-29),15,juce::Justification::centredRight,1);
   const float mid=r.getCentreY()+9.f;
   if(clip&&clip->mono.size()>12){
    g.setColour((active?palette.melt:accent).withAlpha(.8f));
    const auto count=clip->mono.size();
    for(int j=0;j<28;++j){
     const auto begin=(size_t)j*count/28;
     const auto finish=juce::jmin(count,begin+juce::jmax((size_t)1,count/28));
     float peak=0.f;
     const auto stride=juce::jmax((size_t)1,(finish-begin)/12);
     for(auto k=begin;k<finish;k+=stride)peak=juce::jmax(peak,std::abs(clip->mono[k]));
     const float x=r.getX()+8.f+j*(r.getWidth()-16.f)/28.f;
     const float height=juce::jlimit(1.f,19.f,peak*20.f);
     g.drawVerticalLine((int)x,mid-height,mid+height);
    }
   }else{
    g.setColour(palette.border.brighter(.18f));
    g.drawHorizontalLine((int)mid,r.getX()+9.f,r.getRight()-9.f);
    for(int j=0;j<3;++j)g.fillEllipse(r.getCentreX()-10.f+j*8.f,mid-1.f,2.f,2.f);
   }
  }
 }
 void mouseMove(const juce::MouseEvent&e)override{const int next=padAt(e.x);if(hoverPad!=next){hoverPad=next;repaint();}}
 void mouseExit(const juce::MouseEvent&)override{if(hoverPad!=-1){hoverPad=-1;repaint();}}

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
 int hoverPad=-1;
 SkellAdlibLabProcessor&processor;const Theme::Palette&palette;std::unique_ptr<juce::FileChooser>chooser;
};
