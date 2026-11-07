#pragma once
#include "EngineTypes.h"
namespace adlib {
class ChaosEngine {
public:
 void reRoll(bool locked) {if(!locked) ++seed;}
 float signedRandom(int voice, int salt=0) const {
  uint32_t n=seed+(uint32_t)voice*747796405u+(uint32_t)salt*2891336453u;
  n=(n^(n>>16))*2246822519u;n=(n^(n>>13))*3266489917u;n^=n>>16;
  return (float)(n&0xFFFFFF)/8388607.5f-1.0f;
 }
 uint32_t getSeed() const {return seed;}
 void setSeed(uint32_t newSeed) {seed=newSeed;}
private:uint32_t seed=0xaabbccddU;
};
}
