#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
class VoiceMap final: public juce::Component {
public:
 explicit VoiceMap(SkellAdlibLabProcessor&p,const Theme::Palette&pal):processor(p),palette(pal){setRepaintsOnMouseActivity(true);}
 void paint(juce::Graphics&g)override{
  auto r=getLocalBounds().toFloat();g.setColour(palette.background);g.fillRoundedRectangle(r,7);
  g.setColour(palette.stack.withAlpha(.18f));
  for(int i=1;i<5;++i){auto x=r.getWidth()*i/5.f;g.drawVerticalLine((int)x,0,r.getBottom());}
  for(int i=1;i<3;++i){auto y=r.getHeight()*i/3.f;g.drawHorizontalLine((int)y,0,r.getRight());}
  int count=juce::jlimit(2,8,2+(int)std::lround(processor.parameters.getRawParameterValue("stackVoices")->load()));
  for(int i=0;i<count;++i){float pan=processor.getVoicePan(i),pitch=processor.getVoicePitch(i);
   float x=juce::jmap(pan,-1.f,1.f,10.f,r.getWidth()-10),y=juce::jmap(pitch,-1.f,1.f,r.getHeight()-10,10.f);
   g.setColour(palette.stack.withAlpha(i==dragIndex?1.f:.78f));g.fillEllipse(x-4,y-4,8,8);
  }
  g.setColour(palette.muted);g.setFont(Theme::mono(10));g.drawText("PAN / PITCH",8,2,100,14,juce::Justification::left);
 }
 void mouseDown(const juce::MouseEvent&e)override{
  float best=1.e9f;dragIndex=-1;
  int count=juce::jlimit(2,8,2+(int)std::lround(processor.parameters.getRawParameterValue("stackVoices")->load()));
  for(int i=0;i<count;++i){float x=juce::jmap(processor.getVoicePan(i),-1.f,1.f,10.f,(float)getWidth()-10),y=juce::jmap(processor.getVoicePitch(i),-1.f,1.f,(float)getHeight()-10,10.f);float d=std::hypot(e.position.x-x,e.position.y-y);if(d<best){best=d;dragIndex=i;}}
  if(best>23)dragIndex=-1;
  mouseDrag(e);
 }
 void mouseDrag(const juce::MouseEvent&e)override{if(dragIndex<0)return;float pan=juce::jmap(e.position.x,10.f,(float)getWidth()-10,-1.f,1.f),pitch=juce::jmap(e.position.y,10.f,(float)getHeight()-10,1.f,-1.f);processor.setVoicePosition(dragIndex,juce::jlimit(-1.f,1.f,pan),juce::jlimit(-1.f,1.f,pitch));repaint();}
 void mouseUp(const juce::MouseEvent&)override{dragIndex=-1;repaint();}
private:SkellAdlibLabProcessor&processor;const Theme::Palette&palette;int dragIndex=-1;
};
