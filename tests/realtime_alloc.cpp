#include "../Source/dsp/ChainRouter.h"
#include "../Source/dsp/ClipBank.h"
#include "../Source/dsp/PitchTracker.h"
#include "../Source/dsp/OnsetDetector.h"
#include "../Source/dsp/PhraseTracker.h"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <new>
#include <cassert>
static std::atomic<unsigned long long> heapAllocs{0};
void* operator new(std::size_t bytes){
 heapAllocs.fetch_add(1,std::memory_order_relaxed);
 if(void*mem=std::malloc(bytes?bytes:1))return mem;
 throw std::bad_alloc();
}
void* operator new[](std::size_t bytes){return ::operator new(bytes);}
void operator delete(void*ptr)noexcept{std::free(ptr);}
void operator delete[](void*ptr)noexcept{std::free(ptr);}
void operator delete(void*ptr,std::size_t)noexcept{std::free(ptr);}
void operator delete[](void*ptr,std::size_t)noexcept{std::free(ptr);}
int main(){
 constexpr double sr=48000;
 adlib::ChainRouter route;route.prepare(sr);
 adlib::PitchTracker tracker;tracker.prepare(sr);
 adlib::OnsetDetector onset;onset.prepare(sr);
 adlib::PhraseTracker phrase;phrase.prepare(sr);
 adlib::ChaosEngine chaos;
 adlib::ClipBank bank;bank.prepare(sr);
 adlib::Params p; p.stackVoices=8; p.charOn=true;p.meltOn=true;
 auto sample=std::make_unique<adlib::ClipBank::Clip>();
 sample->mono.assign(22050,.16f);bank.publish(0,std::move(sample));
 const auto before=heapAllocs.load(std::memory_order_relaxed);
 auto start=std::chrono::steady_clock::now();
 double peak=0;
 for(int i=0;i<48000;++i){
  if(i==22000){bank.noteOn(60,.7f,p);route.throwEngine.trigger(p);}
  const float input=.15f*std::sin((float)i*adlib::pi*220.f*2.f/(float)sr);
  tracker.tick(input);auto e=onset.tick(input);phrase.tick(e);e.trackedHz=tracker.getHz();
  auto result=route.process({input+bank.tick().l,input},p,e,chaos,bank.getClip(0));
  assert(std::isfinite(result.l)&&std::isfinite(result.r));
  peak=std::max(peak,(double)std::abs(result.l));
 }
 const auto after=heapAllocs.load(std::memory_order_relaxed);
 const auto duration=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 std::cout<<"48kHz, 8 STACK voices + CHARACTER + THROW + MELT: "
          <<(duration*1000)<<" ms to render 1 sec, peak="<<peak
          <<", audio-thread heap allocations="<<(after-before)<<"\n";
 assert(after==before);
 assert(peak<8.0);
}
