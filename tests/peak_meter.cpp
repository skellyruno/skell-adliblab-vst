#include "../Source/dsp/PeakMeter.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
int main(){
 for(const double sr:{22050.,44100.,48000.,96000.,192000.}){
  adlib::PeakMeter meter;meter.prepare(sr);
  assert(meter.update(1.f,128)==1.f);
  constexpr int block=128;
  const int steps=(int)std::round(sr*.18/block);
  for(int i=0;i<steps;++i){
   const float v=meter.update(0.f,block);
   assert(std::isfinite(v));
  }
  assert(std::abs(meter.current()-std::exp(-1.f))<.004f);
  const float prior=meter.current();
  assert(meter.update(std::numeric_limits<float>::quiet_NaN(),0)==prior);
  assert(meter.update(std::numeric_limits<float>::infinity(),128)<=prior);
  assert(meter.update(2.f,128)==1.f);
  meter.prepare(sr);assert(meter.current()==0.f);
 }
 puts("Peak meter PASS: 5 sample rates, finite values, 180ms decay and reset");
}
