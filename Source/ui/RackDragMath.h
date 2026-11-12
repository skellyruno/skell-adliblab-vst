#pragma once
// All drag deltas passed here use the 1280px *design-space* coordinates of
// JUCE MouseEvent::position, never physical screen pixels. This keeps rack
// previews and the eventual drop in agreement at every editor zoom level.
#include <algorithm>

namespace adlib {
inline int rackDragDestination(int from, int designDelta) noexcept {
 const long long delta = designDelta;
 const long long distance = delta < 0 ? -delta : delta;
 if (from < 0 || from >= 4 || distance < 36) return from;
 const long long steps = std::max(1LL, (distance + 156) / 312);
 return static_cast<int>(std::clamp(static_cast<long long>(from) + (delta > 0 ? steps : -steps), 0LL, 3LL));
}
} // namespace adlib
