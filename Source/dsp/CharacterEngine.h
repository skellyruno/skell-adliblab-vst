#pragma once
#include "EngineTypes.h"
#include "ChaosEngine.h"
namespace adlib {
class CharacterEngine {
public:
 void prepare(double sr) {sampleRate=sr;phase=0;held={};holdCounter=0;filter={};tone={};smoothedIntensity=0;}
 Stereo tick(Stereo s,const Params& p,const ChaosEngine& chaos) {
  if(!p.charOn)return s;
  float target=clamp(p.charIntensity,0,1);
  float glide=1.f/(1.f+sampleRate*p.charGlide*.001f);
  smoothedIntensity+=glide*(target-smoothedIntensity);
  float intensity=smoothedIntensity,time=(float)(phase++/sampleRate);
  auto character=juce::jlimit(0,5,p.charType);
  Stereo effected=s;
  switch(character){
   case 0: { // Alien: animated ring/chorus effect
    float mod=.75f+.25f*std::sin(2*pi*(6.f+intensity*18.f)*time);
    effected=s*mod;break;
   }
   case 1: { // Robot: ring-modulated monotone colouring
    float mod=std::sin(2*pi*(75.f+intensity*160.f)*time);
    effected=s*(.5f+.5f*mod);break;
   }
   case 2: {effected.l=std::tanh(s.l*(1+intensity*6));effected.r=std::tanh(s.r*(1+intensity*6));break;}
   case 3: {float mod=1+0.32f*std::sin(2*pi*12*time);effected=s*mod;break;}
   case 4: {filter.l+=.012f*(s.l-filter.l);filter.r+=.012f*(s.r-filter.r);effected=filter*1.5f;break;}
   case 5: {float gain=1+chaos.signedRandom(2,7)*intensity*2;effected=s*gain;break;}
  }
  // Crusher works for any personality and has an optional sample-hold stage.
  const int bits=juce::jlimit(3,16,(int)std::round(p.charCrush));
  float levels=(float)(1<<bits);
  int hold=juce::jlimit(1,16,(int)std::round(1+intensity*12));
  if(++holdCounter>=hold) {holdCounter=0;held=effected;}
  effected=held;
  effected.l=std::round(effected.l*levels)/levels;
  effected.r=std::round(effected.r*levels)/levels;
  // Lightweight tonal colour, not independent formant shifting.
  float filterRate=clamp(.022f*std::pow(2.f,p.charFormant/12.f),.001f,.3f);
  tone.l+=filterRate*(effected.l-tone.l);tone.r+=filterRate*(effected.r-tone.r);
  effected.l=juce::jmap(.3f, tone.l, effected.l);
  effected.r=juce::jmap(.3f, tone.r, effected.r);
  float warble=.5f+.5f*std::sin(2*pi*4.2f*time);
  effected.l*=1.f-p.charDetune*.12f*warble;
  effected.r*=1.f+p.charDetune*.12f*warble;
  return {juce::jmap(intensity,s.l,effected.l),juce::jmap(intensity,s.r,effected.r)};
 }
private:double sampleRate=44100;int64_t phase=0;int holdCounter=0;Stereo held{},filter{},tone{};float smoothedIntensity=0;
};
}
