# AdlibLab 0.9.1 — actual-renderer visual corrections

- Restore the missing ADLIB half of the logo; use explicit glyph-safe geometry rather than deprecated font advance measurements, with a tight ADLIB/LAB gap.
- Replace attachment-controlled slider text boxes with independent, parameter-driven readout labels; normalized values use integer percentages, while time/pitch/filters retain whole-number units.
- Restore the original rotary knob diameter and keep readouts clear of knob labels.
- Fix Clang -Wshadow in formatter and -Wfloat-equal in the new UI control regression.
- Slightly warm/darken Ivory keyboard white keys to reduce glare at minimum size.
- Keep plugin IDs, MIDI behavior, DSP routing and SkellCore development renderer unchanged.

Validation: Windows VST3 / SkellCore screenshots, five-rate DSP safety, CI test pipeline; actual FL Studio runtime remains to be tested.
