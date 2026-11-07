#pragma once
#include "Theme.h"
class LabLookAndFeel final:public juce::LookAndFeel_V4 {
public:
 LabLookAndFeel(){setColour(juce::ComboBox::backgroundColourId,Theme::panel.brighter(.2f));setColour(juce::ComboBox::textColourId,Theme::text);setColour(juce::TextButton::buttonColourId,Theme::panel.brighter(.1f));setColour(juce::TextButton::textColourOffId,Theme::text);setColour(juce::PopupMenu::backgroundColourId,Theme::panel);setColour(juce::PopupMenu::textColourId,Theme::text);setColour(juce::Label::textColourId,Theme::text);}
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&slider)override {
  auto col=slider.findColour(juce::Slider::rotarySliderFillColourId);
  auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(5);
  float diameter=juce::jmin(r.getWidth(),r.getHeight());
  r=r.withSizeKeepingCentre(diameter,diameter);
  const auto c=r.getCentre();
  const float radius=diameter*.5f;
  g.setColour(juce::Colour(0xff0c0e16));g.fillEllipse(r);
  g.setColour(juce::Colour(0xff34313f));g.drawEllipse(r,1.5f);
  juce::Path base,arc;
  base.addCentredArc(c.x,c.y,radius-4,radius-4,0,start,end,true);
  arc.addCentredArc(c.x,c.y,radius-4,radius-4,0,start,start+(end-start)*pos,true);
  g.setColour(juce::Colour(0xff302c3b));g.strokePath(base,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  g.setColour(col);g.strokePath(arc,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  float angle=start+(end-start)*pos;
  juce::Point<float>p(c.x+std::sin(angle)*(radius-10),c.y-std::cos(angle)*(radius-10));
  g.fillEllipse(p.x-2.8f,p.y-2.8f,5.6f,5.6f);
 }
};
