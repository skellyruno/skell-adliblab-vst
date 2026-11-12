#!/usr/bin/env python3
"""Test stable design bounds across the five supported UI widths.
The Windows JUCE build is still needed for actual rasterization and controls.
"""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
canvas=(root/'Source/PluginEditor.cpp').read_text()
modules=(root/'Source/ui/ModulePanel.h').read_text()
header=(root/'Source/ui/HeaderBar.h').read_text()
assert 'setFixedAspectRatio' in canvas and 'juce::AffineTransform::scale(s)' in canvas
assert 'setResizeLimits(800,500,1920,1200)' in canvas
assert 'onThemeSelect' in canvas and 'onThemeChosen' in header
assert 'mouseDown' in (root/'Source/ui/SkullThemeButton.h').read_text()
positions=re.search(r'constexpr int positions\[\]={(.*?)};',canvas)
assert positions
xs=[int(v) for v in positions.group(1).split(',')]
assert xs == [30,342,654,966]
regions=[(xs[i],90,290,455) for i in range(4)]
regions += [(15,8,1250,65),(27,567,233,214),(286,590,708,68),
            (286,666,708,79),(1012,569,233,207)]
for width in (800,1024,1280,1600,1920):
 scale=width/1280
 height=round(width*800/1280)
 assert height>=500 and height<=1200
 for x,y,w,h in regions:
  assert x>=0 and y>=0 and x+w<=1280 and y+h<=800
  assert (x+w)*scale <= width+.01
  assert (y+h)*scale <= height+.6
for a,b in zip(xs,xs[1:]):assert b-(a+290)>=20
assert 'DragGrip' not in modules and 'requestReorder' in modules and 'mouseDrag' in modules
assert 'remove.setBounds(217,11,27,28)' in modules
assert 'on.setBounds(249,11,29,29)' in modules
assert 'globalPower.setBounds(1144,17,38,36)' in header
assert 'setup(mix' not in header and 'mixAtt' not in header
assert 'mix.setBounds(1109,606,56,62)' in canvas
assert 'widthReadout' in canvas and 'mixReadout' in canvas and 'gainReadout' in canvas
assert 'updateBypassVisual' in modules and 'updateBypassVisual()' in canvas
assert 'EffectPickerView' in (root/'Source/ui/EffectPickerView.h').read_text()
assert 'EmptyRackSlot' in (root/'Source/ui/RackExtraPanel.h').read_text()
assert 'paintOverChildren' in canvas, 'Drag ghost must render in actual JUCE editor'
assert 'JucePlugin_VersionString' in canvas, 'UI footer must track the real VST3 version'
assert 'ADLIBLAB 0.3' not in canvas, 'Stale release label'
print('UI layout contract PASS: 5 aspect-locked sizes, all top-level regions in bounds')
