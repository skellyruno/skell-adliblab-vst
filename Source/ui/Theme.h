#pragma once
#include <JuceHeader.h>
namespace Theme {
constexpr int designWidth=1280, designHeight=800;
// UI colours are editor-local; opening two instances with different skins does not
// change either instance's palette or DSP.
struct Palette {
 juce::Colour background, surface, panel, raised, text, muted, border;
 juce::Colour stack, character, throwColour, melt, chaos;
 juce::String name;
 juce::Colour accent(int module) const {
  switch(module){case 0:return stack;case 1:return character;case 2:return throwColour;default:return melt;}
 }
};
inline Palette makePalette(int index){
 switch(index){
  case 1: return {juce::Colour(0xff070d0a),juce::Colour(0xff101913),juce::Colour(0xff111b17),juce::Colour(0xff1c2a20),juce::Colour(0xffe4fce8),juce::Colour(0xff8baa98),juce::Colour(0xff365340),
                   juce::Colour(0xff80f52f),juce::Colour(0xff22d789),juce::Colour(0xff6cdec8),juce::Colour(0xffc8ff63),juce::Colour(0xffffb34d),"NEON"};
  case 2: return {juce::Colour(0xffd4cdbc),juce::Colour(0xffe6dece),juce::Colour(0xffddd4c4),juce::Colour(0xffc9bdab),juce::Colour(0xff252823),juce::Colour(0xff586057),juce::Colour(0xffaea48e),
                   juce::Colour(0xff6a7450),juce::Colour(0xffa26765),juce::Colour(0xff4c8391),juce::Colour(0xff727f37),juce::Colour(0xffbb753a),"IVORY"};
  case 3: return {juce::Colour(0xff10090d),juce::Colour(0xff1b1117),juce::Colour(0xff1c121a),juce::Colour(0xff2b1c23),juce::Colour(0xfff9e8e8),juce::Colour(0xffb69aa4),juce::Colour(0xff5b3241),
                   juce::Colour(0xffe94c70),juce::Colour(0xffff738f),juce::Colour(0xffff947c),juce::Colour(0xfff9c69e),juce::Colour(0xffff6949),"CRIMSON"};
  default:return {juce::Colour(0xff090811),juce::Colour(0xff11101b),juce::Colour(0xff14121e),juce::Colour(0xff222031),juce::Colour(0xffe9e8f4),juce::Colour(0xff9491a8),juce::Colour(0xff40374f),
                   juce::Colour(0xff9a63ff),juce::Colour(0xffff61b9),juce::Colour(0xff51dff7),juce::Colour(0xffceff54),juce::Colour(0xffff984c),"COSMIC"};
 }
}
inline juce::Font display(float size){return juce::Font(juce::FontOptions("sans-serif",size,juce::Font::bold));}
inline juce::Font mono(float size){return juce::Font(juce::FontOptions("monospace",size,juce::Font::plain));}
}
