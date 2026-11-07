#pragma once
#include "ModulePanel.h"
class CharacterPanel final:public ModulePanel {
public:
 explicit CharacterPanel(SkellAdlibLabProcessor&p):ModulePanel(p,1,"CHARACTER",Theme::character,"charOn"){
  combo("charType",{"Alien","Robot","Demon","Cartoon","Ghost","Gremlin"},20,65,249);
  knob("charIntensity","INTENSITY",17,252);knob("charFormant","FORMANT",84,252);knob("charGlide","GLIDE",151,252);knob("charCrush","CRUSH",218,252);knob("charDetune","DETUNE",84,348);
 }
 void paint(juce::Graphics&g)override{
  ModulePanel::paint(g);auto r=juce::Rectangle<float>(20,109,249,120);g.setColour(juce::Colour(0xff0c0b12));g.fillRoundedRectangle(r,7);
  juce::Path wave;for(int x=0;x<230;++x){float t=x*.08f;float v=std::sin(t)*std::sin(t*.33f)*18.f+std::sin(t*2)*4;auto y=170.f+v;if(x==0)wave.startNewSubPath(30.f,(float)y);else wave.lineTo(30.f+x,y);}
  g.setColour(Theme::character.withAlpha(.85f));g.strokePath(wave,juce::PathStrokeType(1.5f));
  g.setColour(Theme::muted);g.setFont(Theme::mono(11));g.drawText("VOICE PERSONALITY",32,112,213,18,juce::Justification::centred);
 }
};
