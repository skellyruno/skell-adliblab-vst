#pragma once
#include "EngineTypes.h"
#include "ClipBank.h"
#include "Vocoder.h"
namespace adlib {
// Sample-carrier vocoder / pitch-follow prototype. No allocation in tick().
// Loop boundaries and enable/mix automation have short de-click envelopes.
class MeltEngine {
public:
 void prepare(double sr){
  sampleRate=juce::jmax(8000.0,sr);vocoder.prepare(sampleRate);
  position=0;lastSample=nullptr;level=0;wetAmount=0;pitchRate=1;
  controlCountdown=0;
  wetSmoothing=1.f-std::exp(-1.f/(float)(sampleRate*.015));
  pitchSmoothing=1.f-std::exp(-1.f/(float)(sampleRate*.025));
 }
 Stereo tick(Stereo s,const Params&p,const ClipBank::Clip* sample,float pitchHz,float inputEnvelope){
  const bool usable=sample!=nullptr&&sample->mono.size()>=2;
  const float target=usable&&p.meltOn?clamp(p.meltAmount,0.f,1.f):0.f;
  wetAmount+=wetSmoothing*(target-wetAmount);
  if(!usable|| (wetAmount<.00001f&&target==0.f))return s;
  if(sample!=lastSample){lastSample=sample;position=0;pitchRate=1;controlCountdown=0;}
  // Pitch tracking may update every sample: expensive exponentials only update
  // every 32 frames, followed by a continuously smoothed playback rate.
  if(--controlCountdown<=0){
   controlCountdown=32;
   float rate=1.f;
   if(p.meltMode==1 && pitchHz>55.f){
    const float rootHz=440.f*std::pow(2.f,(p.meltRoot-69)/12.f);
    rate=juce::jmap(clamp(p.meltFollowPitch,0.f,1.f),1.f,clamp(pitchHz/rootHz,.5f,2.f));
   }
   targetPitchRate=rate;
  }
  pitchRate+=pitchSmoothing*(targetPitchRate-pitchRate);
  const auto length=sample->mono.size();
  const auto i=(size_t)position;
  const float frac=position-(float)i;
  const float a=sample->mono[juce::jmin(i,length-1)];
  const float b=sample->mono[juce::jmin(i+1,length-1)];
  float carrier=juce::jmap(frac,a,b);
  // At each edge fade the carrier to zero before wrapping to prevent a
  // discontinuity from arbitrary loaded samples. 128 frames max at 44.1k.
  const float edge=(float)juce::jmin((size_t)128,juce::jmax((size_t)2,length/8));
  const float envelope=clamp(juce::jmin(position,(float)(length-1)-position)/edge,0.f,1.f);
  carrier*=envelope;
  position+=pitchRate*44100.f/(float)sampleRate;
  if(position>=(float)(length-1))position=0.f;
  level+=(.01f*(1.f-clamp(p.meltSmooth,0.f,1.f))+.0002f)*(inputEnvelope-level);
  const float follow=juce::jmap(clamp(p.meltFollowLoudness,0.f,1.f),1.f,clamp(level*10.f,0.f,1.f));
  const float mono=(s.l+s.r)*.5f;
  float out=0;
  if(p.meltMode==0)out=vocoder.tick(mono,carrier,p.meltSmooth);
  else if(p.meltMode==1)out=carrier*follow;
  else out=vocoder.tick(mono,carrier,p.meltSmooth)*.7f+carrier*follow*.3f;
  return {juce::jmap(wetAmount,s.l,out),juce::jmap(wetAmount,s.r,out)};
 }
private:
 double sampleRate=44100;Vocoder vocoder;const ClipBank::Clip*lastSample=nullptr;
 float position=0,level=0,wetAmount=0,wetSmoothing=.002f;
 float pitchRate=1,targetPitchRate=1,pitchSmoothing=.001f;
 int controlCountdown=0;
};
}
