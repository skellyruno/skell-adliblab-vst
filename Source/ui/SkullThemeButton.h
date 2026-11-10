#pragma once
#include "Theme.h"
#include <functional>
// Original vector-only Skell skull. No bitmap, emoji or unavailable icon font.
class SkullThemeButton final : public juce::Button {
public:
 explicit SkullThemeButton(const Theme::Palette& p):juce::Button("Change visual theme"),palette(p){setTooltip("Change theme: Cosmic / Neon / Ivory / Crimson");}
 void paintButton(juce::Graphics&g,bool hover,bool pressed)override{
  auto r=getLocalBounds().toFloat().reduced(2);
  auto a=palette.melt;
  g.setColour(palette.raised);g.fillRoundedRectangle(r,9);
  g.setColour(a.withAlpha(hover?1.f:.55f));g.drawRoundedRectangle(r,9,1.4f);
  juce::Graphics::ScopedSaveState save(g);
  g.addTransform(juce::AffineTransform::translation((r.getWidth()-48.f)*.5f,(r.getHeight()-48.f)*.5f));
  juce::Path bone;
  bone.startNewSubPath(11,15);bone.cubicTo(11,5,36,5,37,15);bone.cubicTo(42,23,35,27,34,30);
  bone.lineTo(34,36);bone.lineTo(30,39);bone.lineTo(18,39);bone.lineTo(14,36);bone.lineTo(14,30);
  bone.cubicTo(7,25,8,21,11,15);bone.closeSubPath();
  g.setColour(pressed?palette.melt:palette.text.withAlpha(.94f));g.fillPath(bone);
  g.setColour(palette.background);
  juce::Path eye1,eye2,nose;
  eye1.addEllipse(15,19,7.5f,9.5f);eye2.addEllipse(26,19,7.5f,9.5f);
  nose.startNewSubPath(24,29);nose.lineTo(21,34);nose.lineTo(27,34);nose.closeSubPath();
  g.fillPath(eye1);g.fillPath(eye2);g.fillPath(nose);
  for(int i=0;i<3;++i)g.drawLine(19.f+i*5.f,35.f,19.f+i*5.f,39.f,1.3f);
  g.setColour(a.withAlpha(hover?1.f:.55f));g.drawEllipse(11,8,26,26,1.0f);
 }
private:const Theme::Palette&palette;
};
