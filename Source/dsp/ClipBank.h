#pragma once
#include "EngineTypes.h"
#include <memory>
namespace adlib {
// Immutable published clips; pointers stay alive until the owning processor is destroyed.
// noteOn/noteOff/tick execute on the audio thread; publish/clear are message-thread operations.
class ClipBank {
public:
 struct Clip {std::vector<float> mono;juce::String name;int root=60;};
 ClipBank() {for(auto& p:pads)p.store(nullptr);}
 void prepare(double sr){sampleRate=sr;for(auto& v:active)v={};nextVoice=0;}
 void publish(int index,std::unique_ptr<Clip> clip){
  if(!juce::isPositiveAndBelow(index,8)||clip==nullptr)return;
  auto* address=clip.get();owned.push_back(std::move(clip));pads[(size_t)index].store(address,std::memory_order_release);
 }
 const Clip* getClip(int index)const {return juce::isPositiveAndBelow(index,8)?pads[(size_t)index].load(std::memory_order_acquire):nullptr;}
 void clear(int i){if(juce::isPositiveAndBelow(i,8))pads[(size_t)i].store(nullptr,std::memory_order_release);}
 void noteOn(int midi,float velocity,const Params&p){
  const int pad=((midi-p.kbRoot)%8+8)%8;
  const auto* clip=getClip(pad);
  if(clip==nullptr||clip->mono.size()<2)return;
  // Prefer idle voices; when all 16 are active, steal in a fair rotating order.
  int target=-1;
  for(int j=0;j<(int)active.size();++j){int i=(nextVoice+j)%(int)active.size();if(active[(size_t)i].clip==nullptr){target=i;break;}}
  if(target<0)target=nextVoice;
  nextVoice=(target+1)%(int)active.size();
  const float noteRatio=std::pow(2.f,(midi-p.kbRoot)/12.f);
  active[(size_t)target]={clip,0,noteRatio*44100.f/(float)sampleRate,p.kbVelIntensity?clamp(velocity,0,1):1.f,p.kbAltReverse&&(midi%2!=0),midi,0,false,
   (float)(1.0/(sampleRate*(p.kbVelTail?(0.012+0.11*clamp(velocity,0,1)):0.045)))};
 }
 void noteOff(int midi){for(auto& a:active)if(a.clip!=nullptr&&a.midi==midi)a.releasing=true;}
 Stereo tick() {
  float sum=0;
  const float attackStep=(float)(1.0/(sampleRate*.002));
  for(auto& a:active){
   if(a.clip==nullptr)continue;
   const auto length=a.clip->mono.size();
   const float pos=a.reverse?(float)(length-1)-a.position:a.position;
   // The first sample of a reversed clip is exactly the final sample.
   // Clamp the upper interpolation index rather than discarding that voice.
   if(pos<0||pos>(float)(length-1)){a.clip=nullptr;continue;}
   const auto i=(size_t)pos;
   const auto next=juce::jmin(length-1,i+1);
   const float frac=pos-(float)i;
   const float sample=juce::jmap(frac,a.clip->mono[i],a.clip->mono[next]);
   // Both note-off and natural clip end are de-clicked without allocations.
   a.fade=clamp(a.fade+(a.releasing?-a.releaseStep:attackStep),0,1);
   const float remaining=(float)length-1.f-a.position;
   const float tail=clamp(remaining/(float)(sampleRate*.004*a.ratio),0,1);
   sum+=sample*a.velocity*a.fade*tail*.55f;
   a.position+=a.ratio;
   if(a.releasing&&a.fade<=0)a.clip=nullptr;
  }
  return {sum,sum};
 }
private:
 struct Active {const Clip* clip=nullptr;float position=0,ratio=1,velocity=0;bool reverse=false;int midi=-1;float fade=0;bool releasing=false;float releaseStep=.01f;};
 std::array<std::atomic<const Clip*>,8>pads;
 std::vector<std::unique_ptr<Clip>>owned;
 std::array<Active,16>active{};double sampleRate=44100;int nextVoice=0;
};
}
