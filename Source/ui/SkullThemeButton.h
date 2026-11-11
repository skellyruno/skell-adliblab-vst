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
  // Angular skull mark: replace the old rounded cartoon head, keeping the
  // clickable theme action and all skins. Pure JUCE paths, no external asset.
  juce::Path bone;
  bone.startNewSubPath(15.f,8.f);bone.lineTo(33.f,8.f);
  bone.lineTo(40.f,14.f);bone.lineTo(40.f,23.f);bone.lineTo(35.f,29.f);
  bone.lineTo(35.f,35.f);bone.lineTo(31.f,41.f);bone.lineTo(17.f,41.f);
  bone.lineTo(13.f,35.f);bone.lineTo(13.f,29.f);bone.lineTo(8.f,23.f);
  bone.lineTo(8.f,14.f);bone.closeSubPath();
  g.setColour(pressed?accent:palette.text.withAlpha(.96f));g.fillPath(bone);
  g.setColour(accent.withAlpha(hover?.90f:.46f));g.strokePath(bone,juce::PathStrokeType(1.1f));
  juce::Path sockets;
  sockets.startNewSubPath(13.f,22.f);sockets.lineTo(22.f,23.f);
  sockets.lineTo(20.5f,30.f);sockets.lineTo(15.5f,28.5f);sockets.closeSubPath();
  sockets.startNewSubPath(35.f,22.f);sockets.lineTo(26.f,23.f);
  sockets.lineTo(27.5f,30.f);sockets.lineTo(32.5f,28.5f);sockets.closeSubPath();
  sockets.startNewSubPath(24.f,29.f);sockets.lineTo(20.8f,34.f);
  sockets.lineTo(27.2f,34.f);sockets.closeSubPath();
  g.setColour(palette.background);g.fillPath(sockets);
  g.setColour(palette.background.withAlpha(.88f));
  for(float x: {20.f,24.f,28.f})g.drawLine(x,36.f,x,40.f,1.25f);
  g.setColour(accent.withAlpha(.85f));
  g.drawLine(15.f,17.f,21.f,18.f,1.2f);
  g.drawLine(33.f,17.f,27.f,18.f,1.2f);
  g.fillEllipse(40.f,39.f,4.2f,4.2f);
 }
private:const Theme::Palette&palette;
};
