# Skell AdlibLab v0.4 — Windows VST3 / DSP + UI pass

- **Build**: removed macOS runner and Standalone format. GitHub Actions compiles and packages the Windows **VST3** only; Linux hosts run **DSP tests**, not a plugin build.
- **UI**: live THROW active/ready visualization; MELT sample waveform preview with clearer load/drop affordance; repaint limited to active regions.
- **DSP**: THROW zero-feedback first echo remains audible while later echoes die out. MELT now uses a short carrier loop-edge fade, de-clicked enable/mix automation, smoothed pitch-follow rate, and bounded control-rate pitch calculations.
- **Verification**: Added regression tests for zero-feedback echoes, MELT loop discontinuities and bypass smoothing. Run `python tests/verify_project.py`, plus `g++ -std=c++17 -O1 -g -fsanitize=address,undefined -Itests/stubs tests/dsp_smoke.cpp -o dsp-smoke && ./dsp-smoke`.
- **Limitations**: The Stack pitch engine is still a prototype and does not provide independent formant preservation; MELT is carrier/vocoder synthesis, not true spectral morphing. Soft clip is not a certified true-peak limiter. FL Studio live testing is still required.
