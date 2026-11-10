#include "../Source/dsp/ChainRouter.h"
#include "../Source/dsp/OnsetDetector.h"
#include "../Source/dsp/PhraseTracker.h"
#include "../Source/dsp/PitchTracker.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <limits>

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

// THROW regression: dry vocal must not be ducked when no echoes are audible,
// and a manual throw must respect the first tempo-synced division.
static void testThrowTimingAndDuck(){
 adlib::ThrowEngine engine;
 adlib::Params p;
 p.throwMode=2;p.throwDiv=2;p.throwDuck=1.f;p.throwFeedback=.8f;
 engine.prepare(48000.);
 for(int i=0;i<26000;++i){
  const auto in=.18f*std::sin(2.f*adlib::pi*220.f*i/48000.f);
  const auto out=engine.tick({in,in},p,{});
  assert(std::abs(out.l-in)<1.e-6f); // no throw = zero attenuation
 }
 engine.trigger(p);
 float maxBefore=0,maxAfter=0;
 const int delay=6000; // 1/16 note at 120 BPM = 6000 samples
 for(int i=0;i<13000;++i){
  auto out=engine.tick({0,0},p,{});
  if(i<delay-1)maxBefore=std::max(maxBefore,std::abs(out.l));
  else maxAfter=std::max(maxAfter,std::abs(out.l));
 }
 assert(maxBefore<1.e-7f);
 assert(maxAfter>.002f);
}

// Feedback controls subsequent repeats; zero feedback must keep the first echo.
static void testThrowZeroFeedback(){
 adlib::ThrowEngine engine; adlib::Params p;
 engine.prepare(48000); p.throwMode=2; p.throwDiv=2;
 p.throwFeedback=0.f; p.throwRepeats=4; p.throwDuck=0.f;
 for(int i=0;i<14000;++i)engine.tick({i<1000?.5f:0.f,i<1000?.5f:0.f},p,{});
 engine.trigger(p);
 float first=0,second=0;
 for(int i=0;i<17000;++i){
  auto out=engine.tick({},p,{});
  if(i<6000)continue;
  if(i<12000)first=std::max(first,std::abs(out.l));
  else second=std::max(second,std::abs(out.l));
 }
 assert(first>.0001f);assert(second<.00001f);
}

// Arbitrary non-zero end points must not cause a carrier snap at loop boundaries.
static void testMeltLoopAndBypass(){
 adlib::MeltEngine melt;adlib::Params p;
 melt.prepare(44100);p.meltOn=true;p.meltMode=1;p.meltAmount=1.f;
 p.meltFollowLoudness=0.f;p.meltFollowPitch=0.f;
 adlib::ClipBank::Clip sample;
 sample.mono.resize(441);
 for(int i=0;i<441;++i)sample.mono[(size_t)i]=(float)i/440.f;
 float prev=0,maxStep=0;
 for(int i=0;i<5000;++i){auto out=melt.tick({},p,&sample,0,1.f);
  if(i>1500)maxStep=std::max(maxStep,std::abs(out.l-prev));
  prev=out.l;
 }
 assert(maxStep<.055f);
 p.meltOn=false;
 for(int i=0;i<5400;++i){auto out=melt.tick({.2f,.2f},p,&sample,0,1.f);
  assert(std::isfinite(out.l));
  if(i==5399)assert(std::abs(out.l-.2f)<.0003f);
 }
}

static void testSeparatePhraseEvents(){
 adlib::PhraseTracker tracker;
 tracker.prepare(48000);
 int syllables=0,words=0;
 for(int i=0;i<24000;++i){
  adlib::Events e;
  // Two syllables separated by an 80ms gap, then a 160ms word end.
  e.envelope=i<5000?.1f:i<9000?0.f:i<15500?.1f:0.f;
  tracker.tick(e);
  syllables+=e.syllableEnd;words+=e.wordEnd;
 }
 assert(syllables==2);
 assert(words==1);
}


