#include "../Source/dsp/ChainRouter.h"
#include "../Source/dsp/OnsetDetector.h"
#include "../Source/dsp/PhraseTracker.h"
#include "../Source/dsp/PitchTracker.h"
#include <cassert>
#include <iostream>
#include <algorithm>

static void runEngine(double rate) {
 adlib::ChainRouter router;
 adlib::ChaosEngine chaos;
 adlib::OnsetDetector onset;
 adlib::PhraseTracker phrase;
 adlib::PitchTracker pitch;
 router.prepare(rate);onset.prepare(rate);phrase.prepare(rate);pitch.prepare(rate);
 adlib::Params params;
 params.stackSpread=175;
 params.stackScaleLock=true;
 params.stackKey=7; // G major
 params.stackVoices=8;
 float peak=0;
 bool sawOnset=false;
 int throwSamples=0;
 const int frames=(int)(rate*1.6);
 for(int i=0;i<frames;++i){
  const float in=(i%((int)(rate*.4))<(int)(rate*.19))?
    std::sin(2.f*adlib::pi*(float)i*196.f/(float)rate)*.20f:0.f;
  pitch.tick(in);
  auto e=onset.tick(in);phrase.tick(e);e.trackedHz=pitch.getHz();
  sawOnset|=e.hit;
  // Dynamic reorder, re-roll and automation stress without allocation in tick.
  if(i==(int)(rate*.3)){router.setOrder({2,0,1,3});chaos.reRoll(false);}
  if(i==(int)(rate*.8)){router.setOrder({1,3,2,0});chaos.reRoll(false);}
  if(i==(int)(rate*1.1)){params.throwPitchPerRepeat=7;params.throwFeedback=.82f;params.throwFilterStart=250;params.throwFilterEnd=9000;}
  auto s=router.process({in,in},params,e,chaos,nullptr);
  assert(std::isfinite(s.l)&&std::isfinite(s.r));
  peak=std::max(peak,std::abs(s.l));
  if(router.throwEngine.activity()>0)++throwSamples;
 }
 assert(sawOnset && peak>.001f && peak<5.f && throwSamples>0);
 std::cout << "  "<<rate<<"Hz: peak="<<peak<<" throw samples="<<throwSamples<<"\n";
}

static void testClipBank() {
 adlib::Params p;
 adlib::ClipBank bank;
 bank.prepare(44100);
 for(int i=0;i<8;++i){
  auto clip=std::make_unique<adlib::ClipBank::Clip>();
  clip->mono.resize(22050,.4f);
  clip->name="test";
  bank.publish(i,std::move(clip));
 }
 bank.noteOn(60,1,p);
 float peak=0;
 for(int i=0;i<128;++i)peak=std::max(peak,bank.tick().l);
 assert(peak>.01f);
 bank.noteOff(60);
 for(int i=0;i<6500;++i)bank.tick();
 assert(std::abs(bank.tick().l)<.00001f); // released, no pop on next block
 // Stress the 16-voice pool and every pad; no slot should overwrite itself only.
 for(int i=0;i<40;++i)bank.noteOn(60+i,.7f,p);
 for(int i=0;i<300;++i)assert(std::isfinite(bank.tick().l));
 for(int i=0;i<40;++i)bank.noteOff(60+i);
 for(int i=0;i<6500;++i)assert(std::isfinite(bank.tick().l));
 bank.clear(0);
 assert(bank.getClip(0)==nullptr);
 // Regression: reverse playback must not terminate on the first sample,
 // whose read index is exactly length - 1 (the rightmost endpoint).
 adlib::ClipBank reverseBank;
 reverseBank.prepare(44100);
 auto reverseClip=std::make_unique<adlib::ClipBank::Clip>();
 reverseClip->mono.resize(4410,.4f);
 reverseBank.publish(1,std::move(reverseClip));
 p.kbAltReverse=true;
 reverseBank.noteOn(61,1.f,p); // odd note selects reverse, pad index 1
 float reversePeak=0;
 for(int i=0;i<128;++i)reversePeak=std::max(reversePeak,std::abs(reverseBank.tick().l));
 assert(reversePeak>.01f);
}


static void testPitchAccuracy() {
 for(double sr:{44100.,48000.,96000.,192000.}){
  for(float expected:{90.f,110.f,146.83f,220.f,440.f,880.f}){
   adlib::PitchTracker tracker;tracker.prepare(sr);
   for(int i=0;i<(int)(sr*.18);++i)
    tracker.tick(.2f*std::sin(2.f*adlib::pi*expected*i/(float)sr));
   float hz=tracker.getHz();
   if(!(std::abs(hz-expected)/expected<.03f)){
    std::cerr<<"Pitch accuracy failed at "<<sr<<" Hz: "<<expected<<" -> "<<hz<<"\n";
    assert(false);
   }
  }
 }
 adlib::PitchTracker tracker;tracker.prepare(48000);
 for(int i=0;i<48000;++i)tracker.tick(.2f*std::sin(2.f*adlib::pi*220*i/48000.f));
 uint32_t seed=12345;
 for(int i=0;i<48000;++i){seed=seed*1664525u+1013904223u;tracker.tick((((seed>>8)&0xFFFFFF)/8388607.5f-1.f)*.2f);}
 assert(tracker.getHz()<10.f); // No persistent false note lock on white noise.
}

int main(){
 for(double sampleRate:{22050.,44100.,48000.,96000.,192000.})runEngine(sampleRate);
 adlib::Vocoder voc;voc.prepare(44100);
 for(int i=0;i<1000;++i)assert(std::isfinite(voc.tick(.3f,.1f,.3f)));
 testClipBank();
 testPitchAccuracy();
 std::cout << "DSP smoke PASS: 5 sample rates, reorder, throw, YIN pitch/noise, MIDI clips\n";
}
