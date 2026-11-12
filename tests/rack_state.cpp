#include "../Source/dsp/RackState.h"
#include <cassert>
#include <thread>
#include <atomic>
#include <cstdio>
int main(){
 adlib::RackState rack;using T=adlib::RackType;
 assert((rack.load()==adlib::RackTypes{{T::Stack,T::Character,T::Throw,T::Chop}}));
 assert(rack.replace(0,T::Motion));assert(rack.replace(3,T::Stack));
 assert(rack.replace(1,T::Empty));
 assert((rack.load()==adlib::RackTypes{{T::Motion,T::Empty,T::Throw,T::Stack}}));
 assert(!rack.store({T::Stack,T::Stack,T::Chop,T::Motion}));
 std::atomic<bool>done{false},bad{false};
 std::thread reader([&]{while(!done.load()){auto a=rack.load();unsigned seen=0;
   for(auto t:a){const int n=(int)t;if(n<0||n>8)bad=true;
   if(n>=1&&n<=3){if(seen&(1u<<n))bad=true;seen|=1u<<n;}}}});
 for(int i=0;i<250000;++i){
  if(i%2)rack.store({T::Empty,T::Chop,T::Motion,T::Stack});
  else rack.store({T::Stack,T::Character,T::Throw,T::Saturate});
 }
 done=true;reader.join();assert(!bad);
 puts("RackState PASS: replacement, legacy uniqueness, concurrent snapshots");
}
