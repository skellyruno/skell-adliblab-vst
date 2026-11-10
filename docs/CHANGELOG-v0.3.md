# Skell AdlibLab — UI + DSP polish (v0.3 development)

## UI

- Reworked the header into a consistent **SKELL / ADLIBLAB** wordmark and a vector skull button that cycles four theme palettes: Cosmic, Neon, Ivory, Crimson.
- Palettes are **editor-instance-local** and the selection is stored in the plugin's APVTS ValueTree for project/preset restore; no extra automation parameters were added.
- Removed unsupported Unicode glyphs from the preset, menu and MIDI controls to avoid missing-font squares in FL Studio.
- Unified panel, button, knob, border, label, meter and clip-pad colour treatments; improved contrast and spacing.
- Knob readouts now show sensible compact values such as `12k`, `250Hz`, `30ct`, `50ms` or `7st` instead of clipped multi-digit numbers.
- Redrew the 25-note MIDI keyboard with properly aligned white and overlapping black keys and C labels.
- Voice map draws only the active voice count; CHAOS and output sections have clearer separate surfaces.

## DSP

- THROW waits for the selected host-tempo interval before the first repeat instead of emitting an immediate echo.
- THROW dry-path ducking now follows audible echo output with 4ms attack/70ms release, instead of attenuating the dry input whenever the module is enabled.
- PhraseTracker now emits a short-gap syllable end separately from a longer-gap word end, with rearming after successive syllables. This is still an envelope heuristic, not lexical word recognition.
- STACK wet voices no longer replace 30% of the direct vocal signal by default; ratio and delay automation use a 12ms one-pole smoothing stage.
- Added DSP tests for idle ducking, delayed repeat onset and separate syllable / word events.

## Verification and limitations

- Portable DSP regression tests pass at 22.05, 44.1, 48, 96 and 192 kHz with GCC, including ASan/UBSan.
- This is **not** a true independent formant shifter. `PitchFormantEngine` still uses a dual-grain prototype and the supplied formant parameter is not implemented by its spectral engine.
- `MELT` remains an experimental vocoder/carrier cross synthesis. Safety clipping is not certified true-peak limiting.
- GitHub Actions still needs to compile and package Windows and macOS VST3; successful CI is not proof of host playback stability.
- No standalone UI screenshots or FL Studio load/close/automation stress test were available here.
