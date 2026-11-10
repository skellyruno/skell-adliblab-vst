#pragma once
#include "EngineTypes.h"
#include "ChaosEngine.h"
#include <limits>
namespace adlib {
class CharacterEngine {
public:
 void prepare(double sr) {
  sampleRate=juce::jmax(8000.0,sr);phase=0;held={};holdCounter=0;filter={};tone={};smoothedIntensity=0;
  cachedFormant=std::numeric_limits<float>::quiet_NaN();cachedGlide=std::numeric_limits<float>::quiet_NaN();
  enableAmount=0; enableSmoothing=1.f-std::exp(-1.f/(float)(sampleRate*.006));
  ghostAlpha=1.f-std::pow(1.f-.012f,(float)(44100.0/sampleRate));
 }
 Stereo tick(Stereo s,const Params& p,const ChaosEngine& chaos) {
  const float enableTarget=p.charOn?1.f:0.f;
  enableAmount+=enableSmoothing*(enableTarget-enableAmount);
  if(!p.charOn && enableAmount<.00001f)return s;
  const float target=clamp(p.charIntensity,0,1);
  if(p.charGlide!=cachedGlide) {
   cachedGlide=p.charGlide;
   glideCoefficient=1.f/(1.f+(float)(sampleRate*p.charGlide*.001));
  }
  if(p.charFormant!=cachedFormant) {
   cachedFormant=p.charFormant;
   const float nominal=clamp(.022f*std::pow(2.f,p.charFormant/12.f),.001f,.3f);
   toneCoefficient=1.f-std::pow(1.f-nominal,(float)(44100.0/sampleRate));
  }
  smoothedIntensity+=glideCoefficient*(target-smoothedIntensity);
  const float intensity=smoothedIntensity,time=(float)(phase++/sampleRate);
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
   case 4: {filter.l+=ghostAlpha*(s.l-filter.l);filter.r+=ghostAlpha*(s.r-filter.r);effected=filter*1.5f;break;}
   case 5: {float gain=1+chaos.signedRandom(2,7)*intensity*2;effected=s*gain;break;}
  }
  // Crusher works for any personality and has an optional sample-hold stage.
  const int bits=juce::jlimit(3,16,(int)std::round(p.charCrush));
  float levels=(float)(1<<bits);
  int hold=juce::jlimit(1,juce::jmax(1,(int)std::round(16*sampleRate/44100.)),(int)std::round((1+intensity*12)*sampleRate/44100.));
  if(++holdCounter>=hold) {holdCounter=0;held=effected;}
  effected=held;
  effected.l=std::round(effected.l*levels)/levels;
  effected.r=std::round(effected.r*levels)/levels;
  // Lightweight tonal colour, not independent formant shifting.
  tone.l=flushTiny(tone.l+toneCoefficient*(effected.l-tone.l));
  tone.r=flushTiny(tone.r+toneCoefficient*(effected.r-tone.r));
  effected.l=juce::jmap(.3f, tone.l, effected.l);
  effected.r=juce::jmap(.3f, tone.r, effected.r);
  float warble=.5f+.5f*std::sin(2*pi*4.2f*time);
  effected.l*=1.f-p.charDetune*.12f*warble;
  effected.r*=1.f+p.charDetune*.12f*warble;
  const float wet=clamp(intensity*enableAmount,0.f,1.f);
  return {flushTiny(juce::jmap(wet,s.l,effected.l)),
          flushTiny(juce::jmap(wet,s.r,effected.r))};
 }
private:
 double sampleRate=44100;int64_t phase=0;int holdCounter=0;Stereo held{},filter{},tone{};
 float smoothedIntensity=0,enableAmount=0,enableSmoothing=.003f;
 float cachedFormant=0,cachedGlide=0,toneCoefficient=.022f,glideCoefficient=.01f,ghostAlpha=.012f;
};
}
