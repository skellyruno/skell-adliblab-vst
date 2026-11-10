#pragma once
#include "Theme.h"
class LabLookAndFeel final:public juce::LookAndFeel_V4 {
public:
 LabLookAndFeel(){applyPalette(Theme::makePalette(0));}
 void applyPalette(const Theme::Palette& p){
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
 void drawButtonBackground(juce::Graphics&g,juce::Button&button,const juce::Colour&,bool hover,bool down)override {
  auto r=button.getLocalBounds().toFloat().reduced(.7f);
  g.setColour(down?palette.border:palette.raised.brighter(hover?.14f:0.f));g.fillRoundedRectangle(r,6.f);
  g.setColour((hover?palette.muted:palette.border).withAlpha(hover?.9f:.8f));g.drawRoundedRectangle(r,6.f,1.0f);
 }
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&slider)override {
  auto col=slider.findColour(juce::Slider::rotarySliderFillColourId);
  auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(5);
  float d=juce::jmin(r.getWidth(),r.getHeight());r=r.withSizeKeepingCentre(d,d);
  const auto c=r.getCentre();const float radius=d*.5f;
  g.setColour(palette.background);g.fillEllipse(r);
  g.setColour(palette.border);g.drawEllipse(r,1.5f);
  juce::Path track,arc;
  track.addCentredArc(c.x,c.y,radius-4,radius-4,0,start,end,true);
  arc.addCentredArc(c.x,c.y,radius-4,radius-4,0,start,start+(end-start)*pos,true);
  g.setColour(palette.border);g.strokePath(track,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  g.setColour(col);g.strokePath(arc,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  float angle=start+(end-start)*pos;
  juce::Point<float>p(c.x+std::sin(angle)*(radius-10),c.y-std::cos(angle)*(radius-10));
  g.fillEllipse(p.x-3.f,p.y-3.f,6.f,6.f);
  g.setColour(palette.text.withAlpha(.065f));g.fillEllipse(r.reduced(radius*.35f));
 }
private:Theme::Palette palette=Theme::makePalette(0);
};
