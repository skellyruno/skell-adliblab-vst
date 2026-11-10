#include "../Source/dsp/CharacterEngine.h"
#include "../Source/dsp/OnsetDetector.h"
#include "../Source/dsp/PitchTracker.h"
#include "../Source/dsp/Vocoder.h"
#include <cassert>
#include <cmath>
#include <iostream>

static void testDenormalTails(double sr){
 adlib::CharacterEngine charEngine;
 adlib::OnsetDetector onset;
 adlib::PitchTracker tracker;
 adlib::Vocoder vocoder;
 adlib::ChaosEngine chaos;
 adlib::Params params;
 params.charType=4;params.charIntensity=1.f;params.charGlide=0.f;
 charEngine.prepare(sr);onset.prepare(sr);tracker.prepare(sr);vocoder.prepare(sr);
 unsigned subnormals=0;
 const int n=(int)(sr*2.0);
 for(int i=0;i<n;++i){
  const float x=i<200?.3f:0.f;
  const auto c=charEngine.tick({x,x},params,chaos);
  const auto e=onset.tick(x);
  tracker.tick(x);
  const auto v=vocoder.tick(x,x,.5f);
  subnormals+=(std::fpclassify(c.l)==FP_SUBNORMAL);
  subnormals+=(std::fpclassify(c.r)==FP_SUBNORMAL);
  subnormals+=(std::fpclassify(e.envelope)==FP_SUBNORMAL);
  subnormals+=(std::fpclassify(tracker.getHz())==FP_SUBNORMAL);
  subnormals+=(std::fpclassify(v)==FP_SUBNORMAL);
 }
 std::cout<<sr<<" Hz silent-tail subnormal outputs: "<<subnormals<<"\n";
 assert(subnormals==0);
}
int main(){
 for(double sr:{22050.,48000.,192000.})testDenormalTails(sr);
 assert(adlib::sanitizeSample(1.e-39f)==0.f);
}
