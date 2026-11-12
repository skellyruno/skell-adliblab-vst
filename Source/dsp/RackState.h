#pragma once
// Four physical slots packed in one atomic word: coherent audio-thread state.
#include <array>
#include <atomic>
#include <cstdint>
#include <cstddef>
namespace adlib {
enum class RackType : std::uint8_t {
 Empty=0,Stack=1,Character=2,Throw=3,Chop=4,Motion=5,Filter=6,Saturate=7,LegacyMelt=8
};
inline const char* rackTypeName(RackType t) noexcept {
 switch(t){case RackType::Stack:return "STACK";case RackType::Character:return "CHARACTER";
  case RackType::Throw:return "THROW";case RackType::Chop:return "CHOP";
  case RackType::Motion:return "MOTION";case RackType::Filter:return "FILTER";
  case RackType::Saturate:return "SATURATE";case RackType::LegacyMelt:return "MELT (LEGACY)";
  case RackType::Empty:default:return "EMPTY";}
}
using RackTypes=std::array<RackType,4>;
class RackState final {
public:
 RackTypes load() const noexcept {
  const auto value=word.load(std::memory_order_acquire);
  return {{static_cast<RackType>(value&15u),static_cast<RackType>((value>>4)&15u),
           static_cast<RackType>((value>>8)&15u),static_cast<RackType>((value>>12)&15u)}};
 }
 bool store(const RackTypes& types) noexcept {
  std::uint16_t encoded=0;unsigned special=0;
  for(int i=0;i<4;++i){
   const int id=static_cast<int>(types[(std::size_t)i]);
   if(id<0||id>8)return false;
   if(id>=1&&id<=3){const unsigned mask=1u<<id;if((special&mask)!=0)return false;special|=mask;}
   if(id==8){if((special&16u)!=0)return false;special|=16u;}
   encoded|=static_cast<std::uint16_t>(id)<<(4u*static_cast<unsigned>(i));
  }
  word.store(encoded,std::memory_order_release);return true;
 }
 bool replace(int slot,RackType replacement) noexcept {
  if(slot<0||slot>=4)return false;
  const int chosen=static_cast<int>(replacement);
  if(chosen<0||chosen>8)return false;
  auto before=load();
  const auto displaced=before[(std::size_t)slot];
  if((chosen>=1&&chosen<=3)||chosen==8)
   for(int i=0;i<4;++i)if(i!=slot&&before[(std::size_t)i]==replacement)
    before[(std::size_t)i]=displaced;
  before[(std::size_t)slot]=replacement;
  return store(before);
 }
private:
 std::atomic<std::uint16_t> word{0x4321u}; // Stack/Character/Throw/Chop.
};
} // namespace adlib
