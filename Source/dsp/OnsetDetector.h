#pragma once
#include "EngineTypes.h"
namespace adlib {
// Three inexpensive energy bands act as a coarse spectral-flux detector.
class OnsetDetector {
public:
 void prepare(double sampleRate) { sr=sampleRate; slow=fast=lp1=lp2=previous1=previous2=previous3=0; cooldown=0; }
 Events tick(float input) {
  auto a=std::abs(input);
  fast+=0.075f*(a-fast);
  slow+=0.0006f*(a-slow);
  lp1+=0.08f*(input-lp1);lp2+=0.008f*(input-lp2);
  const float v1=std::abs(lp2),v2=std::abs(lp1-lp2),v3=std::abs(input-lp1);
  float flux=juce::jmax(0.0f,v1-previous1)+juce::jmax(0.0f,v2-previous2)+juce::jmax(0.0f,v3-previous3);
  previous1=v1;previous2=v2;previous3=v3;
  bool hit=(cooldown==0 && fast>0.008f && (fast>slow*2.0f+0.007f || flux>0.07f));
  if(hit) cooldown=(int)(0.085*sr);else if(cooldown>0)--cooldown;
  Events e; e.hit=hit;e.envelope=fast;return e;
 }
private:double sr=44100;float slow=0,fast=0,lp1=0,lp2=0,previous1=0,previous2=0,previous3=0;int cooldown=0;
};
}
