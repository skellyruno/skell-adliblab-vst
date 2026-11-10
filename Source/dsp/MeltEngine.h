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
  position=0;previousPosition=0;lastSample=nullptr;previousSample=nullptr;crossfadeRemaining=0;level=0;wetAmount=0;pitchRate=targetPitchRate=1;
  controlCountdown=0;
  wetSmoothing=1.f-std::exp(-1.f/(float)(sampleRate*.015));
  pitchSmoothing=1.f-std::exp(-1.f/(float)(sampleRate*.025));
 }
 Stereo tick(Stereo s,const Params&p,const ClipBank::Clip* sample,float pitchHz,float inputEnvelope){
  const bool usable=sample!=nullptr&&sample->mono.size()>=2;
  const float target=usable&&p.meltOn?clamp(p.meltAmount,0.f,1.f):0.f;
  wetAmount=flushTiny(wetAmount+wetSmoothing*(target-wetAmount));
  if(!usable|| (wetAmount<.00001f&&target==0.f))return s;
  if(sample!=lastSample){
   // The old clip is immutable and retained by the processor. Blend the last
   // rendered carrier into a newly loaded sample over 12ms, never allocating
   // or resetting the vocoder from the audio callback.
   previousSample=lastSample;
   previousPosition=position;
   lastSample=sample;
   position=0;pitchRate=targetPitchRate=1;controlCountdown=0;
   crossfadeRemaining=previousSample?juce::jmax(1,(int)(sampleRate*.012)):0;
  }
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
  float carrier=carrierAt(sample,position);
  const float advance=pitchRate*44100.f/(float)sampleRate;
  position+=advance;
  if(position>=(float)(sample->mono.size()-1))position=0.f;
  if(crossfadeRemaining>0&&previousSample!=nullptr){
   const float previous=carrierAt(previousSample,previousPosition);
   previousPosition+=advance;
   if(previousPosition>=(float)(previousSample->mono.size()-1))previousPosition=0.f;
   const auto frames=juce::jmax(1,(int)(sampleRate*.012));
   const float blend=1.f-(float)crossfadeRemaining/(float)frames;
   carrier=juce::jmap(blend,previous,carrier);
   if(--crossfadeRemaining==0)previousSample=nullptr;
  }
  level=flushTiny(level+(.01f*(1.f-clamp(p.meltSmooth,0.f,1.f))+.0002f)*(inputEnvelope-level));
  const float follow=juce::jmap(clamp(p.meltFollowLoudness,0.f,1.f),1.f,clamp(level*10.f,0.f,1.f));
  const float mono=(s.l+s.r)*.5f;
  float out=0;
  if(p.meltMode==0)out=vocoder.tick(mono,carrier,p.meltSmooth);
  else if(p.meltMode==1)out=carrier*follow;
  else out=vocoder.tick(mono,carrier,p.meltSmooth)*.7f+carrier*follow*.3f;
  return {juce::jmap(wetAmount,s.l,out),juce::jmap(wetAmount,s.r,out)};
 }
private:
 static float carrierAt(const ClipBank::Clip*clip,float position) noexcept {
  if(clip==nullptr||clip->mono.size()<2)return 0.f;
  const auto length=clip->mono.size();
  const size_t i=juce::jmin((size_t)position,length-1);
  const float frac=clamp(position-(float)i,0.f,1.f);
  const float a=clip->mono[i], b=clip->mono[juce::jmin(i+1,length-1)];
  // Silence arbitrary loop boundaries rather than snapping between end points.
  const float edge=(float)juce::jmin((size_t)128,juce::jmax((size_t)2,length/8));
  const float envelope=clamp(juce::jmin(position,(float)(length-1)-position)/edge,0.f,1.f);
  return juce::jmap(frac,a,b)*envelope;
 }
 double sampleRate=44100;Vocoder vocoder;
 const ClipBank::Clip*lastSample=nullptr,*previousSample=nullptr;
 float previousPosition=0;
 int crossfadeRemaining=0;
 float position=0,level=0,wetAmount=0,wetSmoothing=.002f;
 float pitchRate=1,targetPitchRate=1,pitchSmoothing=.001f;
 int controlCountdown=0;
};
}
