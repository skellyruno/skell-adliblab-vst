**Current build target: Windows VST3 only (no Standalone or macOS plugin builds).**

# skell-adliblab-vst

**Skell AdlibLab** — an experimental Windows VST3 adlib designer for JUCE 8 / CMake. Derived from the supplied `skell-vst-template-main(1).zip` starter. The GitHub repository also contains an existing SkellCore foundation; the AdlibLab CMake target preserves and links the reusable SkellCore DSP/UI targets without replacing that shared library.

> **Development status: v0.5 (experimental).** DSP regression checks pass locally and the Windows VST3 builds via GitHub Actions. This is not yet FL Studio host-validated or production certified. Advanced formant preservation and spectral-envelope morphing remain unimplemented.

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

Copy the extracted `.vst3` directory to `C:\Program Files\Common Files\VST3\` and rescan plugins inside FL Studio.

For a private/local JUCE clone:

```bash
cmake -S . -B build -DJUCE_PATH=/absolute/path/to/JUCE
```

GitHub Actions runs a portable sanitizer-backed DSP regression test on Linux, then compiles and publishes the **Windows VST3 artifact only**, packaged for a single extraction (no nested ZIP). Linux is used for testing, not plugin output. There are no macOS builds or Standalone targets.

## MIDI

The plug-in declares `NEEDS_MIDI_INPUT TRUE` and `acceptsMidi() == true`. Some DAWs, including FL Studio, require MIDI routing from a separate controller/channel into an audio effect. Pad mapping wraps notes relative to KB ROOT (`60`/C4 by default). The simplified MIDI playback uses resampling, so tempo and timbre vary with note pitch. `FORMANT LOCK` is exposed but full formant-independent re-pitching is not implemented yet.

## Shared pitch-engine API

`Source/dsp/PitchFormantEngine.h` exposes `prepare(sampleRate)`, `push(Stereo)`, and `render(channel, pitchRatio, Head&, formantSemitones)`. The shared ring is populated once per audio sample; each rendering voice holds independent phase state. To reuse it in another plugin, include `PitchFormantEngine.h`, prepare once outside the callback, call `push()` per input frame, then call `render()` for each desired voice. `formantSemitones` is currently reserved: a shared FFT analysis / identity-phase-locked, envelope-preserving resynthesis stage must replace the granular prototype before claiming genuine independent pitch/formant shifting. Do not assume a full choir-grade engine.

## Improvements through v0.4

- STACK scale-lock respects the selected key and scale; ratio and stereo-pan coefficients are cached when unchanged, reducing trigonometric work.
- THROW interpolates pitched repeat samples, fades playback edges, and caches filter, gain and pitch settings once per repeat/automation change to reduce per-sample CPU.
- MIDI clip bank applies a 2 ms attack, 6 ms release and short end-of-clip fade; 16 simultaneous voices steal in round-robin order.
- On-screen keyboard supports mouse press, drag and note release; the default interface width is 1024 px (still resizable at fixed aspect ratio).
- Pitch detection now uses a voiced/unvoiced-aware YIN-style difference function that selects the first genuine periodicity, avoiding octave-down subharmonic locks; the DSP tests check pitch accuracy and noise rejection.
- Earlier JUCE build errors were corrected: a mixed-type `auto` declaration in LookAndFeel and the missing keyboard processor reference.
- Regression tests exercise 22.05, 44.1, 48, 96 and 192 kHz, chain reordering, throw activity and MIDI clip note release, including ASan/UBSan in CI.

## Validation

Without JUCE installed, you can still test the basic DSP classes at five sample rates using a lightweight **test-only** shim:

```bash
python tests/verify_project.py
g++ -std=c++17 -O2 -Wall -Wextra -Itests/stubs tests/dsp_smoke.cpp -o dsp_smoke
./dsp_smoke
```

**The test shim is not JUCE and cannot validate plugin compilation or host behaviour.** Actual compilation is performed by the Windows GitHub Actions job; audition and load/close tests inside FL Studio remain outstanding.

## Remaining gaps and priorities

1. **Host validation:** test the Windows VST3 in FL Studio, Ableton and REAPER. Validate saved-state restoration and DAW automation.
2. **Audio correctness:** replace granular pitch shifting with one FFT analysis, N identity-phase-locked resynthesis streams, real formant preservation and scale-aware tuning. Report measured wet-path latency and compensate dry/wet and MIDI output correctly.
3. **Realtime hardening:** implement optional true dual-route crossfade for reorder, full sample-accurate MIDI events and an explicit lock-free clip and state synchronization plan; profile worst-case CPU/heap allocations.
4. **Throw refinement:** proper syllable vs word segmentation, a lookahead phrase tracker, non-aliased pitch steps, and repeat-length/velocity tail scaling.
5. **MELT refinement:** STFT spectral envelope morph and a pitch-follow mode with formant control, plus sample start/loop markers.
6. **UI:** original licensed embedded display/mono fonts, 60 Hz minimal repaint + cached glows, clip Rename/Reverse, waveform selection, reordering insertion feedback, factory preset management, complete GUI settings.
7. **Limiter:** substitute a measured oversampled true-peak/lookahead limiter for the current non-lookahead tanh safety clip.

## Audio/data notes

Each loaded sample is limited to 5 s and downsampled to 44.1 kHz mono (220,500 floats maximum). Host state embeds up to 220,500 float samples for each of eight pads plus one MELT sample. This may produce a host preset of several megabytes; avoid using extremely long sample files. Loaded clips are retained until the plugin is destroyed so audio callbacks can read published immutable pointers without locking.

Latest v0.4 improvements: live THROW activity visualization, MELT waveform preview, eased MELT bypass/loop playback, correct first throw when feedback is zero, and Windows-only packaging.

See [v0.4 changelog](docs/CHANGELOG-v0.4.md).

The graphic palette uses the requested colors. UI fonts fall back to system sans-serif and monospace for this revision; fonts have not been bundled.

### v0.5 hardening notes (November 2026)
- Windows x64 VST3 only; the CI DSP safety job runs on Linux but does not distribute Linux plugins.
- UI skull: left-click cycles the four palettes; right-click opens a named theme picker. Interface preserves its 1280×800 aspect ratio at 800–1920 pixels; bypassed module controls visually dim on button clicks or automation changes.
- CHARACTER enable/bypass now crossfades instead of hard switching, and expensive filter coefficients update on changes, not every sample. Onset filters preserve millisecond-scale behavior across sample rates.
- MIDI keyboard gestures use a bounded lock-free event queue to preserve note-off/note-on ordering on glissandos; stolen clip voices carry a 4ms release tail.
- Input/sample validation suppresses non-finite values, output subnormals are zeroed, and captured/imported audio is bounded to ±8 before processing.
- Regression tests include five host rates, sound continuity, allocation checks, and non-gating CPU timing.
- The second hardening pass zeroes inaudible recursive state in Onset, PitchTracker, CHARACTER, THROW, MELT and the Vocoder. Dedicated portable tests report zero subnormal outputs across 22.05, 48 and 192 kHz without host FTZ/DAZ. THROW stereo pan is now cached per repeat.

**Not production-certified:** Independent formant preservation and spectral morphing remain prototype algorithms. A JUCE build proves compilation, not plugin stability or actual CPU usage inside FL Studio. The safety soft clip is not a true-peak limiter, and dry latency remains intentionally zero while effects have their own artistic delays.

The current Windows workflow uses JUCE 8 MIDI metadata iterators instead of deprecated legacy iteration, modern identity transforms, and current Node 24-compatible GitHub Actions checkout/upload versions.


## Permanent SkellCore real-editor visual regression

The development-only executable SkellAdlibLabUIRenderer uses the actual
SkellAdlibLabProcessor and SkellAdlibLabEditor, linked to SkellCore::UIRenderer
pinned to immutable commit 438bc74a86a40bf54cfa8089832a477822c8ccda.
It is not included in the Windows VST3 binary.

The GitHub Actions workflow .github/workflows/ui-screenshots.yml runs on changes
to CMake, Source, Assets, or the renderer. It renders 44 real-editor PNGs and a
contact sheet: Cosmic, Neon, Ivory, and Crimson themes at 800x500, 1024x640,
1280x800, 1600x1000 and 1920x1200, plus bypass, MIDI-selected, and
preset-parameter states at minimum and design sizes. Each scene is validated
for blank frames, mismatched appearance and nested JUCE child bounds.

Reproduce under Linux with the JUCE build requirements and Xvfb installed:

    cmake -S . -B build-renderer -G Ninja -DCMAKE_BUILD_TYPE=Release -DSKELL_ADLIBLAB_BUILD_UI_RENDERER=ON
    cmake --build build-renderer --target SkellAdlibLabUIRenderer --parallel 2
    xvfb-run -a build-renderer/SkellAdlibLabUIRenderer --output ui-screenshots

Known visual-test limitations: native JUCE PopupMenus render in separate
desktop windows, outside the AudioProcessorEditor's component snapshot, so
the current harness does not yet capture those popup pixels or mouse-hover
states. Those need a desktop-window-grab/input test, not a fake screenshot.
Windows VST3 is the only release target.

## Hub-free SkellKey integration (Windows)

AdlibLab now automatically receives SkellKey's confirmed Key and Major/Minor
Scale when the user clicks **SEND TO TUNE** in SkellKey. No Hub, Python, extra
app, internet or extra button is needed. Its existing `stackKey` and
`stackScale` host parameters update on the JUCE message thread; the
`stackScaleLock` toggle stays under the user's control, so this does not
silently enable a new effect. The A4 value is received for future
reference-aware modules, but the present AdlibLab DSP is unchanged.
SkellCore's Windows same-session shared-memory bus holds only the freshest
valid tuning packet and expires crashed senders after five seconds. Existing
VST3/parameter IDs, styles and sound-processing code are preserved.

