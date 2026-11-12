#pragma once
// All drag deltas passed here use the 1280px *design-space* coordinates of
// JUCE MouseEvent::position, never physical screen pixels. This keeps rack
// previews and the eventual drop in agreement at every editor zoom level.
#include <algorithm>
#include <cstdlib>

namespace adlib {
inline int rackDragDestination(int from, int designDelta) noexcept {
 if (from < 0 || from >= 4 || std::abs(designDelta) < 36) return from;
 const int steps = std::max(1, (std::abs(designDelta) + 156) / 312);
 return std::clamp(from + (designDelta > 0 ? steps : -steps), 0, 3);
}
} // namespace adlib
