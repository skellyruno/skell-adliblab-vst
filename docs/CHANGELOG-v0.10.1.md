# AdlibLab v0.10.1 — polished four-slot rack

**Visible UI findings addressed from real v0.10 screenshots**
- CHOP, MOTION, FILTER and SATURATE no longer share a generic wave illustration. Their new type-specific shapes respond to their control settings (presentation previews, not live audio capture).
- Use the empty lower third of the four-slot module panels for short readable effect explanations without adding knobs or changing the existing panel geometry.
- Enlarge and clarify the little remove-X control with a vector glyph and an accessible tooltip.
- Add hover and keyboard Return/Space activation to empty + slots.
- Make the drag ghost and insertion bar use the colour of the moving physical slot, not always the STACK colour.

**DSP hardening**
- Shared and legacy slot enable flags now transition via an allocation-free 12 ms dry/wet ramp.
- Add a regression for bypassing an audible CHOP closed gate and re-enabling, repeated at five sample rates.
- Keep automation IDs, state compatibility (legacy MELT loading), and the existing SkellCore pin unchanged.

**Build contract** Windows x64 VST3 only, no Standalone or macOS artifacts. Real JUCE renderer is development-only.
**Verification** Windows build, DSP safety/sanitizer tests and four-theme screenshots must pass before merging.
