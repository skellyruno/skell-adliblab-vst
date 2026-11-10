#pragma once
#include "ModulePanel.h"
class ThrowPanel final:public ModulePanel {
public:
 explicit ThrowPanel(SkellAdlibLabProcessor&p,const Theme::Palette&pal):ModulePanel(p,pal,2,"THROW","throwOn"){
  combo("throwMode",{"LAST WORD","SYLLABLE","MANUAL","ALWAYS"},18,64,155);
  combo("throwDiv",{"1/4","1/8","1/16","1/8.","1/8T","1/32","1/64"},180,64,90);
  knob("throwFeedback","FEEDBACK",17,252);knob("throwRepeats","REPEATS",84,252);knob("throwFilterStart","FILTER START",151,252);knob("throwFilterEnd","FILTER END",218,252);
  knob("throwPitchPerRepeat","PITCH",17,348);knob("throwDuck","DUCK",84,348);
  toggle("throwReverse","REVERSE",154,350);toggle("throwPingPong","PING-PONG",154,380);
  fire.setButtonText("THROW NOW");fire.setBounds(163,409,105,29);fire.onClick=[&p]{p.triggerThrow();};addAndMakeVisible(fire);
 }
 void paint(juce::Graphics&g)override{
  ModulePanel::paint(g);auto r=juce::Rectangle<float>(20,109,249,120);g.setColour(palette.background);g.fillRoundedRectangle(r,7);
  for(int i=0;i<10;++i){float h=60.f*std::pow(.82f,i);auto col=palette.throwColour.interpolatedWith(palette.melt,i/10.f);g.setColour(col.withAlpha(.75f));g.fillRoundedRectangle(35.f+i*22.f,178-h*.5f,12.f,h,2);}
  g.setColour(palette.muted);g.setFont(Theme::mono(10));g.drawText("TAIL / REPEATS",29,112,206,18,juce::Justification::left);
 }
private:juce::TextButton fire;
};
