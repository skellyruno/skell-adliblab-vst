# AdlibLab v0.9.3 — unified SkellCore and stable visual meters

- Use pinned SkellCore commit b0c79ff75cf7f1ca7e55ad231a9a9013f7c9e6c3 for **all builds** including released Windows VST3 and Linux UI renderer.
- Stop including stale bundled v0.3 and duplicated old separate guided-tour header checkout; preserve old folder only as source provenance.
- Link SkellCore::DSP and SkellCore::UI into the real plugin, SkellCore::UIRenderer into the separate development executable.
- Real processor now uses SkellCore HostTempoClock for finite host BPM handling and sample-based advancement. SkellWorld local tuning and guided tour remain intact.
- Add an allocation-free ~180 ms peak-meter decay shared by input/output displays to smooth block-size-induced meter flicker without altering the audio.
- Add five-sample-rate meter finite/bounds/decay tests to DSP safety CI.
- Windows x64 VST3 only; no macOS/Standalone release artifacts.

Validation: review the Windows build log and all real JUCE UI scenes before merging. Long-session FL Studio host behavior remains unverified.
