#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstring>

namespace {
using Layout=juce::AudioProcessorValueTreeState::ParameterLayout;
void f(Layout& l,const char* id,const char* name,float lo,float hi,float def,float step=0.001f,float skew=1.f){
 juce::NormalisableRange<float>range(lo,hi,step,skew);
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,range,def));
}
void b(Layout&l,const char*id,const char*name,bool def){l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,def));}
void c(Layout&l,const char*id,const char*name,std::initializer_list<const char*>choices,int def){
 juce::StringArray labels;for(auto* label:choices)labels.add(label);
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},name,labels,def));
}
constexpr int stateMagic=0xAD1B1AB;
}

Layout SkellAdlibLabProcessor::createLayout(){
 Layout l;
 f(l,"mix","Mix",0,1,.65f);f(l,"outputGain","Output Gain",-12,12,0);
 f(l,"width","Stereo Width",0,2,1);f(l,"chaos","Chaos",0,1,.25f);
 b(l,"chaosLock","Chaos Lock",false);b(l,"safetyLimiter","Safety Soft Clip",true);
 c(l,"latencyMode","Quality Mode",{"Low","Normal","High Quality"},0);
 b(l,"stackOn","Stack Enabled",true);c(l,"stackVoices","Stack Voices",{"2","3","4","5","6","7","8"},1);
 f(l,"stackSpread","Stack Spread (cents)",0,1200,30,1);
 f(l,"stackFormant","Stack Formant",-12,12,0,.01f);
 f(l,"stackTime","Stack Delay (ms)",0,80,22,.1f);
 f(l,"stackWidth","Stack Width",0,1,.75f);
 f(l,"stackChaosAmt","Stack Chaos Amount",0,1,.45f);
 b(l,"stackRerollOnHit","Re-roll On Hit",true);b(l,"stackScaleLock","Scale Lock",false);
 c(l,"stackKey","Scale Key",{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},0);
 c(l,"stackScale","Scale",{"Major","Minor","Chromatic","Pentatonic"},0);
 b(l,"charOn","Character Enabled",true);
 c(l,"charType","Character",{"Alien","Robot","Demon","Cartoon","Ghost","Gremlin"},0);
 f(l,"charIntensity","Character Intensity",0,1,.35f);f(l,"charFormant","Character Formant",-12,12,0,.01f);
 f(l,"charGlide","Character Glide (ms)",0,500,50,1);
 f(l,"charCrush","Bit Depth",3,16,12,1);f(l,"charDetune","Character Detune",0,1,.2f);
 b(l,"throwOn","Throw Enabled",true);
 c(l,"throwMode","Throw Trigger",{"Last Word","Syllable","Manual","Always"},0);
 c(l,"throwDiv","Throw Division",{"1/4","1/8","1/16","1/8.","1/8T","1/32","1/64"},2);
 f(l,"throwFeedback","Throw Feedback",0,.99f,.55f);
 f(l,"throwRepeats","Throw Repeats",1,16,4,1);
 f(l,"throwFilterStart","Throw Filter Start",20,20000,12000,0,0.27f);
 f(l,"throwFilterEnd","Throw Filter End",20,20000,1000,0,0.27f);
 f(l,"throwPitchPerRepeat","Throw Pitch per Repeat",-12,12,0,.1f);
 f(l,"throwDuck","Throw Ducking",0,1,.4f);
 b(l,"throwReverse","Throw Reverse",false);b(l,"throwPingPong","Throw Ping Pong",true);
 b(l,"meltOn","Melt Enabled",false);
 c(l,"meltMode","Melt Mode",{"Vocoder","Pitch Follow","Morph"},0);
 f(l,"meltAmount","Melt Amount",0,1,.4f);
 f(l,"meltFollowPitch","Follow Pitch",0,1,.5f);
 f(l,"meltFollowLoudness","Follow Loudness",0,1,.8f);
 f(l,"meltSmooth","Melt Smoothing",0,1,.4f);
 f(l,"meltRoot","Melt Root Note",24,96,60,1);
 f(l,"midiChannel","MIDI Channel (0=Omni)",0,16,0,1);
 f(l,"kbRoot","Keyboard Root",24,96,60,1);
 b(l,"kbFormantLock","Keyboard Formant Lock",true);
 b(l,"kbVelIntensity","Velocity to Intensity",true);
 b(l,"kbVelTail","Velocity to Tail",true);
 b(l,"kbAltReverse","Alternate Keys Reverse",false);
 return l;
}
SkellAdlibLabProcessor::SkellAdlibLabProcessor():AudioProcessor(
 BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 parameters(*this,nullptr,"PARAMETERS",createLayout()) {
 formats.registerBasicFormats();
}

bool SkellAdlibLabProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
 auto input=l.getMainInputChannelSet();
 return (input==juce::AudioChannelSet::mono()||input==juce::AudioChannelSet::stereo())&&input==l.getMainOutputChannelSet();
}

