# AdlibLab v0.10.2 — output deck, power, and echo-skull

## Real JUCE interface
- Replaced duplicated header Mix/Output knobs with one coherent bottom-right **WIDTH · MIX · GAIN** output deck. Mix is the original APVTS wet/dry control; Gain is the original dB output gain. Parameter IDs are preserved for host automation and old presets.
- The top bar now houses an automatable **global power** button immediately before the existing Skell-style Settings gear; the input meter moves left to make room. Settings behavior is unchanged.
- Header brand reads **ADLIBLAB** without a gap, highlights LAB, and places the line **SCULPT THE VOICES AFTER DARK** beneath it.
- Introduced an original JUCE-vector hooded skull with echo-wave markings. Left-click still cycles themes; right-click still selects a specific theme.
- Each effect header now has a visible remove X first and a small illuminated/black bypass dot on its right. Removed the six-dot drag grip. The bare effect header still handles click-to-replace and horizontal drag-to-reorder with the previous scale-invariant drag math.
- Strengthened effect card outlines and tightened the footer instruction.

## DSP
- Added parameter `globalOn` (default ON, host-automatable).
- Global power fades over 15 ms between normal processed output and true unmodified dry audio (bypasses WIDTH, MIX, GAIN, soft clip and clip/effect output); bypass transitions use the same coefficient on both channels and allocate no memory on the audio thread.
- Kept the existing 25 ms Mix and output Gain smoothers, the 12 ms slot bypass ramp, 15 ms rack transitions, parameter IDs, state format, and SkellCore revision.
- Both bypass directions and true unity dry are regression-tested at five sample rates, including invalid-rate handling. No latency is added.

## Verification contract
Windows x64 VST3 only. Run Linux sanitizer DSP tests, Windows JUCE VST3 build, and 100 real-editor JUCE screenshots across four themes before merge. Test actual FL Studio automation, routing, project restore, knob placement and bypass in-host separately.

**Known limitation:** global power is an audible bypass, not a suspended-engine CPU bypass; effects continue updating to preserve tails and avoid discontinuities on reactivation. High-quality formant DSP remains experimental.
