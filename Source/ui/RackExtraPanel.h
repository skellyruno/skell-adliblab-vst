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
 void setType(adlib::RackType type){
  if(active==type)return;
  active=type;setEffectTitle(adlib::rackTypeName(type));repaint();
 }
 void paint(juce::Graphics&g)override{
  ModulePanel::paint(g);
  const float amount=value("Amount"),rate=value("Rate"),tone=value("Tone");
  const auto colour=accent();
  const juce::Rectangle<float> frame(20.f,108.f,249.f,129.f);
  g.setColour(palette.background);g.fillRoundedRectangle(frame,8.f);
  g.setColour(palette.border.brighter(.11f));g.drawRoundedRectangle(frame,8.f,.85f);
  g.setColour(palette.muted);g.setFont(Theme::mono(10.5f));
  const juce::String caption=
    active==adlib::RackType::Chop?"BEAT GATE / PATTERN":
    active==adlib::RackType::Motion?"STEREO / MOVEMENT":
    active==adlib::RackType::Filter?"FREQUENCY / SHAPE":
    "DRIVE / TRANSFER";
  g.drawText(caption,31,118,218,17,juce::Justification::left);
  g.setColour(palette.border.withAlpha(.47f));
  for(int i=1;i<=3;++i){
   const float x=34.f+54.f*(float)i;
   g.drawVerticalLine((int)x,149.f,214.f);
  }
  g.drawHorizontalLine(184,34.f,252.f);
  const float x0=34.f,y0=149.f,width=218.f,height=65.f;
  juce::Path shape;
  auto append=[&](float x,float y){
   if(shape.isEmpty())shape.startNewSubPath(x,y);else shape.lineTo(x,y);
  };
  if(active==adlib::RackType::Chop){
   const int pulses=2+(int)std::floor(rate*5.99f);
   const float duty=.82f-.55f*tone;
   for(int n=0;n<=218;++n){
    const float t=(float)n/218.f*(float)pulses;
    const float phase=t-std::floor(t);
    const float envelope=phase<duty?1.f:0.f;
    const float gate=(1.f-amount)+amount*envelope;
    append(x0+(float)n,y0+height-8.f-gate*(height-16.f));
   }
  }else if(active==adlib::RackType::Motion){
   const float turns=1.f+3.5f*rate;
   for(int n=0;n<=218;++n){
    const float t=(float)n/218.f;
    const float l=std::sin(t*juce::MathConstants<float>::twoPi*turns);
    append(x0+(float)n,y0+height*.5f-l*height*.39f*amount);
   }
   juce::Path opposite;
   for(int n=0;n<=218;++n){
    const float t=(float)n/218.f;
    const float v=std::sin(t*juce::MathConstants<float>::twoPi*turns);
    const float x=x0+(float)n,y=y0+height*.5f+v*height*.39f*amount;
    if(n==0)opposite.startNewSubPath(x,y);else opposite.lineTo(x,y);
   }
   g.setColour(colour.withAlpha(.38f));g.strokePath(opposite,juce::PathStrokeType(1.4f));
  }else if(active==adlib::RackType::Filter){
   for(int n=0;n<=218;++n){
    const float t=(float)n/218.f;
    const float low=1.f/(1.f+std::exp((t-tone)*14.f));
    const float response=juce::jlimit(0.f,1.f,(1.f-rate)*low+rate*(1.f-low));
    append(x0+(float)n,y0+height-5.f-response*(height-10.f)*amount);
   }
  }else{
   const float drive=1.f+amount*14.f;
   const float inv=1.f/std::tanh(drive);
   for(int n=0;n<=218;++n){
    const float t=(float)n/218.f*2.f-1.f;
    const float out=std::tanh(t*drive)*inv*(.6f+.4f*tone);
    append(x0+(float)n,y0+height*.5f-out*height*.44f);
   }
  }
  g.setColour(colour.withAlpha(.93f));
  g.strokePath(shape,juce::PathStrokeType(2.f,juce::PathStrokeType::curved,
                                      juce::PathStrokeType::rounded));
  // A short type-specific explanation uses the previously empty lower half.
  const auto info=juce::Rectangle<float>(20.f,365.f,249.f,62.f);
  g.setColour(palette.raised);g.fillRoundedRectangle(info,7.f);
  g.setColour(palette.border.withAlpha(.7f));g.drawRoundedRectangle(info,7.f,.8f);
  const char* heading=active==adlib::RackType::Chop?"RHYTHMIC CUTS":
                      active==adlib::RackType::Motion?"STEREO MOTION":
                      active==adlib::RackType::Filter?"TONE SHAPING":"SOFT SATURATION";
  const char* detail=active==adlib::RackType::Chop?"Tempo-linked gate with smooth edges":
                     active==adlib::RackType::Motion?"Moving left / right spatial balance":
                     active==adlib::RackType::Filter?"From darker to brighter frequency tone":
                     "Harmonic colour and controlled drive";
  g.setFont(Theme::mono(11.f));g.setColour(colour);
  g.drawText(heading,31,376,216,18,juce::Justification::left);
  g.setFont(Theme::mono(10.f));g.setColour(palette.text.withAlpha(.83f));
  g.drawFittedText(detail,31,396,218,18,juce::Justification::centredLeft,1);
 }
private:
 float value(const char* suffix)const noexcept{
  const auto key="slot"+juce::String(physicalSlot)+suffix;
  if(const auto* val=processor.parameters.getRawParameterValue(key))
   return juce::jlimit(0.f,1.f,val->load(std::memory_order_relaxed));
  return 0.5f;
 }
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
 EmptyRackSlot(const Theme::Palette&pal,int i):palette(pal),slot(i){
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
  setWantsKeyboardFocus(true);
  setTooltip("Add an effect to this empty slot");
 }
 std::function<void(int)>onAdd;
 void mouseUp(const juce::MouseEvent&)override{if(onAdd)onAdd(slot);}
 void mouseEnter(const juce::MouseEvent&)override{hover=true;repaint();}
 void mouseExit(const juce::MouseEvent&)override{hover=false;repaint();}
 bool keyPressed(const juce::KeyPress&k)override{
  if(k==juce::KeyPress::returnKey||k==juce::KeyPress::spaceKey){
   if(onAdd)onAdd(slot);
   return true;
  }
  return false;
 }
 void paint(juce::Graphics&g)override{
  const auto b=getLocalBounds().toFloat().reduced(3.f);
  g.setColour(palette.panel);g.fillRoundedRectangle(b,11.f);
  g.setColour(palette.border.brighter(.18f));g.drawRoundedRectangle(b,11.f,1.5f);
  const auto target=juce::Rectangle<float>(36.f,135.f,216.f,171.f);
  g.setColour(palette.raised);g.fillRoundedRectangle(target,11.f);
  g.setColour(palette.stack.withAlpha(hover?1.f:.9f));g.drawRoundedRectangle(target,11.f,hover?2.8f:2.f);
  const float cx=144.f,cy=199.f;
  g.drawLine(cx-19.f,cy,cx+19.f,cy,3.6f);g.drawLine(cx,cy-19.f,cx,cy+19.f,3.6f);
  g.setFont(Theme::display(16));g.setColour(palette.text);
  g.drawText("ADD EFFECT",32,259,224,24,juce::Justification::centred);
  g.setFont(Theme::mono(11.f));g.setColour(palette.muted);
  g.drawText("SLOT "+juce::String(slot+1)+" / EMPTY",27,21,236,22,juce::Justification::centred);
 }
private:const Theme::Palette&palette;int slot;bool hover=false;
};
