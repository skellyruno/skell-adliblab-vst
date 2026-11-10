#pragma once
#include "EngineTypes.h"
namespace adlib {
// Three inexpensive energy bands act as a coarse spectral-flux detector.
class OnsetDetector {
public:
 void prepare(double sampleRate) {
  sr=juce::jmax(8000.0,sampleRate);
  // Original voicing was tuned at 44.1 kHz; preserve its response in seconds.
  const auto alpha=[this](double a){return (float)(1.0-std::pow(1.0-a,44100.0/sr));};
  fastAlpha=alpha(.075);slowAlpha=alpha(.0006);
  band1Alpha=alpha(.08);band2Alpha=alpha(.008);
  slow=fast=lp1=lp2=previous1=previous2=previous3=0;cooldown=0;
 }
 Events tick(float input) {
  auto a=std::abs(input);
  fast+=fastAlpha*(a-fast);
  slow+=slowAlpha*(a-slow);
  lp1+=band1Alpha*(input-lp1);lp2+=band2Alpha*(input-lp2);
  const float v1=std::abs(lp2),v2=std::abs(lp1-lp2),v3=std::abs(input-lp1);
  float flux=juce::jmax(0.0f,v1-previous1)+juce::jmax(0.0f,v2-previous2)+juce::jmax(0.0f,v3-previous3);
  previous1=v1;previous2=v2;previous3=v3;
  bool hit=(cooldown==0 && fast>0.008f && (fast>slow*2.0f+0.007f || flux>0.07f));
  if(hit) cooldown=(int)(0.085*sr);else if(cooldown>0)--cooldown;
  Events e; e.hit=hit;e.envelope=fast;return e;
 }
private:double sr=44100;float fastAlpha=.075f,slowAlpha=.0006f,band1Alpha=.08f,band2Alpha=.008f;float slow=0,fast=0,lp1=0,lp2=0,previous1=0,previous2=0,previous3=0;int cooldown=0;
};
}
