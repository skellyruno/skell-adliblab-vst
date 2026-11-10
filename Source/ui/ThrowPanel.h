#pragma once
#include "ModulePanel.h"
class ThrowPanel final:public ModulePanel {
public:
 explicit ThrowPanel(SkellAdlibLabProcessor&p,const Theme::Palette&pal):ModulePanel(p,pal,2,"THROW","throwOn"){
  combo("throwMode",{"LAST WORD","SYLLABLE","MANUAL","ALWAYS"},18,64,155);
  combo("throwDiv",{"1/4","1/8","1/16","1/8.","1/8T","1/32","1/64"},180,64,90);
  knob("throwFeedback","FEEDBACK",17,252);knob("throwRepeats","REPEATS",84,252);knob("throwFilterStart","FILTER START",151,252);knob("throwFilterEnd","FILTER END",218,252);
  knob("throwPitchPerRepeat","PITCH",17,348);knob("throwDuck","DUCK",84,348);
  toggle("throwReverse","REVERSE",154,350).setBounds(154,350,110,24);
  toggle("throwPingPong","PING-PONG",154,380).setBounds(154,380,110,24);
  fire.setButtonText("THROW NOW");fire.setBounds(163,409,105,29);fire.onClick=[&p]{p.triggerThrow();};addAndMakeVisible(fire);
 }
 void paint(juce::Graphics&g)override{
  ModulePanel::paint(g);
  const auto r=juce::Rectangle<float>(20,109,249,120);
  g.setColour(palette.background);g.fillRoundedRectangle(r,8.f);
  g.setColour(palette.border.withAlpha(.70f));g.drawRoundedRectangle(r,8.f,1.f);
  const float activity=juce::jlimit(0.f,1.f,processor.getThrowActivity());
  g.setColour(palette.muted);g.setFont(Theme::mono(10));
  g.drawText("ECHO SEQUENCE",30,115,148,18,juce::Justification::left);
  g.setColour(activity>.01f?palette.throwColour:palette.muted);
  g.drawText(activity>.01f?"ACTIVE":"READY",192,115,65,18,juce::Justification::right);
  // Meter is live (the old display was a completely static graph).
  for(int i=0;i<10;++i){
   const float decayed=std::pow(.79f,(float)i);
   const float h=68.f*decayed;
   const float highlight=(1.f-activity)*.25f+activity;
   auto colour=palette.throwColour.interpolatedWith(palette.melt,i/9.f);
   g.setColour(colour.withAlpha(.19f+.76f*highlight));
   g.fillRoundedRectangle(34.f+i*22.f,208.f-h,12.f,h,2.4f);
  }
  g.setColour(palette.muted.withAlpha(.65f));g.drawHorizontalLine(212,30.f,259.f);
 }
private:juce::TextButton fire;
};
