#pragma once
#include "EngineTypes.h"
#include "ClipBank.h"
#include "Vocoder.h"
namespace adlib {
class MeltEngine {
public:
 void prepare(double sr){sampleRate=sr;vocoder.prepare(sr);position=0;lastSample=nullptr;level=0;}
 Stereo tick(Stereo s,const Params&p,const ClipBank::Clip* sample,float pitchHz,float inputEnvelope){
  if(!p.meltOn||sample==nullptr||sample->mono.size()<2)return s;
  if(sample!=lastSample){lastSample=sample;position=0;}
  float ratio=1;
  if(p.meltMode==1 && pitchHz>55){
   auto rootHz=440.f*std::pow(2.f,(p.meltRoot-69)/12.f);
   ratio=juce::jmap(clamp(p.meltFollowPitch,0,1),1.f,clamp(pitchHz/rootHz,.5f,2.f));
  }
  auto i=(size_t)position;float f=position-(float)i;
  auto carrier=juce::jmap(f,sample->mono[i%sample->mono.size()],sample->mono[(i+1)%sample->mono.size()]);
  position+=ratio*44100.f/(float)sampleRate;if(position>=sample->mono.size()-1)position=0;
  level+=(.01f*(1-p.meltSmooth)+.0002f)*(inputEnvelope-level);
  float follow=juce::jmap(clamp(p.meltFollowLoudness,0,1),1.f,clamp(level*10.f,0.f,1.f));
  float x=(s.l+s.r)*.5f;
  float out=0;
  if(p.meltMode==0)out=vocoder.tick(x,carrier,p.meltSmooth);
  else if(p.meltMode==1)out=carrier*follow;
  else out=vocoder.tick(x,carrier,p.meltSmooth)*.7f+carrier*follow*.3f;
  return {juce::jmap(p.meltAmount,s.l,out),juce::jmap(p.meltAmount,s.r,out)};
 }
private:double sampleRate=44100;Vocoder vocoder;const ClipBank::Clip*lastSample=nullptr;float position=0,level=0;
};
}
