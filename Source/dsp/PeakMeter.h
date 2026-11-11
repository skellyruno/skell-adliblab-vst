#pragma once
#include <algorithm>
#include <cmath>

namespace adlib {
// Audio-thread-only display envelope. Block-size and sample-rate independent
// release; no allocation and no changes to the audio signal or host latency.
class PeakMeter final {
public:
 void prepare(double sampleRate) noexcept {
  rate=std::isfinite(sampleRate)&&sampleRate>=8000.0?sampleRate:48000.0;
  level=0.f;
 }
 float update(float blockPeak,int samples) noexcept {
  if(!std::isfinite(blockPeak))blockPeak=0.f;
  blockPeak=std::clamp(blockPeak,0.f,1.f);
  const int frames=std::max(0,samples);
  const float release=std::exp(-static_cast<float>(frames/(rate*.18)));
  level=std::max(blockPeak,level*release);
  if(level<1.e-10f)level=0.f;
  return level;
 }
 float current()const noexcept{return level;}
private:
 double rate=48000.0;
 float level=0.f;
};
} // namespace adlib