void SkellAdlibLabProcessor::prepareToPlay(double sr,int){
 sampleRate=sr;
 onset.prepare(sr);phrase.prepare(sr);pitchTracker.prepare(sr);route.prepare(sr);clips.prepare(sr);
 for(auto&v:captureBuffers)v.assign((size_t)juce::jmax(2,(int)(sr*5.0)),0.0f);
 recordActive=-1;recordCount=recordSilent=0;recordStarted=false;recordReadyPad.store(-1);
 mixSmooth.reset(sr,.025);gainSmooth.reset(sr,.025);
 mixSmooth.setCurrentAndTargetValue(parameters.getRawParameterValue("mix")->load());
 gainSmooth.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(parameters.getRawParameterValue("outputGain")->load()));
 // Pitch-shifted STACK voices are delayed intentionally, like a doubler. Dry remains zero-latency.
 setLatencySamples(0);
}

adlib::Params SkellAdlibLabProcessor::loadParams()const{
 adlib::Params p;
 const auto val=[this](const char*id)->float{return parameters.getRawParameterValue(id)->load();};
 const auto yes=[&](const char*id)->bool{return val(id)>=.5f;};
 const auto integer=[&](const char*id)->int{return (int)std::lround(val(id));};
 p.mix=val("mix");p.outputGain=val("outputGain");p.width=val("width");p.chaos=val("chaos");p.chaosLock=yes("chaosLock");p.safetyLimiter=yes("safetyLimiter");p.latencyMode=integer("latencyMode");
 p.stackOn=yes("stackOn");p.stackVoices=integer("stackVoices")+2;p.stackSpread=val("stackSpread");p.stackFormant=val("stackFormant");p.stackTime=val("stackTime");p.stackWidth=val("stackWidth");p.stackChaosAmt=val("stackChaosAmt");p.stackRerollOnHit=yes("stackRerollOnHit");p.stackScaleLock=yes("stackScaleLock");p.stackKey=integer("stackKey");p.stackScale=integer("stackScale");
 p.charOn=yes("charOn");p.charType=integer("charType");p.charIntensity=val("charIntensity");p.charFormant=val("charFormant");p.charGlide=val("charGlide");p.charCrush=val("charCrush");p.charDetune=val("charDetune");
 p.throwOn=yes("throwOn");p.throwMode=integer("throwMode");p.throwDiv=integer("throwDiv");p.throwFeedback=val("throwFeedback");p.throwRepeats=integer("throwRepeats");p.throwFilterStart=val("throwFilterStart");p.throwFilterEnd=val("throwFilterEnd");p.throwPitchPerRepeat=val("throwPitchPerRepeat");p.throwDuck=val("throwDuck");p.throwReverse=yes("throwReverse");p.throwPingPong=yes("throwPingPong");
 p.meltOn=yes("meltOn");p.meltMode=integer("meltMode");p.meltAmount=val("meltAmount");p.meltFollowPitch=val("meltFollowPitch");p.meltFollowLoudness=val("meltFollowLoudness");p.meltSmooth=val("meltSmooth");p.meltRoot=integer("meltRoot");
 p.midiChannel=integer("midiChannel");p.kbRoot=integer("kbRoot");p.kbFormantLock=yes("kbFormantLock");p.kbVelIntensity=yes("kbVelIntensity");p.kbVelTail=yes("kbVelTail");p.kbAltReverse=yes("kbAltReverse");
 return p;
}

