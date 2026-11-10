#pragma once
#include <JuceHeader.h>
#include "dsp/ChainRouter.h"
#include "dsp/ClipBank.h"
#include "dsp/PitchTracker.h"
#include "dsp/OnsetDetector.h"
#include "dsp/PhraseTracker.h"
#include "dsp/ChaosEngine.h"

class SkellAdlibLabProcessor final : public juce::AudioProcessor {
public:
 SkellAdlibLabProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName()const override {return "Skell AdlibLab";}
 bool acceptsMidi()const override {return true;}
 bool producesMidi()const override {return false;}
 bool isMidiEffect()const override {return false;}
 double getTailLengthSeconds()const override{return 8.0;}
 int getNumPrograms()override{return 1;}
 int getCurrentProgram()override{return 0;}
 void setCurrentProgram(int)override{}
 const juce::String getProgramName(int)override{return {};}
 void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;
 void setStateInformation(const void*,int)override;
 static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

 juce::AudioProcessorValueTreeState parameters;
 adlib::ClipBank clips;
 adlib::ClipBank::Clip const* getMeltClip() const {return meltSample.load();}
 void loadWav(const juce::File&,int padOrMelt);
 void clearPad(int pad){clips.clear(pad);}
 void triggerPad(int pad){auditionPadRequested.store(pad);}
 void requestMidiOn(int note){midiNoteRequested.store(note);}
 void requestMidiOff(int note){midiOffRequested.store(note);}
 void armRecording(int pad){if(juce::isPositiveAndBelow(pad,8))recordPadRequested.store(pad);}
 bool collectRecordedClip();
 int getThemeIndex()const{return juce::jlimit(0,3,(int)parameters.state.getProperty("themeIndex",0));}
 void setThemeIndex(int theme){parameters.state.setProperty("themeIndex",juce::jlimit(0,3,theme),nullptr);}
 void reRoll(){reRollRequested.store(true);}
 void triggerThrow(){throwRequested.store(true);}
 void setModuleOrder(std::array<int,4>);
 std::array<int,4> getModuleOrder()const;
 void setVoicePosition(int i,float pan,float pitch){route.stack.setVoicePosition(i,pan,pitch);}
 float getVoicePan(int i)const{return route.stack.getVoicePan(i);}
 float getVoicePitch(int i)const{return route.stack.getVoicePitch(i);}
 float getInputLevel()const{return inputLevel.load();}
 float getOutputLevel()const{return outputLevel.load();}
 float getPitchHz()const{return trackedPitch.load();}
 float getThrowActivity()const{return throwActivity.load();}
 int getNote()const{return activeMidiNote.load();}
 adlib::Params loadParams()const;
private:
 adlib::ChainRouter route;
 adlib::ChaosEngine chaos;
 adlib::PitchTracker pitchTracker;
 adlib::OnsetDetector onset;
 adlib::PhraseTracker phrase;
 juce::AudioFormatManager formats;
 std::atomic<const adlib::ClipBank::Clip*>meltSample{nullptr};
 std::vector<std::unique_ptr<adlib::ClipBank::Clip>>retainedMelt;
 std::array<std::atomic<int>,4>moduleOrder{{0,1,2,3}};
 std::atomic<bool>reRollRequested{false},throwRequested{false};
 std::atomic<int>auditionPadRequested{-1},midiNoteRequested{-1},midiOffRequested{-1},recordPadRequested{-1},recordReadyPad{-1};
 std::atomic<int>recordReadyCount{0};
 std::array<std::vector<float>,8>captureBuffers;
 int recordActive=-1,recordCount=0,recordSilent=0;bool recordStarted=false;
 std::atomic<float>inputLevel{0},outputLevel{0},trackedPitch{0},throwActivity{0};
 std::atomic<int>activeMidiNote{-1};
 double sampleRate=44100;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> mixSmooth,gainSmooth;
 void publishMelt(std::unique_ptr<adlib::ClipBank::Clip>);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellAdlibLabProcessor)
};
