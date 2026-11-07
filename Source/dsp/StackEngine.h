#pragma once
#include "EngineTypes.h"
#include "PitchFormantEngine.h"
#include "ChaosEngine.h"
#include <limits>
namespace adlib {
class StackEngine {
public:
 void prepare(double sr) {
  sampleRate=sr;pitch.prepare(sr);
  for(auto& v:heads)for(auto& h:v)h={};
  for(auto& v:cache){v.detune=std::numeric_limits<float>::infinity();v.pan=std::numeric_limits<float>::infinity();}
  cachedTrackedHz=0;
  if(!initialized)for(int i=0;i<8;i++){
   pans[(size_t)i].store((i-3.5f)/4.f);offsets[(size_t)i].store((i-3.5f)/4.f);
  }
  initialized=true;
 }
 void setVoicePosition(int index,float pan,float pitchOffset) {
  if(juce::isPositiveAndBelow(index,8)){pans[(size_t)index].store(clamp(pan,-1,1));offsets[(size_t)index].store(clamp(pitchOffset,-1,1));}
 }
 float getVoicePan(int i) const {return pans[(size_t)juce::jlimit(0,7,i)].load();}
 float getVoicePitch(int i) const {return offsets[(size_t)juce::jlimit(0,7,i)].load();}
 Stereo tick(Stereo s,const Params& p,const ChaosEngine& chaos,float trackedHz) {
  pitch.push(s);
  if(!p.stackOn)return s;
  Stereo acc=s*.7f;
  const int voices=juce::jlimit(2,8,p.stackVoices);
  const float amount=clamp(p.chaos*p.stackChaosAmt,0,1);
  if(trackedHz!=cachedTrackedHz){
   cachedTrackedHz=trackedHz;
   trackedMidi=trackedHz>55.f?69.f+12.f*std::log2(trackedHz/440.f):0.f;
  }
  for(int v=0;v<voices;++v){
   float detune=(offsets[(size_t)v].load()+chaos.signedRandom(v,1)*amount*.6f)*p.stackSpread;
   if(p.stackScaleLock && trackedHz>55.f)
    detune=(nearestScaleMidi(trackedMidi+detune/100.f,p.stackKey,p.stackScale)-trackedMidi)*100.f;
   float pan=clamp(pans[(size_t)v].load()*p.stackWidth+chaos.signedRandom(v,3)*amount*.14f,-1,1);
   auto& c=cache[(size_t)v];
   if(detune!=c.detune){c.detune=detune;c.ratio=std::pow(2.f,detune/1200.f);}
   if(pan!=c.pan){c.pan=pan;c.panL=equalPowerPanL(pan);c.panR=equalPowerPanR(pan);}
   const int delay=(int)((double)p.stackTime*sampleRate*.001*(v+1)/(double)voices);
   const auto left=pitch.render(0,c.ratio,heads[(size_t)v][0],p.stackFormant,delay);
   const auto right=pitch.render(1,c.ratio,heads[(size_t)v][1],p.stackFormant,delay);
   const float mono=(left+right)*.5f;
   const float level=.48f/voices;
   acc.l+=mono*c.panL*level;
   acc.r+=mono*c.panR*level;
  }
  return acc;
 }
private:
 static float nearestScaleMidi(float target,int key,int scale){
  constexpr bool major[12]={true,false,true,false,true,true,false,true,false,true,false,true};
  constexpr bool minor[12]={true,false,true,true,false,true,false,true,true,false,true,false};
  constexpr bool pent[12] ={true,false,true,false,true,false,false,true,false,true,false,false};
  if(scale==2)return std::round(target); // Chromatic
  float best=std::round(target),distance=1.e9f;
  int center=(int)std::round(target);
  for(int midi=center-3;midi<=center+3;++midi){
   int degree=((midi-key)%12+12)%12;
   bool included=scale==1?minor[degree]:scale==3?pent[degree]:major[degree];
   if(included && std::abs(target-midi)<distance){distance=std::abs(target-midi);best=(float)midi;}
  }
  return best;
 }
 struct Cached {float detune=0,ratio=1,pan=0,panL=0,panR=0;};
 bool initialized=false;double sampleRate=44100;PitchFormantEngine pitch;
 float cachedTrackedHz=0,trackedMidi=0;
 std::array<Cached,8>cache{};
 std::array<std::array<PitchFormantEngine::Head,2>,8> heads{};
 std::array<std::atomic<float>,8> pans{},offsets{};
};
}
