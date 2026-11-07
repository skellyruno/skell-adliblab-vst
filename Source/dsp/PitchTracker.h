#pragma once
#include "EngineTypes.h"
namespace adlib {
// Lightweight YIN-style pitch tracker with cumulative-mean normalization.
// Detects the FIRST convincing periodicity instead of selecting the absolute
// difference minimum (which would prefer octave-down subharmonics).
// Fixed 4096 sample ring, fixed 201-bin work array, no allocations in tick().
class PitchTracker {
public:
 void prepare(double sampleRate) {
  sr=sampleRate;ring.fill(0);difference.fill(0);idx=counter=0;frequency=0;
 }
 void tick(float in) {
  ring[(size_t)idx]=in;
  idx=(idx+1)&4095;
  if(++counter<512)return;
  counter=0;
  const int step=juce::jmax(1,(int)std::round(sr/12000.));
  const int minLag=juce::jmax(5,(int)std::floor(sr/(1000.0*step)));
  const int maxLag=juce::jmin(200,(int)std::ceil(sr/(75.0*step)));
  constexpr int window=128;
  float energy=0;
  for(int j=0;j<window;++j){const float x=ring[(size_t)((idx-1-j*step)&4095)];energy+=x*x;}
  if(energy<.0004f){frequency*=.8f;return;}
  float cumulative=0,best=1000;
  int bestLag=-1,firstValley=-1;
  // Difference and cumulative mean normalized difference, including low lags
  // in the normalization so comparisons are not biased toward long periods.
  for(int lag=1;lag<=maxLag;++lag){
   float error=0;
   for(int j=0;j<window;++j){
    const int a=(idx-1-j*step)&4095;
    const int b=(a-lag*step)&4095;
    const float delta=ring[(size_t)a]-ring[(size_t)b];
    error+=delta*delta;
   }
   cumulative+=error;
   difference[(size_t)lag]=error*(float)lag/(cumulative+1.e-12f);
   if(lag>minLag){
    const float previous=difference[(size_t)(lag-1)];
    if(firstValley<0 && previous<.18f && previous<difference[(size_t)lag] && previous<=difference[(size_t)(lag-2)])
     firstValley=lag-1;
    if(difference[(size_t)lag]<best){best=difference[(size_t)lag];bestLag=lag;}
   }
  }
  const int picked=firstValley>0?firstValley:(best<.22f?bestLag:-1);
  if(picked<=0){frequency*=.92f;return;}
  float refined=(float)picked;
  if(picked>1&&picked<maxLag){
   const float a=difference[(size_t)(picked-1)],b=difference[(size_t)picked],c=difference[(size_t)(picked+1)];
   const float curve=a-2*b+c;
   if(std::abs(curve)>1.e-7f)refined+=clamp(.5f*(a-c)/curve,-.5f,.5f);
  }
  float hertz=(float)(sr/(refined*step));
  if(std::isfinite(hertz)&&hertz>=70&&hertz<=1050)frequency=hertz;
 }
 float getHz() const {return frequency;}
private:
 double sr=44100;
 std::array<float,4096>ring{};
 std::array<float,201>difference{};
 int idx=0,counter=0;
 float frequency=0;
};
}
