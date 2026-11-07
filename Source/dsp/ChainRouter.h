#pragma once
#include "EngineTypes.h"
#include "StackEngine.h"
#include "CharacterEngine.h"
#include "ThrowEngine.h"
#include "MeltEngine.h"
namespace adlib {
class ChainRouter {
public:
 void prepare(double sr){sampleRate=sr;stack.prepare(sr);character.prepare(sr);throwEngine.prepare(sr);melt.prepare(sr);fade=1.f;transition=0;}
 void setOrder(const std::array<int,4>& newOrder) {
  std::array<bool,4>seen{};for(int id:newOrder){if(id<0||id>=4||seen[(size_t)id])return;seen[(size_t)id]=true;}
  if(newOrder!=order){pendingOrder=newOrder;if(transition!=1)transition=1;}
 }
 Stereo process(Stereo s,const Params&p,const Events&e,const ChaosEngine& chaos,const ClipBank::Clip* sample){
  const Stereo bypass=s;
  Params mod=p;
  // CHAOS changes per-hit settings across every module without touching host automation.
  mod.stackSpread=clamp(p.stackSpread*(1.f+p.chaos*chaos.signedRandom(1,11)*.4f),0,1200);
  mod.charIntensity=clamp(p.charIntensity+p.chaos*chaos.signedRandom(2,12)*.25f,0,1);
  mod.throwFeedback=clamp(p.throwFeedback+p.chaos*chaos.signedRandom(3,13)*.18f,0,.99f);
  mod.meltAmount=clamp(p.meltAmount+p.chaos*chaos.signedRandom(4,14)*.2f,0,1);
  for(auto id:order){
   switch(id){
    case 0:s=stack.tick(s,mod,chaos,e.trackedHz);break;
    case 1:s=character.tick(s,mod,chaos);break;
    case 2:s=throwEngine.tick(s,mod,e);break;
    case 3:s=melt.tick(s,mod,sample,e.trackedHz,e.envelope);break;
    default:break;
   }
  }
  // Two-stage transition: fade wet->dry (15ms) while old route runs,
  // switch module order at dry, then fade dry->wet (15ms) on new route.
  // Avoids route-switch discontinuities without processing the graph twice.
  const float rate=(float)(1.0/(sampleRate*.015));
  if(transition==1){fade=juce::jmax(0.f,fade-rate);if(fade<=0){order=pendingOrder;transition=2;}}
  else if(transition==2){fade=juce::jmin(1.f,fade+rate);if(fade>=1)transition=0;}
  return {juce::jmap(fade,bypass.l,s.l),juce::jmap(fade,bypass.r,s.r)};
 }
 StackEngine stack;
 ThrowEngine throwEngine;
private:double sampleRate=44100;std::array<int,4>order{{0,1,2,3}},pendingOrder{{0,1,2,3}};float fade=1.f;int transition=0;
 CharacterEngine character;MeltEngine melt;
};
}
