#include "../Source/ui/RackDragMath.h"
#include <cassert>
#include <cstdio>
#include <limits>

int main() {
 using adlib::rackDragDestination;
 assert(rackDragDestination(1, 0) == 1);
 assert(rackDragDestination(1, 35) == 1);
 assert(rackDragDestination(1, -35) == 1);
 assert(rackDragDestination(1, 36) == 2);
 assert(rackDragDestination(2, -36) == 1);
 assert(rackDragDestination(0, -1000) == 0);
 assert(rackDragDestination(3, 1000) == 3);
 assert(rackDragDestination(0, 468) == 2);
 assert(rackDragDestination(0, 780) == 3);
 assert(rackDragDestination(-1, 100) == -1);
 assert(rackDragDestination(4, 100) == 4);
 assert(rackDragDestination(0, std::numeric_limits<int>::max()) == 3);
 assert(rackDragDestination(3, std::numeric_limits<int>::min()) == 0);
 for (int from = 0; from < 4; ++from) {
  int previous = -1;
  for (int delta = -1200; delta <= 1200; ++delta) {
   const int dest = rackDragDestination(from, delta);
   assert(dest >= 0 && dest < 4);
   assert(dest >= previous); // Drag distance is monotonic in design pixels.
   previous = dest;
   if (delta >= -35 && delta <= 35) assert(dest == from);
  }
 }
 std::puts("Rack drag PASS: preview/drop parity, thresholds and bounds");
}