std::array<int,4>SkellAdlibLabProcessor::getModuleOrder()const{
 std::array<int,4> result{};for(int i=0;i<4;++i)result[(size_t)i]=moduleOrder[(size_t)i].load();return result;
}
void SkellAdlibLabProcessor::setModuleOrder(std::array<int,4> order){
 std::array<bool,4> seen{{false,false,false,false}};
 for(int id:order){if(!juce::isPositiveAndBelow(id,4)||seen[(size_t)id])return;seen[(size_t)id]=true;}
 for(int i=0;i<4;++i)moduleOrder[(size_t)i].store(order[(size_t)i]);
 juce::String text;for(int i=0;i<4;++i){if(i)text<<",";text<<order[(size_t)i];}
 parameters.state.setProperty("moduleOrder",text,nullptr);
}

void SkellAdlibLabProcessor::processBlock(juce::AudioBuffer<float>&buffer,juce::MidiBuffer&midi){
 juce::ScopedNoDenormals noDenormals;
 const int n=buffer.getNumSamples(),channels=buffer.getNumChannels();
 if(channels==0||n==0)return;
 auto p=loadParams();
 if(auto*playHead=getPlayHead()) if(auto pos=playHead->getPosition()) if(auto bpm=pos->getBpm())p.bpm=(float)*bpm;
 route.setOrder(getModuleOrder());
 const auto*sample=meltSample.load(std::memory_order_acquire);
 mixSmooth.setTargetValue(p.mix);
 gainSmooth.setTargetValue(juce::Decibels::decibelsToGain(p.outputGain));
 float*left=buffer.getWritePointer(0);
 float*right=channels>1?buffer.getWritePointer(1):nullptr;
 float inMeter=0,outMeter=0;
 juce::MidiBuffer::Iterator it(midi);
 juce::MidiMessage message;int midiPosition=0;
 bool hasMidi=it.getNextEvent(message,midiPosition);
 if(reRollRequested.exchange(false))chaos.reRoll(p.chaosLock);
 if(throwRequested.exchange(false))route.throwEngine.trigger(p);
 if(int pad=auditionPadRequested.exchange(-1);juce::isPositiveAndBelow(pad,8))clips.noteOn(p.kbRoot+pad,1,p);
 if(int note=midiNoteRequested.exchange(-1);juce::isPositiveAndBelow(note,128)){clips.noteOn(note,1,p);activeMidiNote.store(note);}
 if(int note=midiOffRequested.exchange(-1);juce::isPositiveAndBelow(note,128)){clips.noteOff(note);activeMidiNote.store(-1);}
 if(int pad=recordPadRequested.exchange(-1);juce::isPositiveAndBelow(pad,8)&&recordReadyPad.load()<0){recordActive=pad;recordCount=recordSilent=0;recordStarted=false;}
 for(int i=0;i<n;++i){
  while(hasMidi&&midiPosition<=i){
   if(p.midiChannel==0||message.getChannel()==p.midiChannel){
    if(message.isNoteOn()){
     activeMidiNote.store(message.getNoteNumber());clips.noteOn(message.getNoteNumber(),message.getFloatVelocity(),p);
     if(p.throwMode==2)route.throwEngine.trigger(p);
    }else if(message.isNoteOff()){
     clips.noteOff(message.getNoteNumber());activeMidiNote.store(-1);
    }
   }
   hasMidi=it.getNextEvent(message,midiPosition);
  }
  adlib::Stereo dry{left[i],right?right[i]:left[i]};
  float mono=(dry.l+dry.r)*.5f;
  inMeter=juce::jmax(inMeter,std::abs(mono));
  pitchTracker.tick(mono);
  auto events=onset.tick(mono);
  phrase.tick(events);
  if(recordActive>=0){
   if(events.envelope>0.014f)recordStarted=true;
   if(recordStarted){
    auto& dest=captureBuffers[(size_t)recordActive];
    if(recordCount<(int)dest.size())dest[(size_t)recordCount++]=mono;
    if(events.envelope<0.005f)++recordSilent;else recordSilent=0;
    if(recordCount>256&&(recordSilent>(int)(sampleRate*.16)||recordCount>=(int)dest.size())){
     recordReadyCount.store(recordCount);recordReadyPad.store(recordActive,std::memory_order_release);
     recordActive=-1;recordStarted=false;
    }
   }
  }
  events.trackedHz=pitchTracker.getHz();
  if(events.hit&&p.stackRerollOnHit&&p.chaos>0)chaos.reRoll(p.chaosLock);
  adlib::Stereo input=dry;
  input+=clips.tick();
  auto effected=route.process(input,p,events,chaos,sample);
  auto mix=mixSmooth.getNextValue();
  adlib::Stereo output{juce::jmap(mix,dry.l,effected.l),juce::jmap(mix,dry.r,effected.r)};
  float middle=(output.l+output.r)*.5f,side=(output.l-output.r)*.5f*p.width;
  float gain=gainSmooth.getNextValue();
  output.l=(middle+side)*gain;output.r=(middle-side)*gain;
  // Non-lookahead safety soft clip; this is not a true peak limiter.
  if(p.safetyLimiter){output.l=std::tanh(output.l);output.r=std::tanh(output.r);}
  left[i]=output.l;if(right)right[i]=output.r;
  outMeter=juce::jmax(outMeter,juce::jmax(std::abs(output.l),std::abs(output.r)));
 }
 inputLevel.store(inMeter);outputLevel.store(outMeter);
 trackedPitch.store(pitchTracker.getHz());throwActivity.store(route.throwEngine.activity());
}

