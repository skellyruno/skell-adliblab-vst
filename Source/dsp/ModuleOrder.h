#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace adlib {
// A four-module permutation occupies eight bits. Packing prevents the audio
// thread seeing a partially published order during a message-thread drag.
class ModuleOrder final {
public:
 std::array<int,4> load() const noexcept {
  const auto packed=bits.load(std::memory_order_acquire);
  return {{int(packed&3u),int((packed>>2)&3u),int((packed>>4)&3u),int((packed>>6)&3u)}};
 }
 bool store(const std::array<int,4>& order) noexcept {
  unsigned seen=0u,encoded=0u;
  for(unsigned i=0;i<4;++i){
   const int id=order[i];
   if(id<0||id>3||(seen&(1u<<id)))return false;
   seen|=1u<<id;encoded|=static_cast<unsigned>(id)<<(2u*i);
  }
  bits.store(encoded,std::memory_order_release);
  return true;
 }
private:
 std::atomic<unsigned> bits{0xe4u}; // 0,1,2,3
};
}
