# Shared Guided Tour polish (AdlibLab 0.10 development)

- Pin a SkellCore revision with compact, styled BACK/SKIP/NEXT navigation. OmniChoir remains unchanged.
- Render two real Guided Tour scenes (800x500 and 1280x800) for each of four skins.
- Assert navigation hit-box containment, no overlap, and compact dimensions in the real JUCE editor.
- Fix older screenshot CI count: 76 rendered scenes + one contact sheet = 77 PNGs.
- Correct scene-mode collision so the Guided Tour and effect picker are separate states.
- No changes to plugin automation IDs, preset format, or audio processing. Windows VST3 only.
