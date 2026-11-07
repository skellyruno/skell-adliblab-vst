#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <cmath>
#include <vector>
#include <memory>

namespace adlib {
constexpr int kModuleCount = 4;
constexpr int kPadCount = 8;
constexpr int kMaxStackVoices = 8;
constexpr float pi = juce::MathConstants<float>::pi;
inline float clamp (float x, float lo, float hi) { return juce::jlimit(lo, hi, x); }
inline float equalPowerPanL (float pan) { return std::cos((pan + 1.0f) * pi * 0.25f); }
inline float equalPowerPanR (float pan) { return std::sin((pan + 1.0f) * pi * 0.25f); }
struct Stereo { float l = 0, r = 0; Stereo& operator+= (Stereo b) { l+=b.l; r+=b.r; return *this; } };
inline Stereo operator*(Stereo a,float v) { return {a.l*v,a.r*v}; }
struct Events { bool hit=false, wordEnd=false, syllableEnd=false; float envelope=0; float trackedHz=0; };
struct Params {
 float mix=0.65f, outputGain=0, width=1, chaos=0, bpm=120;
 bool chaosLock=false, safetyLimiter=true;
 int latencyMode=0;
 bool stackOn=true; int stackVoices=3; float stackSpread=30, stackFormant=0, stackTime=22,stackWidth=0.75f,stackChaosAmt=0.45f;
 bool stackRerollOnHit=true,stackScaleLock=false; int stackKey=0,stackScale=0;
 bool charOn=true; int charType=0; float charIntensity=0.35f,charFormant=0,charGlide=50,charCrush=12,charDetune=0.2f;
 bool throwOn=true; int throwMode=0,throwDiv=2,throwRepeats=4;float throwFeedback=0.55f,throwFilterStart=12000,throwFilterEnd=1000,throwPitchPerRepeat=0,throwDuck=0.4f;
 bool throwReverse=false,throwPingPong=true;
 bool meltOn=false; int meltMode=0; float meltAmount=0.4f,meltFollowPitch=0.5f,meltFollowLoudness=0.8f,meltSmooth=0.4f;int meltRoot=60;
 int midiChannel=0,kbRoot=60;bool kbFormantLock=true,kbVelIntensity=true,kbVelTail=true,kbAltReverse=false;
};
}
