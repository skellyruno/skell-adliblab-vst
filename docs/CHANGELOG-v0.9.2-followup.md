# AdlibLab v0.9.2 follow-up: Balanced STACK defaults and MELT CPU cache
- New instances place the first two STACK voices symmetrically about center and the third at center; additional voices continue alternating across the stereo field. Saved voice positions and automation IDs are preserved.
- The MELT loudness coefficient now recomputes only when its smoothing control actually changes, reducing pow() calls on long sessions at constant settings.
- Added regression verifying balanced factory voices and preservation across prepareToPlay calls.
- Release targets remain Windows VST3 only; real editor visual regression and DSP tests run separately.
