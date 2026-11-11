# AdlibLab 0.9 — FL Studio feedback / interface behavior

- The module header (except power button) is draggable; drag grip retained.
- Bypass checkmarks replaced by vector power switches.
- Presets now open as a styled in-editor overlay, so SkellCore can render it.
- Factory presets reset all parameters to defaults before applying full audible patches. Robot Incoming retained and enhanced; Demon Doubles and Cartoon Punch added.
- SHUFFLE now randomizes host-notified DSP parameters (voices, pitch spread, character, throw, CHAOS) and rerolls the seed, rather than only rerolling CHAOS at potentially zero intensity.
- The preset name persists in the existing ValueTree state without changing APVTS IDs or clip storage.
- Settings uses a vector gear; unused quality-mode controls are no longer presented as functional.
- ADLIB + LAB spacing tightened; Ivory darkened; normalized knobs now use whole percent readouts.
- SkellCore real-editor CI checks factory preset and shuffle behavior and captures the in-editor preset overlay at minimum/design sizes.
- No change to Windows VST3-only output or legacy plugin automation IDs.

Remaining: audio-quality selection is currently a reserved parameter only; this release does not claim lower-latency processing for that setting. Real FL Studio runtime validation remains necessary.
