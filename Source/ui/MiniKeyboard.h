#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
// Two octaves and a final C. Black keys overlay white keys instead of
// allocating the same width to each chromatic semitone.
class MiniKeyboard final:public juce::Component {
public:
 explicit MiniKeyboard(SkellAdlibLabProcessor&p,const Theme::Palette&pal):processor(p),palette(pal){}
 ~MiniKeyboard()override{release();}
 void mouseDown(const juce::MouseEvent&e)override{if(e.mods.isPopupMenu())return;pressed=noteAt(e.position);processor.requestMidiOn(pressed);repaint();}
 void mouseDrag(const juce::MouseEvent&e)override{
  if(pressed<0)return;
  if(!getLocalBounds().contains(e.getPosition())){release();return;}
  int n=noteAt(e.position);if(n!=pressed){processor.requestMidiOff(pressed);pressed=n;processor.requestMidiOn(pressed);repaint();}}
 void mouseUp(const juce::MouseEvent&)override{release();}
 void mouseExit(const juce::MouseEvent&e)override{if(!e.mods.isAnyMouseButtonDown())release();}
 void paint(juce::Graphics&g)override{
  const auto note=pressed>=0?pressed:processor.getNote();const float w=(float)getWidth()/15.f;
  // Light skins must preserve the familiar white/black piano-key relationship.
  const bool light=palette.background.getPerceivedBrightness()>.6f;
  const auto whiteKey=light?juce::Colour(0xffeee5d6):palette.text;
  const auto blackKey=light?juce::Colour(0xff242824):palette.raised;
  g.setColour(palette.surface);g.fillRoundedRectangle(getLocalBounds().toFloat(),6);
  for(int white=0;white<15;++white){
   int midi=whiteNote(white);auto r=juce::Rectangle<float>(white*w+0.4f,1.f,w-1.2f,(float)getHeight()-2);
   g.setColour(midi==note?palette.melt:whiteKey);g.fillRoundedRectangle(r,3);
   g.setColour(palette.background.withAlpha(.75f));g.drawRoundedRectangle(r,3,1);
   if(midi%12==0){g.setColour(midi==note?palette.background:palette.muted);g.setFont(Theme::mono(10));g.drawText(midi==48?"C3":midi==60?"C4":"C5",r.reduced(2).toNearestInt().withY(getHeight()-19),juce::Justification::centred);}
  }
  for(int black=0;black<10;++black){
   int key=blackNote(black);auto r=blackRect(key);
   g.setColour(key==note?palette.melt:blackKey);g.fillRoundedRectangle(r,3);
   g.setColour(key==note?palette.background:palette.border);g.drawRoundedRectangle(r,3,1);
  }
 }
private:
 static int whiteNote(int idx){static constexpr int degrees[]={0,2,4,5,7,9,11};return 48+(idx/7)*12+degrees[idx%7];}
 static int blackNote(int idx){static constexpr int degrees[]={1,3,6,8,10};return 48+(idx/5)*12+degrees[idx%5];}
 juce::Rectangle<float> blackRect(int midi)const{
  int octave=(midi-48)/12,degree=(midi-48)%12;
  int whiteBefore=(degree==1?0:degree==3?1:degree==6?3:degree==8?4:5)+7*octave;
  auto w=(float)getWidth()/15.f;
  return {w*(whiteBefore+1.f)-w*.28f,1.f,w*.56f,(float)getHeight()*.61f};
 }
 int noteAt(juce::Point<float>p)const{
  for(int b=0;b<10;++b){int note=blackNote(b);if(blackRect(note).contains(p))return note;}
  int white=juce::jlimit(0,14,(int)(p.x*15.f/juce::jmax(1,getWidth())));
  return whiteNote(white);
 }
 void release(){if(pressed>=0){processor.requestMidiOff(pressed);pressed=-1;repaint();}}
 SkellAdlibLabProcessor&processor;const Theme::Palette&palette;int pressed=-1;
};
