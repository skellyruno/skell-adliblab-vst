#pragma once
// Click-resistant global wet/bypass ramp. No allocation, timers, locks or
// JUCE dependencies on the audio thread. Targets only change once per block.
#include <algorithm>
#include <cmath>

namespace adlib {
class GlobalBypassFader final {
public:
 void prepare(double sampleRate, bool enabled) noexcept {
  const auto safeRate=std::isfinite(sampleRate)&&sampleRate>1.0?sampleRate:44100.0;
  step=static_cast<float>(1.0/(safeRate*0.015)); // 15ms to fully toggle.
  current=target=enabled?1.f:0.f;
 }
 void setEnabled(bool enabled) noexcept {target=enabled?1.f:0.f;}
 float next() noexcept {
  if(current<target)current=std::min(current+step,target);
  else if(current>target)current=std::max(current-step,target);
  return current;
 }
 float blend(float dry,float wet) noexcept {
  // Caller supplies next() exactly once per stereo sample.
  return dry+(wet-dry)*current;
 }
 float position() const noexcept {return current;}
private:
 float current=1.f,target=1.f,step=1.f;
};
} // namespace adlib
