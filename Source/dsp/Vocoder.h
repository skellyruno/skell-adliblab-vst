#pragma once
#include "EngineTypes.h"
namespace adlib {
// 24 band-pass channels, envelope followers and carrier excitation.
class Vocoder {
public:
 void prepare(double sr){
  for(int i=0;i<24;++i){
   float hi=juce::jmax(300.f,juce::jmin(7000.f,(float)sr*.43f));
   float f=100.f*std::pow(hi/100.f,(i+.5f)/24.f);
   float w=2*pi*f/(float)sr;
   float s=std::sin(w),alpha=s/(2*2.0f),a0=1+alpha;
   auto& b=bands[(size_t)i];b.b0=alpha/a0;b.b1=0;b.b2=-alpha/a0;
   b.a1=-2*std::cos(w)/a0;b.a2=(1-alpha)/a0;b.env=0;b.x1=b.x2=b.y1=b.y2=0;b.cx1=b.cx2=b.cy1=b.cy2=0;
  }
 }
 float tick(float mod,float carrier,float smooth) {
  float total=0, alpha=clamp(.025f*(1.01f-smooth)+.0004f,.0004f,.04f);
  for(auto& b:bands){
   float m=b.b0*mod+b.b1*b.x1+b.b2*b.x2-b.a1*b.y1-b.a2*b.y2;
   b.x2=b.x1;b.x1=mod;b.y2=b.y1;b.y1=m;
   float c=b.b0*carrier+b.b1*b.cx1+b.b2*b.cx2-b.a1*b.cy1-b.a2*b.cy2;
   b.cx2=b.cx1;b.cx1=carrier;b.cy2=b.cy1;b.cy1=c;
   b.env+=alpha*(std::abs(m)-b.env);
   total+=c*b.env*13.f;
  }
  return std::tanh(total);
 }
private:
 struct Band {float b0=0,b1=0,b2=0,a1=0,a2=0,x1=0,x2=0,y1=0,y2=0,cx1=0,cx2=0,cy1=0,cy2=0,env=0;};
 std::array<Band,24>bands{};
};
}
