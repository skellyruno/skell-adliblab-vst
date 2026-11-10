#pragma once
#include "EngineTypes.h"
namespace adlib {
// Four overlapping, fixed-allocation tempo-synced throw voices.
// Parameters are cached per repeat to avoid pow/exp calls at audio sample rate.
class ThrowEngine {
public:
 void prepare(double sr) {
  sampleRate=juce::jmax(8000.0,sr);
  duckGain=0;duckAttack=1.f-std::exp(-1.f/(float)(sampleRate*.004));
  duckRelease=1.f-std::exp(-1.f/(float)(sampleRate*.070));
  history.assign((size_t)juce::jmax(4096,(int)(sr*.70)),{});write=0;voiceCursor=0;
  for(auto& v:voices){v.clip.assign((size_t)juce::jmax(2048,(int)(sr*.40)),{});v.active=false;v.length=0;v.position=0;v.tick=0;v.repeat=0;v.low={};v.started=false;}
 }
 void trigger(const Params& p){
  auto& v=voices[(size_t)voiceCursor];voiceCursor=(voiceCursor+1)%(int)voices.size();
  const int length=(int)juce::jmin((size_t)(sampleRate*.28),v.clip.size());
  const int historyLength=(int)history.size();
  for(int i=0;i<length;++i){int pos=(write-length+i+historyLength*2)%historyLength;v.clip[(size_t)i]=history[(size_t)pos];}
  v.active=length>0;v.length=length;v.repeat=0;v.tick=0;v.position=0;v.low={};v.started=false;
  // An invalid transport BPM must never be cast to an integer interval.
  const double bpm=std::isfinite(p.bpm)&&p.bpm>=20.f&&p.bpm<=400.f
    ?(double)p.bpm:120.0;
  v.interval=juce::jmax(1,(int)(sampleRate*60.0/bpm*division(p.throwDiv)));
  updateVoice(v,p);
 }
 Stereo tick(Stereo s,const Params& p,const Events& e){
  history[(size_t)write]=s;write=(write+1)%(int)history.size();
  if(!p.throwOn) {for(auto&v:voices)v.active=false;duckGain=0;return s;}
  if((p.throwMode==0 && e.wordEnd)||(p.throwMode==1 && e.syllableEnd)||(p.throwMode==3 && e.hit)) trigger(p);
  Stereo wet{};
  for(auto& v:voices){
   if(!v.active)continue;
   // A throw is an echo: the first repeat starts after one beat division,
   // not at the instant the word ends.
   if(++v.tick>=v.interval){
    v.tick=0;
    if(v.started){++v.repeat;if(v.repeat>=p.throwRepeats){v.active=false;continue;}}
    else v.started=true;
    v.position=0;v.low={};updateVoice(v,p);
   }
   if(!v.started)continue;
   // Recompute if host automation changes pitch, repeat count, feedback, or filter.
   if(std::abs(v.pitch-p.throwPitchPerRepeat)>1.e-4f||std::abs(v.feedback-p.throwFeedback)>1.e-5f
      ||std::abs(v.filterStart-p.throwFilterStart)>.05f||std::abs(v.filterEnd-p.throwFilterEnd)>.05f
      ||v.repeats!=p.throwRepeats||v.pingPong!=p.throwPingPong)
    updateVoice(v,p);
   if(v.position<(float)v.length-1){
    float pos=p.throwReverse?(float)(v.length-1)-v.position:v.position;
    int i=juce::jlimit(0,v.length-2,(int)pos);
    float frac=clamp(pos-(float)i,0,1);
    Stereo a=v.clip[(size_t)i],b=v.clip[(size_t)i+1];
    Stereo sample{juce::jmap(frac,a.l,b.l),juce::jmap(frac,a.r,b.r)};
    v.low.l=flushTiny(v.low.l+v.alpha*(sample.l-v.low.l));
    v.low.r=flushTiny(v.low.r+v.alpha*(sample.r-v.low.r));
    const float fadeSamples=(float)(sampleRate*.004);
    const float edge=juce::jmin(1.f,juce::jmin(v.position,v.length-1.f-v.position)/fadeSamples);
    const float gain=v.gain*juce::jmax(0.f,edge);
    wet.l+=v.low.l*gain*v.panL;
    wet.r+=v.low.r*gain*v.panR;
    v.position+=v.pitchRatio;
   }
  }
  // Duck only in response to audible throws, never continuously on dry voice.
  const float activity=clamp((std::abs(wet.l)+std::abs(wet.r))*.75f,0.f,1.f);
  const float desiredDuck=clamp(p.throwDuck,0,1)*activity*.60f;
  const float smoothing=desiredDuck>duckGain?duckAttack:duckRelease;
  duckGain=flushTiny(duckGain+smoothing*(desiredDuck-duckGain));
  return {s.l*(1-duckGain)+wet.l,s.r*(1-duckGain)+wet.r};
 }
 float activity() const {float a=0;for(const auto& v:voices)if(v.active)a+=1;return a/(float)voices.size();}
private:
 static float division(int v){constexpr float divisions[]={1.0f,.5f,.25f,.75f,1.f/3.f,.125f,.0625f};return divisions[juce::jlimit(0,6,v)];}
 struct Voice{
  std::vector<Stereo> clip;bool active=false,started=false;int length=0,repeat=0,tick=0,interval=1;
  float position=0,pitchRatio=1,alpha=.2f,gain=0;
  float pitch=999,feedback=-1,filterStart=-1,filterEnd=-1;int repeats=-1;
  bool pingPong=false;float panL=1.f,panR=1.f;
  Stereo low{};
 };
 void updateVoice(Voice&v,const Params&p){
  v.pitch=p.throwPitchPerRepeat;v.feedback=p.throwFeedback;v.filterStart=p.throwFilterStart;v.filterEnd=p.throwFilterEnd;v.repeats=p.throwRepeats;
  v.pingPong=p.throwPingPong;
  const float pan=v.pingPong?(v.repeat%2==0?-.85f:.85f):0.f;
  v.panL=equalPowerPanL(pan);v.panR=equalPowerPanR(pan);
  v.pitchRatio=std::pow(2.f,clamp(v.pitch,-12,12)*(float)v.repeat/12.f);
  v.gain=std::pow(clamp(v.feedback,0,.99f),(float)v.repeat)*.75f;
  const float t=(float)v.repeat/(float)juce::jmax(1,p.throwRepeats-1);
  const float start=juce::jmax(20.f,p.throwFilterStart),end=juce::jmax(20.f,p.throwFilterEnd);
  const float frequency=std::exp(juce::jmap(t,std::log(start),std::log(end)));
  v.alpha=clamp(1.f-std::exp((float)(-2.0*pi*frequency/sampleRate)),.001f,.995f);
 }
 double sampleRate=44100;float duckGain=0,duckAttack=.01f,duckRelease=.001f;
 std::vector<Stereo>history;int write=0,voiceCursor=0;std::array<Voice,4>voices;
};
}
