#pragma once
#include "EngineTypes.h"
#include <limits>
namespace adlib {
// 24 band-pass channels, envelope followers and carrier excitation.
class Vocoder {
public:
 void prepare(double sr){
  sampleRate=juce::jmax(8000.0,sr);
  lastSmooth=std::numeric_limits<float>::quiet_NaN();
  for(int i=0;i<24;++i){
   float hi=juce::jmax(300.f,juce::jmin(7000.f,(float)sr*.43f));
   float f=100.f*std::pow(hi/100.f,(i+.5f)/24.f);
   float w=2*pi*f/(float)sr;
   float s=std::sin(w),alpha=s/(2*2.0f),a0=1+alpha;
   auto& b=bands[(size_t)i];b.b0=alpha/a0;b.b1=0;b.b2=-alpha/a0;
   b.a1=-2*std::cos(w)/a0;b.a2=(1-alpha)/a0;b.env=0;b.x1=b.x2=b.y1=b.y2=0;b.cx1=b.cx2=b.cy1=b.cy2=0;
  }
 }
 // Preserve a consistent envelope time constant at every sample rate.
 // Recalculate the power only when the block-level smoothing parameter changes.
 static float envelopeCoefficient(float smooth,double sampleRate) noexcept {
  const float base=clamp(.025f*(1.01f-clamp(smooth,0.f,1.f))+.0004f,.0004f,.04f);
  return 1.f-std::pow(1.f-base,(float)(44100.0/juce::jmax(8000.0,sampleRate)));
 }
 float tick(float mod,float carrier,float smooth) {
  if(!std::isfinite(lastSmooth)||std::abs(lastSmooth-smooth)>1.e-5f){
   lastSmooth=smooth;
   envelopeAlpha=envelopeCoefficient(smooth,sampleRate);
  }
  float total=0;
  for(auto& b:bands){
   float m=b.b0*mod+b.b1*b.x1+b.b2*b.x2-b.a1*b.y1-b.a2*b.y2;
   m=flushTiny(m);
   b.x2=b.x1;b.x1=mod;b.y2=b.y1;b.y1=m;
   float c=b.b0*carrier+b.b1*b.cx1+b.b2*b.cx2-b.a1*b.cy1-b.a2*b.cy2;
   c=flushTiny(c);
   b.cx2=b.cx1;b.cx1=carrier;b.cy2=b.cy1;b.cy1=c;
   b.env=flushTiny(b.env+envelopeAlpha*(std::abs(m)-b.env));
   total+=c*b.env*13.f;
  }
  return std::tanh(total);
 }
private:
 struct Band {float b0=0,b1=0,b2=0,a1=0,a2=0,x1=0,x2=0,y1=0,y2=0,cx1=0,cx2=0,cy1=0,cy2=0,env=0;};
 std::array<Band,24>bands{};
 double sampleRate=44100.0;float lastSmooth=0.f,envelopeAlpha=.02f;
};
}
