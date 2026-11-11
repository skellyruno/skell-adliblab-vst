#pragma once
#include "Theme.h"
#include <array>
#include <functional>

// Same editor tree as the live VST3: captured by SkellCore UI renderer.
class PresetMenuView final:public juce::Component {
public:
 explicit PresetMenuView(const Theme::Palette& pal):palette(pal){
  setWantsKeyboardFocus(true);
 }
 std::function<void(int)> onChoice;
 std::function<void()> onDismiss;
 void paint(juce::Graphics&g)override{
  g.fillAll(palette.background.withAlpha(.40f));
  const juce::Rectangle<float> box(365.f,78.f,412.f,390.f);
  g.setColour(palette.background.withAlpha(.28f));g.fillRoundedRectangle(box.translated(3.f,6.f),12.f);
  g.setGradientFill(juce::ColourGradient(palette.surface.brighter(.04f),box.getTopLeft(),palette.panel,box.getBottomRight(),false));
  g.fillRoundedRectangle(box,11.f);
  g.setColour(palette.border);g.drawRoundedRectangle(box,11.f,1.f);
  g.setColour(palette.stack);g.fillRoundedRectangle(380.f,91.f,3.f,34.f,1.f);
  g.setColour(palette.text);g.setFont(Theme::display(17.f));
  g.drawText("ADLIBLAB  /  PRESETS",392,90,270,25,juce::Justification::left);
  g.setColour(palette.muted);g.setFont(Theme::mono(10.f));
  g.drawText("FACTORY SOUND DESIGN",393,116,265,17,juce::Justification::left);
  g.drawText("ESC TO CLOSE",656,117,108,16,juce::Justification::right);
  for(int i=0;i<8;++i){
   const int y=139+i*39;
   if(i==6){g.setColour(palette.border);g.drawHorizontalLine(y-3,383.f,757.f);}
   if(hover==i){
    g.setColour(palette.raised);g.fillRoundedRectangle(377.f,(float)y+2,388.f,35.f,6.f);
    g.setColour(palette.stack);g.fillRoundedRectangle(378.f,(float)y+8,3.f,23.f,1.f);
   }
   g.setColour(hover==i?palette.stack:palette.muted);g.setFont(Theme::mono(11.f));
   g.drawText(i<6?juce::String(i+1).paddedLeft('0',2):"+",388,y+6,35,25,juce::Justification::left);
   g.setColour(palette.text);g.setFont(Theme::display(13.f));
   g.drawText(names[(size_t)i],428,y+6,300,25,juce::Justification::left);
  }
 }
 void mouseMove(const juce::MouseEvent&e)override{const int next=rowAt(e.position);if(next!=hover){hover=next;repaint();}}
 void mouseExit(const juce::MouseEvent&)override{hover=-1;repaint();}
 void mouseUp(const juce::MouseEvent&e)override{
  const int row=rowAt(e.position);
  if(row>=0){if(onChoice)onChoice(row<6?row+1:row==6?10:11);}
  else if(onDismiss)onDismiss();
 }
 bool keyPressed(const juce::KeyPress&k)override{
  if(k==juce::KeyPress::escapeKey){if(onDismiss)onDismiss();return true;}
  if(k==juce::KeyPress::downKey){hover=(hover+1)%8;repaint();return true;}
  if(k==juce::KeyPress::upKey){hover=(hover+7)%8;repaint();return true;}
  if(k==juce::KeyPress::returnKey){if(onChoice)onChoice(hover<0?1:hover<6?hover+1:hover==6?10:11);return true;}
  return false;
 }
private:
 int rowAt(juce::Point<float>p)const{
  if(p.x<376.f||p.x>764.f||p.y<139.f||p.y>=451.f)return -1;
  return juce::jlimit(0,7,(int)((p.y-139.f)/39.f));
 }
 const Theme::Palette&palette;
 int hover=-1;
 const std::array<juce::String,8> names{{"INIT PATCH","WIDE ADLIB STACK","ROBOT INCOMING",
 "GHOST THROWS","DEMON DOUBLES","CARTOON PUNCH","SAVE PATCH...","LOAD PATCH..."}};
};
