# skell-adliblab-vst

**Skell AdlibLab** — an experimental adlib-designer VST3/Standalone project for JUCE 8 / CMake. Derived from the supplied `skell-vst-template-main(1).zip` starter. The GitHub repository also contains an existing SkellCore foundation; the AdlibLab CMake target preserves and links the reusable SkellCore DSP/UI targets without replacing that shared library.

> **Development status: v0.2 source prototype, not a verified production VST3.** The DSP smoke test and static parameter checks pass. A real JUCE build, host validation, microphone/vocal listening tests, and CPU profiling have not been completed here. The advanced pitch/formant and spectral-morph features described in the original brief are not yet implemented at production quality.

## Interface & features implemented

- Responsive 1280 × 800 dark interface with original ADLIBLAB wordmark, distinct STACK / CHARACTER / THROW / MELT accent colors, rotary controls, clip pads, a piano keyboard, and orange CHAOS macro. Scales as a fixed-aspect window.
- Drag a module's small handle horizontally and release to reorder the four module panels. The router applies a roughly 30 ms wet-to-dry-to-wet bridge around the order switch, reducing discontinuities (not a dual-router crossfade).
- STACK: 2–8 additive/detuned pitch-processed voices, per-voice draggable pan/pitch positions and time offset, key/scale-aware semitone quantisation, chaos re-roll on detected onsets. Current pitch engine is a two-grain overlapped resampler; NOT a high-quality identity-phase-locked phase vocoder. Independent formant shaping, independent formant spectral shaping is future work.
- CHARACTER: six characters (Alien, Robot, Demon, Cartoon, Ghost, Gremlin) implemented with economical modulation, waveshaping, frequency-dependent colour, glide of intensity, bit-depth reduction, and sample hold. This is a creative-effects baseline, not the full independent-formant set specified.
- THROW: onset/phrase-end detection, four overlapping prerecorded echo voices with cached per-repeat coefficients, interpolated pitch playback and de-clicked edges, host-BPM divisions (120 BPM fallback), repeating with feedback attenuation, per-repeat pitch resampling, reverse and ping-pong with filter sweep. It currently captures the last ~280 ms, with a lower-complexity phrase detector and without a true lookahead. Manual trigger available by button or MIDI (when in Manual mode).
- MELT: load WAV/AIFF/FLAC audio (5 seconds maximum), 24-band filter-bank vocoding, envelope-followed sample re-pitching, and a morph-inspired blend. This is NOT STFT spectral-envelope morphing yet.
- EIGHT CLIP PADS: drag audio onto a pad; right-click to load, clear, audition, or record the next detected phrase; Shift-click also arms recording. Click to audition. Loaded clips can be played from routed MIDI notes with click-free attack/release, fair 16-voice stealing and pass through the chain. Keyboard C3–C5 supports mouse-down, drag-to-new-note and release audition. Recording has a five-second cap and publishes the finished clip from the GUI timer, not the realtime audio callback.
- CHAOS: deterministic seeded re-roll with seed lock, re-roll button, and onset-responsive variations in STACK, CHARACTER, THROW, and MELT.
- APVTS automation: 49 user parameters, with unautomated module order stored in state. Full patch and in-memory WAV clip data are serialized into host state or exported to `.skad` patch files.
- Global wet/dry mix, output gain, stereo width, and an optional safety **soft clip**. The soft clip is not a true-peak limiter.

## Build

JUCE 8.0.12 is fetched by CMake when network access is available. Or supply a local JUCE checkout with `-DJUCE_PATH=...`.