static void testSafetyLimiterAndCorruptInput(){
 for(float x:{-.85f,-.2f,0.f,.2f,.85f})
  assert(std::abs(adlib::softLimitSample(x)-x)<1.e-6f);
 float previous=adlib::softLimitSample(0.f);
 for(int i=1;i<=20000;++i){
  const float x=(float)i/100.f;
  const float y=adlib::softLimitSample(x);
  assert(std::isfinite(y)&&y>=previous&&y<=1.f);
  previous=y;
 }
 assert(adlib::sanitizeSample(std::numeric_limits<float>::infinity())==0.f);
 assert(adlib::sanitizeSample(std::numeric_limits<float>::quiet_NaN())==0.f);
 assert(adlib::softLimitSample(-100.f)>=-1.f);
}

// A MIDI voice stolen under heavy polyphony must retain the previous sample
// and fade its tail, not abruptly snap to silence before the new attack.
static void testClipVoiceStealClick(){
 adlib::ClipBank bank;bank.prepare(48000);
 adlib::Params p;
 auto clip=std::make_unique<adlib::ClipBank::Clip>();
 clip->mono.assign(48000,.6f);
 bank.publish(0,std::move(clip));
 for(int i=0;i<16;++i)bank.noteOn(60,1.f,p);
 float previous=0;
 for(int i=0;i<1500;++i)previous=bank.tick().l;
 bank.noteOn(60,1.f,p); // forced steal of a loud, sustained slot
 const float next=bank.tick().l;
 assert(std::abs(next-previous)<.04f);
 for(int i=0;i<2000;++i)assert(std::isfinite(bank.tick().l));
 bank.releaseAll();
 for(int i=0;i<10000;++i)assert(std::isfinite(bank.tick().l));
}

// Envelope time constants should represent the same time in milliseconds
// irrespective of whether the host chooses 22.05, 48 or 192 kHz.
static void testOnsetTimeConstants(){
 float reference=-1.f;
 for(double sr:{22050.,44100.,48000.,96000.,192000.}){
  adlib::OnsetDetector onset;onset.prepare(sr);
  float level=0;
  for(int i=0;i<(int)(sr*.002);++i)level=onset.tick(.2f).envelope;
  if(reference<0)reference=level;
  else assert(std::abs(level-reference)<.006f);
 }
}

static void testCharacterBypassClick(){
 adlib::CharacterEngine effect;adlib::ChaosEngine chaos;adlib::Params p;
 effect.prepare(48000.);
 p.charType=1;p.charIntensity=1.f;p.charGlide=0;
 p.charOn=false;
 for(int i=0;i<300;++i)assert(std::abs(effect.tick({.12f,.12f},p,chaos).l-.12f)<.0001f);
 p.charOn=true;
 const auto initial=effect.tick({.12f,.12f},p,chaos);
 assert(std::abs(initial.l-.12f)<.008f);
 for(int i=0;i<3000;++i)effect.tick({.12f,.12f},p,chaos);
 const auto before=effect.tick({.12f,.12f},p,chaos);
 p.charOn=false;
 const auto after=effect.tick({.12f,.12f},p,chaos);
 assert(std::abs(after.l-before.l)<.012f);
 for(int i=0;i<5000;++i)effect.tick({.12f,.12f},p,chaos);
 assert(std::abs(effect.tick({.12f,.12f},p,chaos).l-.12f)<.001f);
}

int main(){
 for(double sampleRate:{22050.,44100.,48000.,96000.,192000.})runEngine(sampleRate);
 testThrowTimingAndDuck();
 testThrowZeroFeedback();
 testMeltLoopAndBypass();
 testSeparatePhraseEvents();
 adlib::Vocoder voc;voc.prepare(44100);
 for(int i=0;i<1000;++i)assert(std::isfinite(voc.tick(.3f,.1f,.3f)));
 testClipBank();
 testClipVoiceStealClick();
 testOnsetTimeConstants();
 testCharacterBypassClick();
 testSafetyLimiterAndCorruptInput();
 testPitchAccuracy();
 std::cout << "DSP smoke PASS: 5 sample rates, reorder, throw, YIN pitch/noise, MIDI clips\n";
}
