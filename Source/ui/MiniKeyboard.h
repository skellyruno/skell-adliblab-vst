#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
class MiniKeyboard final: public juce::Component {
public:
 explicit MiniKeyboard(SkellAdlibLabProcessor&p):processor(p){}
 void mouseDown(const juce::MouseEvent& e) override {
  if(e.mods.isPopupMenu())return;
  pressed=noteFromX(e.x);processor.requestMidiOn(pressed);repaint();
 }
 void mouseDrag(const juce::MouseEvent& e) override {
  if(pressed<0)return;
  const int next=noteFromX(e.x);if(next!=pressed){processor.requestMidiOff(pressed);pressed=next;processor.requestMidiOn(pressed);repaint();}
 }
 void mouseUp(const juce::MouseEvent&) override {release();}
 void mouseExit(const juce::MouseEvent& e) override {if(!e.mods.isAnyMouseButtonDown())release();}
 void paint(juce::Graphics&g)override{
  int midiOn=pressed>=0?pressed:processor.getNote();float kw=getWidth()/25.f;
  for(int i=0;i<25;++i){int note=48+i;int degree=note%12;bool black=degree==1||degree==3||degree==6||degree==8||degree==10;
   auto rect=juce::Rectangle<float>(i*kw,0,kw-.8f,(float)getHeight());
   g.setColour(note==midiOn?Theme::melt:black?juce::Colour(0xff1c1927):juce::Colour(0xffd4d2dc));g.fillRoundedRectangle(rect,3);
   g.setColour(Theme::background);g.drawRoundedRectangle(rect,3,1);
  }
 }
private:
 int noteFromX(int x)const{return 48+juce::jlimit(0,24,(int)((float)x*25/juce::jmax(1,getWidth())));}
 void release(){if(pressed>=0){processor.requestMidiOff(pressed);pressed=-1;repaint();}}
 int pressed=-1;
};
