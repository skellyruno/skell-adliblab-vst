#pragma once
#include "ModulePanel.h"
class StackPanel final:public ModulePanel {
public:
 explicit StackPanel(SkellAdlibLabProcessor&p):ModulePanel(p,0,"STACK",Theme::stack,"stackOn"),map(p){
  combo("stackVoices",{"2","3","4","5","6","7","8"},20,65,105);
  addAndMakeVisible(map);map.setBounds(20,109,249,120);
  knob("stackSpread","SPREAD",17,252);knob("stackFormant","FORMANT",84,252);knob("stackTime","TIME",151,252);knob("stackWidth","WIDTH",218,252);
  knob("stackChaosAmt","CHAOS AMT",17,348);toggle("stackRerollOnHit","RE-ROLL ON HIT",95,349);toggle("stackScaleLock","SCALE LOCK",95,375);
  combo("stackKey",{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},105,408,74);
  combo("stackScale",{"Major","Minor","Chromatic","Pentatonic"},186,408,82);
 }
private:VoiceMap map;
};