bool SkellAdlibLabProcessor::collectRecordedClip(){
 int pad=recordReadyPad.load(std::memory_order_acquire);
 if(!juce::isPositiveAndBelow(pad,8))return false;
 int count=juce::jlimit(0,(int)captureBuffers[(size_t)pad].size(),recordReadyCount.load());
 auto clip=std::make_unique<adlib::ClipBank::Clip>();
 clip->name="REC "+juce::String(pad+1);
 // Convert captured host-rate audio to the fixed 44.1 kHz patch storage rate.
 int outputCount=juce::jlimit(2,220500,(int)std::round((double)count*44100.0/sampleRate));
 clip->mono.resize((size_t)outputCount);
 for(int i=0;i<outputCount;++i){
  auto src=(double)i*sampleRate/44100.0;
  auto n=juce::jlimit(0,juce::jmax(0,count-2),(int)src);
  float t=(float)(src-n);
  clip->mono[(size_t)i]=juce::jmap(t,captureBuffers[(size_t)pad][(size_t)n],captureBuffers[(size_t)pad][(size_t)juce::jmin(n+1,count-1)]);
 }
 clips.publish(pad,std::move(clip));
 recordReadyPad.store(-1,std::memory_order_release);
 return true;
}

void SkellAdlibLabProcessor::publishMelt(std::unique_ptr<adlib::ClipBank::Clip> clip){
 if(!clip)return;
 const auto*ptr=clip.get();retainedMelt.push_back(std::move(clip));
 meltSample.store(ptr,std::memory_order_release);
}

void SkellAdlibLabProcessor::loadWav(const juce::File&file,int padOrMelt){
 if(!file.existsAsFile())return;
 std::unique_ptr<juce::AudioFormatReader>reader(formats.createReaderFor(file));
 if(!reader||reader->lengthInSamples<2||reader->sampleRate<=0)return;
 auto sourceLength=juce::jmin((juce::int64)(reader->sampleRate*5.0),reader->lengthInSamples);
 if(reader->lengthInSamples>sourceLength)
  juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
    "AdlibLab sample limit","Only the first 5 seconds of this sample will be loaded. Keep clips short to limit patch size.");
 juce::AudioBuffer<float>temp((int)juce::jmin(2u,reader->numChannels),(int)sourceLength);
 if(!reader->read(&temp,0,(int)sourceLength,0,true,true))return;
 auto sample=std::make_unique<adlib::ClipBank::Clip>();
 sample->name=file.getFileNameWithoutExtension();
 auto destLength=juce::jlimit(2,220500,(int)std::round((double)sourceLength*44100.0/reader->sampleRate));
 sample->mono.resize((size_t)destLength);
 for(int i=0;i<destLength;++i){
  double src=(double)i*reader->sampleRate/44100.0;
  int n=juce::jlimit(0,(int)sourceLength-2,(int)src);
  float f=(float)(src-n);
  float a=temp.getSample(0,n),b=temp.getSample(0,n+1);
  if(temp.getNumChannels()>1){a=(a+temp.getSample(1,n))*.5f;b=(b+temp.getSample(1,n+1))*.5f;}
  sample->mono[(size_t)i]=juce::jmap(f,a,b);
 }
 if(padOrMelt<0){publishMelt(std::move(sample));parameters.state.setProperty("meltSamplePath",file.getFullPathName(),nullptr);}
 else if(padOrMelt<8)clips.publish(padOrMelt,std::move(sample));
}

