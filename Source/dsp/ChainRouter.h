#pragma once
#include "EngineTypes.h"
#include "RackState.h"
#include "StackEngine.h"
#include "CharacterEngine.h"
#include "ThrowEngine.h"
#include "MeltEngine.h"
#include <SkellCore/AdlibFX.h>
namespace adlib {
class ChainRouter {
public:
 void prepare(double sr){
  sampleRate=sr;stack.prepare(sr);character.prepare(sr);throwEngine.prepare(sr);melt.prepare(sr);
  for(auto& processor:shared)processor.prepare(sr);
  fade=1.f;transition=0;order=pendingOrder={{0,1,2,3}};
  types=pendingTypes={{RackType::Stack,RackType::Character,RackType::Throw,RackType::Chop}};
 }
 void setOrder(const std::array<int,4>& next) noexcept {
  std::array<bool,4> seen{};
  for(int slot:next){if(slot<0||slot>=4||seen[(size_t)slot])return;seen[(size_t)slot]=true;}
  if(next!=pendingOrder){pendingOrder=next;if(transition==0)transition=1;}
 }
 void setRackTypes(const RackTypes& next) noexcept {
  if(next!=pendingTypes){pendingTypes=next;if(transition==0)transition=1;}
 }
 Stereo process(Stereo s,const Params&p,const Events&e,const ChaosEngine& chaos,const ClipBank::Clip* sample){
  const Stereo bypass=s;Params mod=p;
  mod.stackSpread=clamp(p.stackSpread*(1.f+p.chaos*chaos.signedRandom(1,11)*.4f),0,1200);
  mod.charIntensity=clamp(p.charIntensity+p.chaos*chaos.signedRandom(2,12)*.25f,0,1);
  mod.throwFeedback=clamp(p.throwFeedback+p.chaos*chaos.signedRandom(3,13)*.18f,0,.99f);
  mod.meltAmount=clamp(p.meltAmount+p.chaos*chaos.signedRandom(4,14)*.2f,0,1);
  for(int slot:order){
   if(!p.rackOn[(size_t)slot])continue;
   switch(types[(size_t)slot]){
    case RackType::Stack:s=stack.tick(s,mod,chaos,e.trackedHz);break;
    case RackType::Character:s=character.tick(s,mod,chaos);break;
    case RackType::Throw:s=throwEngine.tick(s,mod,e);break;
    case RackType::LegacyMelt:s=melt.tick(s,mod,sample,e.trackedHz,e.envelope);break;
    case RackType::Chop:case RackType::Motion:case RackType::Filter:case RackType::Saturate:{
     const auto type=static_cast<skellcore::adlibfx::Type>((int)types[(size_t)slot]-3);
     const skellcore::adlibfx::Parameters fx{type,
      clamp(p.rackAmount[(size_t)slot]+p.chaos*chaos.signedRandom(5+slot,17)*.07f,0.f,1.f),
      p.rackRate[(size_t)slot],p.rackTone[(size_t)slot],p.rackMix[(size_t)slot],p.bpm};
     const auto result=shared[(size_t)slot].process({s.l,s.r},fx);
     s={result.left,result.right};break;
    }
    case RackType::Empty:default:break;
   }
  }
  // 15ms fade to dry while changing the active chain, then fade up.
  const float rate=(float)(1.0/(sampleRate*.015));
  if(transition==1){fade=juce::jmax(0.f,fade-rate);if(fade<=0){
    order=pendingOrder;types=pendingTypes;transition=2;
    for(auto& processor:shared)processor.reset();
  }}
  else if(transition==2){fade=juce::jmin(1.f,fade+rate);if(fade>=1.f)transition=0;}
  return {juce::jmap(fade,bypass.l,s.l),juce::jmap(fade,bypass.r,s.r)};
 }
 StackEngine stack;
 ThrowEngine throwEngine;
private:
 double sampleRate=44100;
 std::array<int,4>order{{0,1,2,3}},pendingOrder{{0,1,2,3}};
 RackTypes types{{RackType::Stack,RackType::Character,RackType::Throw,RackType::Chop}};
 RackTypes pendingTypes{{RackType::Stack,RackType::Character,RackType::Throw,RackType::Chop}};
 std::array<skellcore::adlibfx::Processor,4>shared;
 float fade=1.f;int transition=0;
 CharacterEngine character;MeltEngine melt;
};
}
