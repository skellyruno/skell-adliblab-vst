# Skell AdlibLab v0.9.2 — screenshot-grounded UI polish and DSP hardening

- Replaced the rounded skull icon in the header with a sharper, angular original vector skull; left/right click theme behavior unchanged.
- Made module power symbols more legible: larger hit targets, clearer illuminated ON state and muted OFF state, same APVTS automation IDs.
- Strengthened individual module outlines with a dark outer contour and subtle light inner hairline. Retained whole-header dragging and the six-dot grip, removed the redundant tiny DRAG label, and made the footer explain how to reorder.
- Increased bypassed control readability (especially disabled MELT and the 800x500 Ivory theme) while keeping obvious OFF state. Shortened near-1 kHz filter readouts to fit.
- Made 24-band VOCODER follower and MELT loudness follower time constants consistent across host sample rates. Coefficients are cached instead of recalculated per sample.
- Added numerical and audio regression tests at 22.05/44.1/48/96/192 kHz, no allocation in the audio path, no parameter ID changes.
- Shipping target remains Windows x64 VST3 only. Linux screenshots and DSP checks are development CI, not distributable plugins.

No claim of DAW runtime certification: validate FL Studio interaction, audio artifacts and CPU with actual vocals before release.
