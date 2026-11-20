# Skell AdlibLab — Commercial Release Gates

**Last reviewed:** 2026-11-19  
**Decision:** **NOT CLEARED — experimental owner-test preview only**  
**Scope:** release provenance, licensing, distribution and acceptance gates. This is an engineering inventory, not legal advice, and does not grant or change any licence.

This checklist is deliberately separate from the product README and the preview installer. A passing build or a reversible local installer does not make the product commercially releasable.

## 1. Protected source and artifact baseline

| Item | Verified identity |
|---|---|
| Default-branch source baseline | `main` at [`aae755dc746fa3989552b0bdf522258c7702bf71`](https://github.com/skellyruno/skell-adliblab-vst/commit/aae755dc746fa3989552b0bdf522258c7702bf71), 2026-11-14 |
| Installer preview PR | [PR #11](https://github.com/skellyruno/skell-adliblab-vst/pull/11), draft; head `398f69b1d2373934e5943937e689e142fbbd36b9`, based on `release/adliblab-integrate-verifier` at `7187275fd2f5aee8ee5c58ec03caa78f8ca4d2f9` |
| Windows VST3 build | [Workflow #44365584373](https://github.com/skellyruno/skell-adliblab-vst/actions/runs/44365584373), exact PR head `398f69b1d2373934e5943937e689e142fbbd36b9`, success |
| Preserved Windows preview artifact | [Artifact #14456259709](https://github.com/skellyruno/skell-adliblab-vst/actions/runs/44365584373/artifacts/14456259709), 7,900,747 bytes; archive SHA-256 `dbc501bf8900b4a593691a1b45a9c12a5d891aa9e832b23c5da0a274208e3866`; expires 2026-12-18 |
| VST3 module in that artifact | `Skell AdlibLab.vst3/Contents/x86_64-win/Skell AdlibLab.vst3`, 7,839,232 bytes; SHA-256 `ef6c2ff15eb2a46e31627c1158f63a12b23a4a52c60da42e09f09d35ec69a6fa` |
| JUCE dependency | JUCE 8.0.12, resolved commit `29396c22c93392d6738e021b83196283d6e4d850` |
| SkellCore dependency | `eb56702153c4373799a4e8e2663e0f158c0115a6` |
| UI evidence | [Actual JUCE UI run #44365584546](https://github.com/skellyruno/skell-adliblab-vst/actions/runs/44365584546), same source head; [artifact #14456771291](https://github.com/skellyruno/skell-adliblab-vst/actions/runs/44365584546/artifacts/14456771291) |

The downloaded VST3 archive passed ZIP CRC validation; all 13 `SHA256SUMS.txt` entries matched; strict JSON parsing passed for `moduleinfo.json`; the package contained one flat VST3 bundle, one AMD64 PE32+ module, two unique VST3 class IDs, and no Standalone executable. The module hash above was read from the package manifest and independently matched against the extracted module.

The existing preview artifact is preserved as evidence; this checklist does not overwrite it, alter its source branch, or replace a known-good binary. No stable GitHub release or release tag was present at review time.

## 2. Licensing and source-provenance clearance

| Component | Observed evidence | Commercial gate |
|---|---|---|
| JUCE 8.0.12 | The pinned JUCE [`LICENSE.md`](https://github.com/juce-framework/JUCE/blob/8.0.12/LICENSE.md) states that JUCE modules are dual-licensed under AGPLv3 or the commercial JUCE licence. The preview package includes this notice. | **OPEN:** record the chosen AGPLv3-compliance or valid commercial-licence route for the shipped product. No evidence of the applicable licence decision is recorded in this repository. |
| VST3 SDK bundled with JUCE | JUCE's licence inventory identifies the bundled VST3 SDK as MIT-licensed. | Preserve the applicable notice in the shipped notices bundle. |
| SkellCore | AdlibLab pins the immutable SkellCore commit above. The SkellCore repository has no root `LICENSE` file or repository SPDX licence metadata; its provenance document identifies several imported DSP components. | **OPEN:** confirm the distribution rights for the complete pinned SkellCore source set. The SkellyTune notice is not a blanket licence for all SkellCore code. |
| SkellyTune DSP | The pinned SkellCore provenance identifies the imported SkellyTune components as MIT and the preview package includes `SkellyTune-MIT.txt`. | Preserve the MIT copyright and permission notice for the covered files. Confirm the exact covered-file set against the pinned source. |
| DeEsser phrase detector | SkellCore provenance identifies a namespaced copy of `PhraseAwareDetector.h` from [skell-deesser-vst](https://github.com/skellyruno/skell-deesser-vst/blob/main/Source/dsp/PhraseAwareDetector.h), blob `7337cfcb1ec299128336f7d24fbae4965af6fe2f`. The upstream repository currently exposes no root licence file or SPDX licence metadata. | **OPEN:** confirm original authorship/ownership and commercial redistribution rights, or obtain and record an explicit licence/permission. |
| Skell Master mastering engine | SkellCore provenance identifies a namespaced copy of `MasteringEngine.h` from [skell-master-vst](https://github.com/skellyruno/skell-master-vst/blob/main/Source/DSP/MasteringEngine.h), blob `a2b2264815e07647422864bcdd75a17b63695e50`. The upstream repository currently exposes no root licence file or SPDX licence metadata. | **OPEN:** confirm original authorship/ownership and commercial redistribution rights, or obtain and record an explicit licence/permission. |
| AdlibLab product source and assets | The AdlibLab repository has no root `LICENSE` file and GitHub reports no repository licence. | **OPEN:** owner must choose and approve the product's source/distribution licence. Public visibility is not a permissive licence. Do not add a licence grant without owner approval. |

References:
- [JUCE 8.0.12 licence inventory](https://github.com/juce-framework/JUCE/blob/8.0.12/LICENSE.md)
- [JUCE 8 licence terms](https://juce.com/legal/juce-8-licence/)
- [AGPLv3 text](https://www.gnu.org/licenses/agpl-3.0.en.html)
- [Pinned SkellCore source provenance](https://github.com/skellyruno/skellcore-vst-template/blob/eb56702153c4373799a4e8e2663e0f158c0115a6/SkellCore/SOURCE_PROVENANCE.md)
- [SkellyTune MIT notice](https://github.com/skellyruno/skelltune-mini/blob/main/LICENSE)

The preview archive contains `JUCE-LICENSE.md`, `SkellCore-SOURCE-PROVENANCE.md`, and `SkellyTune-MIT.txt`. A provenance record is not itself a licence grant for the copied DeEsser or Master files.

## 3. Distribution and installer status

- The GitHub repository is **public**. Treat its Actions artifacts as public distribution material, not confidential/private downloads.
- The preview package's `PREVIEW-ONLY.txt` describes an “unsigned private per-user installer”. “Per-user” describes the local install target; it does not make a public-repository artifact private.
- PR #11 adds a manually invoked, unsigned per-user installer with `-WhatIf`, explicit confirmation, checksum and bundle validation, verified update backup, and rollback. Its Windows integration assertions passed in the exact-head run.
- This helper is not a signed commercial installer, an automatic updater, an activation system, or a privacy boundary.
- The VST3 factory metadata has blank vendor URL and e-mail fields. Approved support/contact details are still needed.
- No code-signing evidence, approved EULA/customer terms, commercial licence decision, or genuinely private distribution channel is recorded here.
- No repository-visibility change, artifact deletion, upload-policy change, signing action, payment action, or release publication is made by this checklist.

**Do not describe the current artifact as a private customer beta or commercial download.** A new distribution policy requires a separate owner-approved change; this document intentionally does not change repository visibility or artifact-upload behaviour.

## 4. Product, host and quality acceptance

| Gate | Status at review |
|---|---|
| Windows x64 VST3 compile and package | **PASS** — exact-head workflow #44365584373 |
| Portable DSP/static/ASan/UBSan suite | **PASS** — 18/18 steps in `dsp-safety` |
| Windows VST3, metadata, AMD64 PE32+, pluginval strictness 5 | **PASS** — `build-windows` job |
| Preview install/update/backup/rollback integration | **PASS** — exact-head Windows job |
| Archive integrity and all package checksums | **PASS** — independently downloaded artifact #14456259709 |
| FL Studio scan/load and MIDI routing | **UNVERIFIED** |
| Host preset/state/automation save-and-reopen | **UNVERIFIED** |
| Musical A/B against reference adlibs | **UNVERIFIED** |
| Target Windows machine CPU, latency and long-session stability | **UNVERIFIED** |
| Current-source local Quick4 renderer | **UNVERIFIED** — repository `AGENTS.md` requires the shared native Quick4 path; the inspected artifact is a real JUCE renderer capture, not a local Quick4 verification |
| Formant-preserving pitch shifting and STFT spectral-envelope morphing | **NOT IMPLEMENTED** — the README explicitly describes the current granular pitch and filter-bank prototypes as incomplete |
| Code signing, customer terms and release approval | **OPEN** |

### Bounded UI review

The real JUCE screenshot archive was downloaded from run #44365584546. Four existing scenes were inspected: neon main at 1280×800, the effect-picker menu, the Ivory alternate theme at 1280×800, and neon minimum at 800×500.

- No clipping or overlap was apparent in those four scenes.
- The effect-picker labels and Ivory theme remained readable.
- At the 800×500 minimum, supporting labels and secondary text become very small; treat minimum-size legibility as a remaining usability concern.
- No new screenshots were generated, no UI source was changed, and no control interaction or FL Studio runtime behaviour is claimed.

The README still labels the product experimental v0.10.2 and documents important pitch/formant and spectral-morph limitations. Those limitations must remain explicit in any owner-test material; do not market the exposed Formant Lock control as completed independent formant preservation.

## 5. Release decision and required owner clearances

**Current decision: NO COMMERCIAL RELEASE.** Keep the existing preview branch and artifact as owner-test evidence only.

Before a commercial release candidate can be approved:

1. Record the JUCE AGPLv3 or commercial-licence route that applies to the distributed binary.
2. Confirm authorship and commercial redistribution rights for the copied DeEsser and Master DSP, and define the licence for the complete SkellCore and AdlibLab source set.
3. Approve the product licence, third-party notices, vendor/support URL and e-mail, EULA/customer terms, and code-signing plan.
4. Choose an owner-approved distribution channel and describe its actual access level accurately.
5. Complete FL Studio scan/load, MIDI routing, state/preset/automation recall, reference-matched audio A/B, and target-Windows CPU/latency/stability acceptance.
6. Either implement and verify the advertised formant/spectral features or keep the product scope and marketing explicitly limited to the current experimental algorithms.
7. Re-run the release gates against the exact final source commit and independently inspect the resulting signed package before publishing.

No licence, payment, activation, signing, visibility, or release decision is made by this document.
