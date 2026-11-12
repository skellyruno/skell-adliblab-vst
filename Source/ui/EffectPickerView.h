#pragma once
#include "Theme.h"
#include "../dsp/RackState.h"
#include <array>
#include <functional>
// Same JUCE component tree as the VST3; screenshotable by SkellCore renderer.
class EffectPickerView final:public juce::Component {
public:
 explicit EffectPickerView(const Theme::Palette& p):palette(p){setWantsKeyboardFocus(true);}
 std::function<void(adlib::RackType)>onChoose;
 std::function<void()>onDismiss;
 void paint(juce::Graphics&g)override{
  g.fillAll(palette.background.withAlpha(.76f));
  const auto panel=juce::Rectangle<float>(392.f,84.f,495.f,565.f);
  g.setColour(palette.surface);g.fillRoundedRectangle(panel,12.f);
  g.setColour(palette.border.brighter(.28f));g.drawRoundedRectangle(panel,12.f,1.5f);
  g.setColour(palette.stack);g.fillRoundedRectangle(408.f,101.f,3.f,40.f,1.5f);
  g.setColour(palette.text);g.setFont(Theme::display(20.f));
  g.drawText("REPLACE / ADD EFFECT",420,104,438,28,juce::Justification::left);
  g.setColour(palette.muted);g.setFont(Theme::mono(11));
  g.drawText("ONE EFFECT PER SLOT  /  FOUR SLOTS MAX",420,132,425,19,juce::Justification::left);
  for(int i=0;i<8;++i){
   const float y=163.f+i*56.f;
   auto r=juce::Rectangle<float>(408.f,y,463.f,49.f);
   g.setColour(hover==i?palette.raised:palette.panel);g.fillRoundedRectangle(r,7.f);
   g.setColour(hover==i?palette.stack:palette.border);g.drawRoundedRectangle(r,7.f,1.f);
   g.setColour(hover==i?palette.stack:palette.muted);g.setFont(Theme::mono(11));
   g.drawText(juce::String(i+1).paddedLeft('0',2),421,(int)y+6,35,29,juce::Justification::left);
   g.setColour(palette.text);g.setFont(Theme::display(14.f));
   g.drawText(names[(size_t)i],462,(int)y+6,380,29,juce::Justification::left);
  }
  g.setColour(palette.muted);g.setFont(Theme::mono(11));
  g.drawText("ESC / OUTSIDE CLICK TO CLOSE",419,621,440,18,juce::Justification::centred);
 }
 void mouseMove(const juce::MouseEvent&e)override{const int v=rowAt(e.position);if(hover!=v){hover=v;repaint();}}
 void mouseExit(const juce::MouseEvent&)override{hover=-1;repaint();}
 void mouseUp(const juce::MouseEvent&e)override{
  int index=rowAt(e.position);
  if(index>=0){if(onChoose)onChoose(types[(size_t)index]);}
  else if(onDismiss)onDismiss();
 }
 bool keyPressed(const juce::KeyPress&k)override{
  if(k==juce::KeyPress::escapeKey){if(onDismiss)onDismiss();return true;}
  if(k==juce::KeyPress::downKey){hover=(hover+1)%8;repaint();return true;}
  if(k==juce::KeyPress::upKey){hover=(hover+7)%8;repaint();return true;}
  if(k==juce::KeyPress::returnKey){if(onChoose)onChoose(types[(size_t)juce::jmax(0,hover)]);return true;}
  return false;
 }
private:
 int rowAt(juce::Point<float> p)const{
  if(p.x<408.f||p.x>871.f||p.y<163.f||p.y>=611.f)return -1;
  const int row=(int)((p.y-163.f)/56.f);
  return juce::isPositiveAndBelow(row,8)?row:-1;
 }
 const Theme::Palette&palette;int hover=-1;
 static constexpr std::array<adlib::RackType,8> types{{
  adlib::RackType::Stack,adlib::RackType::Character,adlib::RackType::Throw,
  adlib::RackType::Chop,adlib::RackType::Motion,adlib::RackType::Filter,
  adlib::RackType::Saturate,adlib::RackType::Empty}};
 const std::array<juce::String,8> names{{
  "STACK / HARMONY","CHARACTER / VOICE DESIGN","THROW / ECHO",
  "CHOP / RHYTHMIC CUTS","MOTION / AUTO-PAN","FILTER / TONE",
  "SATURATE / DRIVE","EMPTY / REMOVE EFFECT"}};
};