### Windows (MSVC / Visual Studio 2022 or newer)

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel 2
```

VST3 output: `build/SkellAdlibLab_artefacts/Release/VST3/Skell AdlibLab.vst3`
Standalone: `build/SkellAdlibLab_artefacts/Release/Standalone/Skell AdlibLab.exe`

### macOS (Xcode)

```bash
cmake -S . -B build -G Xcode
cmake --build build --config Release --parallel 2
```

VST3 output: `build/SkellAdlibLab_artefacts/Release/VST3/Skell AdlibLab.vst3`
Standalone app: `build/SkellAdlibLab_artefacts/Release/Standalone/Skell AdlibLab.app`

For a private/local JUCE clone:

```bash
cmake -S . -B build -DJUCE_PATH=/absolute/path/to/JUCE
```

GitHub Actions is configured to run a sanitizer-backed DSP regression test before building Windows and macOS VST3/Standalone. The initial repository upload may use a temporary compressed source bundle (due to connector limitations); CI expands it and commits the extracted sources as normal tracked files if Actions has contents-write permission.

## MIDI

The plug-in declares `NEEDS_MIDI_INPUT TRUE` and `acceptsMidi() == true`. Some DAWs, including FL Studio, require MIDI routing from a separate controller/channel into an audio effect. Pad mapping wraps notes relative to KB ROOT (`60`/C4 by default). The simplified MIDI playback uses resampling, so tempo and timbre vary with note pitch. `FORMANT LOCK` is exposed but full formant-independent re-pitching is not implemented yet.

## Shared pitch-engine API

`Source/dsp/PitchFormantEngine.h` exposes `prepare(sampleRate)`, `push(Stereo)`, and `render(channel, pitchRatio, Head&, formantSemitones)`. The shared ring is populated once per audio sample; each rendering voice holds independent phase state. To reuse it in another plugin, include `PitchFormantEngine.h`, prepare once outside the callback, call `push()` per input frame, then call `render()` for each desired voice. `formantSemitones` is currently reserved: a shared FFT analysis / identity-phase-locked, envelope-preserving resynthesis stage must replace the granular prototype before claiming genuine independent pitch/formant shifting. Do not assume a full choir-grade engine.

## v0.2 improvements

- STACK scale-lock respects the selected key and scale; ratio and stereo-pan coefficients are cached when unchanged, reducing trigonometric work.
- THROW interpolates pitched repeat samples, fades playback edges, and caches filter, gain and pitch settings once per repeat/automation change to reduce per-sample CPU.
- MIDI clip bank applies a 2 ms attack, 6 ms release and short end-of-clip fade; 16 simultaneous voices steal in round-robin order.
- On-screen keyboard supports mouse press, drag and note release; the default interface width is 1024 px (still resizable at fixed aspect ratio).
- Regression tests exercise 22.05, 44.1, 48, 96 and 192 kHz, chain reordering, throw activity and MIDI clip note release, including ASan/UBSan in CI.

## Validation

Without JUCE installed, you can still test the basic DSP classes at five sample rates using a lightweight **test-only** shim:

```bash
python tests/verify_project.py
g++ -std=c++17 -O2 -Wall -Wextra -Itests/stubs tests/dsp_smoke.cpp -o dsp_smoke
./dsp_smoke
```

**The test shim is not JUCE and cannot validate plugin compilation or host behaviour.** All real builds use `JUCE_PATH` or fetch JUCE. The complete plugin was NOT compiled or auditioned in the archive-generation environment because network resolution of `github.com` was unavailable and there was no local JUCE source checkout.

## Known v0.1 gaps and priorities

1. **Build/host validation:** compile on Windows/macOS CI, resolve any JUCE 8 API issues, test VST3 in FL Studio, Ableton and REAPER. Validate saved-state restoration and DAW automation.
2. **Audio correctness:** replace granular pitch shifting with one FFT analysis, N identity-phase-locked resynthesis streams, real formant preservation and scale-aware tuning. Report measured wet-path latency and compensate dry/wet and MIDI output correctly.
3. **Realtime hardening:** implement optional true dual-route crossfade for reorder, full sample-accurate MIDI events and an explicit lock-free clip and state synchronization plan; profile worst-case CPU/heap allocations.
4. **Throw refinement:** proper syllable vs word segmentation, a lookahead phrase tracker, non-aliased pitch steps, and repeat-length/velocity tail scaling.
5. **MELT refinement:** STFT spectral envelope morph and a pitch-follow mode with formant control, plus sample start/loop markers.
6. **UI:** original licensed embedded display/mono fonts, 60 Hz minimal repaint + cached glows, clip Rename/Reverse, waveform selection, reordering insertion feedback, factory preset management, complete GUI settings.
7. **Limiter:** substitute a measured oversampled true-peak/lookahead limiter for the current non-lookahead tanh safety clip.

## Audio/data notes

Each loaded sample is limited to 5 s and downsampled to 44.1 kHz mono (220,500 floats maximum). Host state embeds up to 220,500 float samples for each of eight pads plus one MELT sample. This may produce a host preset of several megabytes; avoid using extremely long sample files. Loaded clips are retained until the plugin is destroyed so audio callbacks can read published immutable pointers without locking.

The graphic palette uses the requested colors. UI fonts fall back to system sans-serif and monospace for this revision; fonts have not been bundled.