void SkellAdlibLabProcessor::getStateInformation(juce::MemoryBlock&dest){
 juce::MemoryOutputStream out;
 out.writeInt(stateMagic);out.writeInt(1);
 auto tree=parameters.copyState();
 juce::String ord;for(int i=0;i<4;++i){if(i)ord<<",";ord<<moduleOrder[(size_t)i].load();}
 tree.setProperty("moduleOrder",ord,nullptr);
 for(int i=0;i<8;++i){tree.setProperty("voicePan"+juce::String(i),route.stack.getVoicePan(i),nullptr);tree.setProperty("voicePitch"+juce::String(i),route.stack.getVoicePitch(i),nullptr);}
 tree.writeToStream(out);
 for(int i=0;i<8;++i){
  const auto*clip=clips.getClip(i);
  auto size=clip?juce::jmin((int)clip->mono.size(),220500):0;
  out.writeInt(size);
  out.writeString(clip?clip->name:juce::String());
  if(size)out.write(clip->mono.data(),(size_t)size*sizeof(float));
 }
 const auto*mel=meltSample.load();
 auto size=mel?juce::jmin((int)mel->mono.size(),220500):0;
 out.writeInt(size);out.writeString(mel?mel->name:juce::String());
 if(size)out.write(mel->mono.data(),(size_t)size*sizeof(float));
 dest.replaceAll(out.getData(),out.getDataSize());
}

void SkellAdlibLabProcessor::setStateInformation(const void*data,int bytes){
 if(data==nullptr||bytes<8)return;
 juce::MemoryInputStream in(data,(size_t)bytes,false);
 if(in.readInt()!=stateMagic||in.readInt()!=1)return;
 auto tree=juce::ValueTree::readFromStream(in);
 if(!tree.isValid()||tree.getType()!=parameters.state.getType())return;
 auto arr=juce::StringArray::fromTokens(tree.getProperty("moduleOrder","0,1,2,3").toString(),",","");
 if(arr.size()==4){std::array<int,4>order{};for(int i=0;i<4;++i)order[(size_t)i]=arr[i].getIntValue();
  std::array<bool,4>seen{};bool valid=true;for(int x:order){if(!juce::isPositiveAndBelow(x,4)||seen[(size_t)x])valid=false;else seen[(size_t)x]=true;}
  if(valid)for(int i=0;i<4;++i)moduleOrder[(size_t)i].store(order[(size_t)i]);
 }
 parameters.replaceState(tree);
 for(int i=0;i<8;++i){
  float pan=(float)tree.getProperty("voicePan"+juce::String(i),route.stack.getVoicePan(i));
  float pitch=(float)tree.getProperty("voicePitch"+juce::String(i),route.stack.getVoicePitch(i));
  route.stack.setVoicePosition(i,pan,pitch);
 }
 for(int i=0;i<9;++i){
  if(in.getNumBytesRemaining()<4)break;
  auto count=in.readInt();auto name=in.readString();
  if(count<0||count>220500||in.getNumBytesRemaining()<(juce::int64)count*(int)sizeof(float))break;
  if(count==0){if(i<8)clips.clear(i);else meltSample.store(nullptr);continue;}
  auto clip=std::make_unique<adlib::ClipBank::Clip>();clip->name=name;
  clip->mono.resize((size_t)count);
  const int byteCount=count*(int)sizeof(float);
  if(in.read(clip->mono.data(),byteCount)!=byteCount)break;
  if(i<8)clips.publish(i,std::move(clip));else publishMelt(std::move(clip));
 }
}
juce::AudioProcessorEditor*SkellAdlibLabProcessor::createEditor(){return new SkellAdlibLabEditor(*this);}
juce::AudioProcessor*JUCE_CALLTYPE createPluginFilter(){return new SkellAdlibLabProcessor();}
