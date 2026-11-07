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
 float render(int channel,float ratio,Head& head,float /*formantSt*/=0,int additionalDelay=0) const {
  constexpr int grain=768;
  ratio=clamp(ratio,0.5f,2.0f);
  auto p=head.phase;
  auto a=grainPhase(p),b=grainPhase(p+grain*.5f);
  // fixed 900-sample base delay keeps read heads behind write location.
  const float tA=sampleAt(channel,(float)writeIndex-900.0f-additionalDelay-a);
  const float tB=sampleAt(channel,(float)writeIndex-900.0f-additionalDelay-b);
  float wA=1.f-std::abs(a-grain*.5f)/(grain*.5f);
  float wB=1.f-std::abs(b-grain*.5f)/(grain*.5f);
  head.phase=grainPhase(p+ratio-1.f);
  return (tA*wA+tB*wB)/(wA+wB+0.00001f);
 }
 static int getBaseDelaySamples() {return 900;}
private:
 static float grainPhase(float p) {p=std::fmod(p,768.f);if(p<0)p+=768.f;return p;}
 float sampleAt(int ch,float p) const {
  int i=(int)std::floor(p);float f=p-(float)i;int a=i&65535,b=(i+1)&65535;
  return juce::jmap(f,ring[(size_t)ch][(size_t)a],ring[(size_t)ch][(size_t)b]);
 }
 double sampleRate=44100;std::array<std::vector<float>,2> ring;int writeIndex=0;
};
}
