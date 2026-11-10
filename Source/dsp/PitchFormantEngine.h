#pragma once
#include "EngineTypes.h"
namespace adlib {
// Reusable low-allocation, two-grain overlap pitch resampler (not a phase vocoder).
// The formant argument is reserved for the future independent spectral-envelope stage.
class PitchFormantEngine {
public:
 struct Head { float phase=0; };
 void prepare(double sr) {sampleRate=sr;for(auto& c:ring)c.assign(65536,0);writeIndex=0;}
 void push(Stereo s) {ring[0][(size_t)writeIndex]=s.l;ring[1][(size_t)writeIndex]=s.r;writeIndex=(writeIndex+1)&65535;}
 float render(int channel,float ratio,Head& head,float /*formantSt*/=0,float additionalDelay=0) const {
  constexpr int grain=768;
  ratio=clamp(ratio,0.5f,2.0f);
  auto p=head.phase;
  // Head phase is always in [0, grain), so bounded wraps avoid fmod() on the audio thread.
  const float a=p;
  float b=p+grain*.5f; if(b>=grain)b-=grain;
  // fixed 900-sample base delay keeps read heads behind write location.
  const float tA=sampleAt(channel,(float)writeIndex-900.0f-additionalDelay-a);
  const float tB=sampleAt(channel,(float)writeIndex-900.0f-additionalDelay-b);
  float wA=1.f-std::abs(a-grain*.5f)/(grain*.5f);
  float wB=1.f-std::abs(b-grain*.5f)/(grain*.5f);
  head.phase=p+ratio-1.f;
  if(head.phase>=grain)head.phase-=grain;
  else if(head.phase<0)head.phase+=grain;
  return (tA*wA+tB*wB)/(wA+wB+0.00001f);
 }
 static int getBaseDelaySamples() {return 900;}
private:
 float sampleAt(int ch,float p) const {
  int i=(int)std::floor(p);float f=p-(float)i;int a=i&65535,b=(i+1)&65535;
  return juce::jmap(f,ring[(size_t)ch][(size_t)a],ring[(size_t)ch][(size_t)b]);
 }
 double sampleRate=44100;std::array<std::vector<float>,2> ring;int writeIndex=0;
};
}
