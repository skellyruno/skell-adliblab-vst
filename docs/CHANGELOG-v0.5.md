# AdlibLab v0.5 — realtime hardening (Windows VST3 only)

- Per-sample CHARACTER filter coefficient recomputation replaced with parameter-change caching; 6 ms enable/bypass smoothing.
- Sample-rate-normalized onset timing and CHARACTER tone/filter coefficients.
- 4 ms value-continuous tail when 16 MIDI voices require stealing.
- Bounded 64-event UI→audio MIDI SPSC queue; order of note-off / note-on is preserved while dragging keys; overflow releases notes safely.
- Removed high-frequency DC coloration from the safety soft clip: unity response until 0.85 sample peak, smooth asymptotic ±1 beyond. This is **not** oversampled true-peak protection.
- Sanitizes host input, imported/serialized audio, and nonfinite output; flushes inaudible output to zero.
- Right-click skull for a named four-theme palette popup; explicit tooltip layer and safer ASCII strings for Windows glyph compatibility.
- Headless responsive UI-layout contracts at five supported widths.
- Expanded five-rate ASan/UBSan DSP regression; 48 kHz allocation/CPU report; Windows VST3 only CI.

Limitations: CPU benchmark does not substitute actual FL Studio validation; formant preservation and spectral envelope morphing are not yet full high-quality implementations.
