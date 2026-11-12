#pragma once
#include "ModulePanel.h"
#include "../dsp/RackState.h"
#include <array>
// Physical slot overlays for effects owned by the shared SkellCore DSP engine.
class RackExtraPanel final:public ModulePanel {
public:
 RackExtraPanel(SkellAdlibLabProcessor&p,const Theme::Palette&pal,int slot)
  :ModulePanel(p,pal,slot,"CHOP",onId(slot)),physicalSlot(slot){
  knob(amountId(slot),"AMOUNT",17,252);
  knob(rateId(slot),"RATE",84,252);
  knob(toneId(slot),"TONE",151,252);
  knob(mixId(slot),"MIX",218,252);
 }
 void setType(adlib::RackType type){active=type;setEffectTitle(adlib::rackTypeName(type));repaint();}
 void paint(juce::Graphics&g)override{
  ModulePanel::paint(g);
  const auto box=juce::Rectangle<float>(20,109,249,120);
  g.setColour(palette.background);g.fillRoundedRectangle(box,8.f);
  g.setColour(accent().withAlpha(.6f));g.drawRoundedRectangle(box,8.f,1.f);
  g.setColour(palette.muted);g.setFont(Theme::mono(11.f));
  g.drawText(juce::String(adlib::rackTypeName(active))+" / SKELLCORE FX",31,120,216,17,juce::Justification::left);
  juce::Path wave;
  const float phase=(float)(physicalSlot+1)*.23f;
  for(int x=0;x<218;++x){
   const float v=std::sin(x*.074f+phase)*(13.f+9.f*std::sin(x*.017f+phase));
   const float y=180.f+v;
   if(x==0)wave.startNewSubPath(33.f,y);else wave.lineTo(33.f+x,y);
  }
  g.setColour(accent().withAlpha(.88f));g.strokePath(wave,juce::PathStrokeType(1.8f));
  g.setColour(palette.muted);g.setFont(Theme::mono(10.f));
  g.drawText("CLICK NAME TO REPLACE  /  X TO REMOVE",29,206,231,16,juce::Justification::centred);
 }
private:
 static const char* onId(int i){static const char* names[]={"slot0On","slot1On","slot2On","slot3On"};return names[(size_t)i];}
 static const char* amountId(int i){static const char* names[]={"slot0Amount","slot1Amount","slot2Amount","slot3Amount"};return names[(size_t)i];}
 static const char* rateId(int i){static const char* names[]={"slot0Rate","slot1Rate","slot2Rate","slot3Rate"};return names[(size_t)i];}
 static const char* toneId(int i){static const char* names[]={"slot0Tone","slot1Tone","slot2Tone","slot3Tone"};return names[(size_t)i];}
 static const char* mixId(int i){static const char* names[]={"slot0Mix","slot1Mix","slot2Mix","slot3Mix"};return names[(size_t)i];}
 int physicalSlot;
 adlib::RackType active=adlib::RackType::Chop;
};
class EmptyRackSlot final:public juce::Component {
public:
 EmptyRackSlot(const Theme::Palette&pal,int i):palette(pal),slot(i){setMouseCursor(juce::MouseCursor::PointingHandCursor);}
 std::function<void(int)>onAdd;
 void mouseUp(const juce::MouseEvent&)override{if(onAdd)onAdd(slot);}
 void paint(juce::Graphics&g)override{
  const auto b=getLocalBounds().toFloat().reduced(3.f);
  g.setColour(palette.panel);g.fillRoundedRectangle(b,11.f);
  g.setColour(palette.border.brighter(.18f));g.drawRoundedRectangle(b,11.f,1.5f);
  const auto target=juce::Rectangle<float>(36.f,135.f,216.f,171.f);
  g.setColour(palette.raised);g.fillRoundedRectangle(target,11.f);
  g.setColour(palette.stack.withAlpha(.9f));g.drawRoundedRectangle(target,11.f,2.f);
  const float cx=144.f,cy=199.f;
  g.drawLine(cx-19.f,cy,cx+19.f,cy,3.6f);g.drawLine(cx,cy-19.f,cx,cy+19.f,3.6f);
  g.setFont(Theme::display(16));g.setColour(palette.text);
  g.drawText("ADD EFFECT",32,259,224,24,juce::Justification::centred);
  g.setFont(Theme::mono(11.f));g.setColour(palette.muted);
  g.drawText("SLOT "+juce::String(slot+1)+" / EMPTY",27,21,236,22,juce::Justification::centred);
 }
private:const Theme::Palette&palette;int slot;
};
