#pragma once
#include <JuceHeader.h>
namespace Theme {
constexpr int designWidth=1280,designHeight=800;
static const juce::Colour background=juce::Colour::fromRGB(8,6,13);
static const juce::Colour panel=juce::Colour::fromRGB(18,16,24);
static const juce::Colour text=juce::Colour::fromRGB(212,210,220);
static const juce::Colour muted=juce::Colour::fromRGB(117,115,133);
static const juce::Colour stack=juce::Colour::fromRGB(138,61,255);
static const juce::Colour character=juce::Colour::fromRGB(255,47,179);
static const juce::Colour throwColour=juce::Colour::fromRGB(47,217,255);
static const juce::Colour melt=juce::Colour::fromRGB(198,255,46);
static const juce::Colour chaos=juce::Colour::fromRGB(255,122,26);
inline juce::Font display(float size){return juce::Font(juce::FontOptions("sans-serif",size,juce::Font::bold));}
inline juce::Font mono(float size){return juce::Font(juce::FontOptions("monospace",size,juce::Font::plain));}
}
