#!/usr/bin/env python3
"""Static invariants. A real Windows JUCE/VST3 build is still required."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
source=(root/'Source/PluginProcessor.cpp').read_text()
header=(root/'Source/PluginProcessor.h').read_text()
ui='\n'.join(p.read_text() for p in (root/'Source/ui').glob('*.h'))
ids=re.findall(r'\b[fb c]\(l,"([A-Za-z0-9]+)"',source)
assert len(ids)>=45, f"Too few parameters: {len(ids)}"
assert len(ids)==len(set(ids)), 'Duplicate parameter IDs'
ui_ids=set(re.findall(r'(?:knob|combo|toggle)\("([A-Za-z0-9]+)"',ui))
ui_ids.update(re.findall(r'parameters,"([A-Za-z0-9]+)"',ui))
assert not (ui_ids-set(ids)), f"Unregistered UI parameters: {ui_ids-set(ids)}"
for part in 'StackPanel CharacterPanel ThrowPanel MeltPanel ClipBankView MiniKeyboard VoiceMap ChaosKnob HeaderBar LookAndFeel'.split():
 assert (root/'Source/ui'/f'{part}.h').exists(),part
for part in 'ChainRouter OnsetDetector PhraseTracker PitchTracker PitchFormantEngine StackEngine CharacterEngine ThrowEngine MeltEngine Vocoder ClipBank ChaosEngine'.split():
 assert (root/'Source/dsp'/f'{part}.h').exists(),part
cmake=(root/'CMakeLists.txt').read_text()
assert re.search(r'\bFORMATS\s+VST3\s*\n', cmake), 'Expected Windows VST3 only'
assert 'SkellCoreDependency' in cmake and 'SkellCore::UIRenderer' in cmake
assert 'SkellCore::DSP SkellCore::UI' in cmake, 'SkellCore missing from release'
assert 'skellcore-vst-template.git' in cmake, 'Wrong SkellCore repo'
assert 'eb56702153c4373799a4e8e2663e0f158c0115a6' in cmake
assert 'SkellGuidedTourHeaders' not in cmake, 'Duplicate SkellCore checkout'
assert 'add_subdirectory(SkellCore)' not in cmake, 'Stale bundled core must not build'
assert 'b(l,"globalOn","Global Power",true)' in source
assert 'globalBypass.setEnabled(p.globalOn)' in source
assert (root/'Source/dsp/GlobalBypassFader.h').exists()
assert 'tests/global_bypass.cpp' in (root/'.github/workflows/build.yml').read_text()
assert 'HostTempoClock.h' in header and 'updateTempoFromPlayhead' in source
assert (root/'Source/dsp/PeakMeter.h').exists()
workflow=(root/'.github/workflows/build.yml').read_text()
assert 'build-windows:' in workflow
assert 'build-macos:' not in workflow and 'macos-latest' not in workflow
assert 'Standalone' not in workflow
assert 'Skell-AdlibLab-Windows-VST3' in workflow
assert 'NEEDS_MIDI_INPUT TRUE' in (root/'CMakeLists.txt').read_text()
assert 'createLayout()' in header
assert 'MidiBuffer::Iterator' not in source, 'Deprecated JUCE MIDI iterator reintroduced'
assert 'midi.cbegin()' in source and 'midiIterator' in source
assert 'AffineTransform::identity' not in (root/'Source/PluginEditor.cpp').read_text()
assert 'actions/checkout@v5' in workflow and 'actions/upload-artifact@v6' in workflow
assert 'Compress-Archive' not in workflow, 'Nested ZIP artifact would require two extractions'
assert 'path: artifact/' in workflow
assert (root/'Source/dsp/ModuleOrder.h').exists()
assert 'adlib::ModuleOrder moduleOrder;' in header
assert 'resetFactoryPatch()' in (root/'Source/ui/HeaderBar.h').read_text()
assert 'fileDragMove' in (root/'Source/ui/ClipBankView.h').read_text()
print(f"Project static checks PASS: {len(ids)} parameters, {len(ui_ids)} UI bindings")
