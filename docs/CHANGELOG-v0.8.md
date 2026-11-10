# AdlibLab v0.8 development hardening

- Make module-order publishing atomic: a single 8-bit permutation snapshot is read by the audio callback, eliminating transient duplicate/missing modules when dragging the routing chain.
- Crossfade MELT sample replacement over 12 ms without allocating on the audio thread. Keep the old immutable clip alive only as long as the processor's existing owned-sample storage already guarantees.
- Improve eight-pad usability: persistent selected-pad cue, brighter drop target, supported-audio filtering, and sequential import of several files (up to the last pad).
- Make the **Init Patch** command restore real APVTS parameter defaults and default chain order; preserve separately stored audio clips and theme.
- Improve bypass legibility and show **LOADED PATCH** after a patch file is imported.
- Add a concurrent module-order stress test and a sample-swap click regression at 22.05, 44.1, 48, 96 and 192 kHz.
- Keep the Windows x64 VST3-only packaging and independent SkellCore real-editor screenshot workflow intact.

Validation: portable GCC ASan/UBSan DSP smoke, silent-tail denorm probes, zero allocations in the existing worst-case DSP benchmark, and static layout checks passed locally. Full JUCE compile, screenshots, and FL Studio host validation must be reported separately from those portable tests.

Known outstanding work: independent formant-preserving pitch shifting, true-peak limiting, robust undo/preset semantics, native pop-up snapshots, in-host CPU and automation testing, and long-session repeated WAV-replacement memory analysis.

- Fade STACK wet layers smoothly on bypass and re-enable instead of abruptly removing up to eight pitched voices; test both transition edges and the final transparent bypass.

- Display the VST3's actual generated JUCE version in the UI footer instead of the stale hardcoded 0.3 label.

- Clear GCC/Clang renderer warnings at their sources: use tolerance-based float cache invalidation; explicitly initialize Character NaN sentinels; expose JUCE double-precision processBlock overload; fix VoiceMap indentation. Preserve sample-accurate DSP and parameter automation.
