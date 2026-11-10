# AdlibLab 0.8.1 — de-clicking and STACK map readability

- Fade THROW repetitions before short tempo intervals reset playback, avoiding a hard cut when a 1/64 repeat is shorter than the source clip. Existing throw timing, repeat pitch and automation IDs are unchanged.
- Preserve MELT's normal ~15 ms wet fade when a loaded carrier is cleared, instead of cutting directly to dry. Retained immutable sample lifetime remains managed by the existing processor.
- Improve STACK VoiceMap marker contrast and number voices 1–8 to make panning and pitch editing clearer across all four themes. Preserve the existing drag behavior and positions.
- Add multi-rate regressions for short THROW divisions and MELT sample removal, run alongside existing sanitizer, denormal and allocation tests.

Validation: portable GCC ASan/UBSan smoke at 22.05, 44.1, 48, 96, 192 kHz; Clang -Wall -Wextra -Werror compilation; denormal probe (22.05, 48, 192 kHz); allocation/CPU smoke (48 kHz). Full Windows VST3 and actual JUCE screenshot checks are in GitHub CI and must be assessed separately. No FL Studio runtime certification is claimed.
