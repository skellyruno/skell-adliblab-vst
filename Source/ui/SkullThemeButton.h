#pragma once
#include "Theme.h"
#include <functional>
// Original vector-only Skell skull. No bitmap, emoji or unavailable icon font.
class SkullThemeButton final : public juce::Button {
public:
 explicit SkullThemeButton(const Theme::Palette& p):juce::Button("Change visual theme"),palette(p){setTooltip("Left click: next theme. Right click: select Cosmic, Neon, Ivory or Crimson.");}
 std::function<void(int)>onThemeChosen;
 void mouseDown(const juce::MouseEvent&event)override{
  if(!event.mods.isPopupMenu()){juce::Button::mouseDown(event);return;}
  juce::PopupMenu menu;
  menu.addItem(1,"Cosmic");menu.addItem(2,"Neon");
  menu.addItem(3,"Ivory");menu.addItem(4,"Crimson");
  juce::Component::SafePointer<SkullThemeButton>safe(this);
  menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),[safe](int result){
   if(safe&&result>=1&&result<=4&&safe->onThemeChosen)safe->onThemeChosen(result-1);
  });
 }
 void paintButton(juce::Graphics&g,bool hover,bool pressed)override{
  const auto bounds=getLocalBounds().toFloat().reduced(1.5f);
  const auto accent=palette.melt;
  g.setGradientFill(juce::ColourGradient(palette.raised.brighter(.09f),bounds.getTopLeft(),
                                        palette.surface,bounds.getBottomRight(),false));
  g.fillRoundedRectangle(bounds,9.f);
  g.setColour(accent.withAlpha(hover?.98f:.65f));
  g.drawRoundedRectangle(bounds.reduced(.6f),8.5f,hover?1.5f:1.1f);
  juce::Graphics::ScopedSaveState save(g);
  const auto shape=juce::AffineTransform::scale(bounds.getWidth()/48.f,bounds.getHeight()/48.f)
       .translated(bounds.getX(),bounds.getY());
  g.addTransform(shape);
  // AdlibLab's echo-reaper variant: the Skell skull sits inside a dark
  // hood with symmetric waveform marks. Original JUCE vector geometry.
  juce::Path hood;
  hood.startNewSubPath(24.f,4.5f);
  hood.cubicTo(10.f,5.f,7.f,17.f,6.7f,29.f);
  hood.cubicTo(7.f,40.f,15.f,44.f,24.f,45.f);
  hood.cubicTo(33.f,44.f,41.f,40.f,41.3f,29.f);
  hood.cubicTo(41.f,17.f,38.f,5.f,24.f,4.5f);
  hood.closeSubPath();
  g.setColour(palette.background);g.fillPath(hood);
  g.setColour(accent.withAlpha(hover?.95f:.73f));
  g.strokePath(hood,juce::PathStrokeType(1.5f,juce::PathStrokeType::curved,
                                       juce::PathStrokeType::rounded));
  // Inner skull keeps the shared bone silhouette without SkellKey's key motif.
  juce::Path face;
  face.startNewSubPath(16.f,15.f);
  face.cubicTo(19.f,12.f,29.f,12.f,32.f,15.f);
  face.cubicTo(37.f,20.f,35.f,28.f,31.f,31.f);
  face.lineTo(30.3f,38.f);face.lineTo(27.6f,40.f);
  face.lineTo(20.4f,40.f);face.lineTo(17.7f,38.f);
  face.lineTo(17.f,31.f);
  face.cubicTo(13.f,28.f,11.f,20.f,16.f,15.f);
  face.closeSubPath();
  g.setColour(pressed?accent:palette.text);g.fillPath(face);
  juce::Path eyes;
  eyes.startNewSubPath(16.f,23.f);eyes.lineTo(22.f,24.f);
  eyes.lineTo(21.f,29.f);eyes.lineTo(17.5f,28.f);eyes.closeSubPath();
  eyes.startNewSubPath(32.f,23.f);eyes.lineTo(26.f,24.f);
  eyes.lineTo(27.f,29.f);eyes.lineTo(30.5f,28.f);eyes.closeSubPath();
  g.setColour(palette.background);g.fillPath(eyes);
  juce::Path nose;
  nose.startNewSubPath(24.f,29.f);nose.lineTo(21.7f,34.f);
  nose.lineTo(26.3f,34.f);nose.closeSubPath();
  g.fillPath(nose);
  for(float x:{21.f,24.f,27.f})g.drawLine(x,36.4f,x,39.3f,1.1f);
  // Echo marks, not a key: a distinct identity for vocal effects.
  juce::Path waves;
  waves.startNewSubPath(4.5f,21.f);
  waves.quadraticTo(2.2f,26.f,4.5f,31.f);
  waves.startNewSubPath(43.5f,21.f);
  waves.quadraticTo(45.8f,26.f,43.5f,31.f);
  g.setColour(accent.withAlpha(.86f));
  g.strokePath(waves,juce::PathStrokeType(1.4f,juce::PathStrokeType::curved,
                                        juce::PathStrokeType::rounded));
 }
private:const Theme::Palette&palette;
};
