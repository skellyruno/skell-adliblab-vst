# AdlibLab v0.5 — realtime hardening (Windows VST3 only)

- Per-sample CHARACTER filter coefficient recomputation replaced with parameter-change caching; 6 ms enable/bypass smoothing.
- Sample-rate-normalized onset timing and CHARACTER tone/filter coefficients.
- 4 ms value-continuous tail when 16 MIDI voices require stealing.
- Bounded 64-event UI→audio MIDI SPSC queue; order of note-off / note-on is preserved while dragging keys; overflow drops stale events and releases all notes safely.
- Removed high-frequency DC coloration from the safety soft clip: unity response until 0.85 sample peak, smooth asymptotic ±1 beyond. This is **not** oversampled true-peak protection.
- Sanitizes host input, imported/serialized audio, and nonfinite output; flushes inaudible output to zero.
- Right-click skull for a named four-theme palette popup; explicit tooltip layer and safer ASCII strings for Windows glyph compatibility.
- Headless responsive UI-layout contracts at five supported widths.
- Expanded five-rate ASan/UBSan DSP regression; 48 kHz allocation/CPU report; Windows VST3 only CI.

Limitations: CPU benchmark does not substitute actual FL Studio validation; formant preservation and spectral envelope morphing are not yet full high-quality implementations.
- THROW per-repeat panning is cached; no trig in the audio-rate wet loop.
- Preview keyboard releases notes when dragged outside or destroyed.
- Added portable state flushing for inaudible recursive tails; regression verifies no subnormal output/envelopes after silence without host FTZ/DAZ.
- UI bypass now dims all module controls while preserving readable title, power switch and drag grip, updated immediately on click and on host automation via the editor timer.
- Eliminated Windows MSVC deprecation/shadowing warnings: MIDI metadata iterator, identity transform, and host playhead naming. GitHub Actions checkout/upload upgraded to Node-24-based v5.
- Windows artifact now stores the .vst3 folder directly in the GitHub artifact, eliminating nested/double ZIP extraction.
