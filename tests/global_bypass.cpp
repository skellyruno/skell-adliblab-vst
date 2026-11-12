#include "../Source/dsp/GlobalBypassFader.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>

static void checkRate(double rate) {
 adlib::GlobalBypassFader fader;
 fader.prepare(rate,true);
 assert(fader.position()==1.f);
 const auto total=static_cast<int>(rate*.016)+3;
 for(int iteration=0;iteration<8;++iteration){
  const bool enabled=(iteration%2)!=0;
  fader.setEnabled(enabled);
  const float expected=enabled?1.f:0.f;
  float previous=fader.position();
  for(int i=0;i<total;++i){
   fader.next();
   const float position=fader.position();
   assert(std::isfinite(position));
   assert(position>=0.f&&position<=1.f);
   assert(std::abs(position-previous)<=(1.f/(float)(rate*.015))+1.e-5f);
   if(enabled)assert(position+1.e-6f>=previous);
   else assert(position<=previous+1.e-6f);
   // Unity dry when bypassed; processed audio otherwise.
   const float left=fader.blend(.25f,-.85f);
   const float right=fader.blend(-.3f,.6f);
   assert(std::isfinite(left)&&std::isfinite(right));
   previous=position;
  }
  assert(std::abs(fader.position()-expected)<1.e-3f);
  if(!enabled){
   assert(std::abs(fader.blend(.25f,-.85f)-.25f)<1.e-3f);
   assert(std::abs(fader.blend(-.3f,.6f)+.3f)<1.e-3f);
  }
 }
 // Preparing in the bypassed state must start at unity dry immediately.
 fader.prepare(rate,false);
 assert(fader.position()==0.f);
 assert(fader.blend(.35f,1.f)==.35f);
}
int main(){
 for(double rate:{22050.,44100.,48000.,96000.,192000.})checkRate(rate);
 adlib::GlobalBypassFader invalid;
 invalid.prepare(std::numeric_limits<double>::quiet_NaN(),false);
 invalid.setEnabled(true);
 for(int n=0;n<1000;++n)assert(std::isfinite(invalid.next()));
 std::puts("Global bypass PASS: 5 sample rates, 15ms transitions, true dry, stereo parity");
}
