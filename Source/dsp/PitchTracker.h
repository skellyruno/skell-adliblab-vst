#pragma once
#include "EngineTypes.h"
namespace adlib {
class PitchTracker {
public:
 void prepare(double sampleRate) { sr=sampleRate;ring.fill(0);idx=counter=0;frequency=0; }
 void tick(float in) {
  ring[(size_t)idx]=in;
  idx=(idx+1)%((int)ring.size());
  if(++counter<1024) return;
  counter=0;
  // Decimated AMDF, voiced signals only. Limited for polyphonic sources.
  const int step=juce::jmax(1,(int)std::round(sr/12000.0));
  const int minLag=juce::jmax(6,(int)(sr/1000.0/step));
  const int maxLag=juce::jmin(200,(int)(sr/75.0/step));
  float best=1.e9f,energy=0;int lagBest=0;
  for(int j=0;j<128;++j) { const int i=(idx-1-j*step+4096*8)%4096;energy+=ring[(size_t)i]*ring[(size_t)i]; }
  if(energy<0.0002f) {frequency*=0.9f;return;}
  for(int lag=minLag;lag<=maxLag;++lag) {
   float error=0;
   for(int j=0;j<128;++j) {
    int a=(idx-1-j*step+4096*8)%4096,b=(a-lag*step+4096*8)%4096;
    error+=std::abs(ring[(size_t)a]-ring[(size_t)b]);
   }
   if(error<best) {best=error;lagBest=lag;}
  }
  if(lagBest>0 && best<16.0f) frequency=(float)(sr/(lagBest*step));
 }
 float getHz() const {return frequency;}
private:double sr=44100;std::array<float,4096> ring{};int idx=0,counter=0;float frequency=0;
};
}
