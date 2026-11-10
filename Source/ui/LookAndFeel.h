#pragma once
#include "Theme.h"
#include <cmath>
// Lightweight vector control rendering; no runtime image or font dependencies.
class LabLookAndFeel final:public juce::LookAndFeel_V4 {
public:
 LabLookAndFeel(){applyPalette(Theme::makePalette(0));}
 void applyPalette(const Theme::Palette&p){
  palette=p;
  setColour(juce::ComboBox::backgroundColourId,p.raised);
  setColour(juce::ComboBox::textColourId,p.text);
  setColour(juce::ComboBox::arrowColourId,p.muted);
  setColour(juce::ComboBox::outlineColourId,p.border);
  setColour(juce::TextButton::buttonColourId,p.raised);
  setColour(juce::TextButton::buttonOnColourId,p.stack);
  setColour(juce::TextButton::textColourOffId,p.text);
  setColour(juce::TextButton::textColourOnId,p.background);
  setColour(juce::PopupMenu::backgroundColourId,p.surface);
  setColour(juce::PopupMenu::textColourId,p.text);
  setColour(juce::PopupMenu::highlightedBackgroundColourId,p.raised);
  setColour(juce::Label::textColourId,p.text);
  setColour(juce::ToggleButton::textColourId,p.text);
  setColour(juce::TooltipWindow::backgroundColourId,p.surface);
  setColour(juce::TooltipWindow::textColourId,p.text);
 }
 void drawButtonBackground(juce::Graphics&g,juce::Button&button,const juce::Colour&,bool hover,bool down)override{
  const auto r=button.getLocalBounds().toFloat().reduced(1.f);
  const auto top=down?palette.raised.darker(.22f):palette.raised.brighter(hover?.15f:.04f);
  g.setGradientFill(juce::ColourGradient(top,r.getX(),r.getY(),palette.surface,r.getX(),r.getBottom(),false));
  g.fillRoundedRectangle(r,6.f);
  g.setColour((hover?palette.stack:palette.border).withAlpha(hover?.75f:.72f));
  g.drawRoundedRectangle(r.reduced(.5f),6.f,1.f);
  g.setColour(palette.text.withAlpha(hover?.14f:.06f));
  g.drawHorizontalLine(juce::roundToInt(r.getY()+2.f),r.getX()+8.f,r.getRight()-8.f);
 }
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&slider)override{
  const auto accent=slider.findColour(juce::Slider::rotarySliderFillColourId);
  auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(3.f);
  const float diameter=juce::jmin(r.getWidth(),r.getHeight());
  if(diameter<=8.f)return;
  r=r.withSizeKeepingCentre(diameter,diameter);
  const auto centre=r.getCentre();const float radius=diameter*.5f;
  const bool hover=slider.isMouseOverOrDragging();
  g.setColour(palette.border.withAlpha(.62f));g.fillEllipse(r);
  const auto inner=r.reduced(2.f);
  g.setGradientFill(juce::ColourGradient(palette.raised.brighter(.17f),inner.getX(),inner.getY(),
                                        palette.background,inner.getRight(),inner.getBottom(),false));
  g.fillEllipse(inner);
  g.setColour(palette.text.withAlpha(.085f));g.drawEllipse(inner.reduced(1.f),.9f);
  const float trackRadius=radius-6.f;
  juce::Path track,arc;
  track.addCentredArc(centre.x,centre.y,trackRadius,trackRadius,0.f,start,end,true);
  arc.addCentredArc(centre.x,centre.y,trackRadius,trackRadius,0.f,
                    start+(end-start)*juce::jlimit(0.f,1.f,pos),true);
  g.setColour(palette.border.brighter(.15f));
  g.strokePath(track,juce::PathStrokeType(3.2f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  if(hover){
   g.setColour(accent.withAlpha(.17f));
   g.strokePath(arc,juce::PathStrokeType(6.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  }
  g.setColour(accent);
  g.strokePath(arc,juce::PathStrokeType(2.8f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  const float angle=start+(end-start)*juce::jlimit(0.f,1.f,pos);
  const auto tip=juce::Point<float>(centre.x+std::sin(angle)*(radius-10.f),centre.y-std::cos(angle)*(radius-10.f));
  g.setColour(accent.withAlpha(.22f));g.fillEllipse(tip.x-5.f,tip.y-5.f,10.f,10.f);
  g.setColour(accent);g.fillEllipse(tip.x-2.6f,tip.y-2.6f,5.2f,5.2f);
  g.setColour(palette.text.withAlpha(.13f));
  g.fillEllipse(centre.x-radius*.28f,centre.y-radius*.28f,radius*.56f,radius*.56f);
  g.setColour(palette.background.withAlpha(.53f));g.drawEllipse(r.reduced(radius*.29f),.8f);
 }
private:Theme::Palette palette=Theme::makePalette(0);
};
