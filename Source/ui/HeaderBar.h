#pragma once
#include "Theme.h"
#include "SkullThemeButton.h"
#include "../PluginProcessor.h"
#include <memory>
// Compact vector gear: no emoji glyphs or machine-dependent icon fonts.
class GearButton final:public juce::Button {
public:
 explicit GearButton(const Theme::Palette&pal):juce::Button("Settings"),palette(pal){
  setTooltip("Plugin settings");
 }
 void paintButton(juce::Graphics&g,bool hover,bool down)override{
  auto r=getLocalBounds().toFloat().reduced(1.f);
  g.setColour(down?palette.panel:palette.raised.brighter(hover?.12f:0.f));g.fillRoundedRectangle(r,7.f);
  g.setColour(hover?palette.stack:palette.border);g.drawRoundedRectangle(r,7.f,1.f);
  auto c=r.getCentre();g.setColour(hover?palette.stack:palette.text);
  for(int i=0;i<8;++i){float a=(float)i*juce::MathConstants<float>::pi/4.f;
   g.drawLine(c.x+8.f*std::sin(a),c.y-8.f*std::cos(a),
              c.x+11.f*std::sin(a),c.y-11.f*std::cos(a),2.4f);
  }
  g.drawEllipse(c.x-7.f,c.y-7.f,14.f,14.f,1.9f);
  g.drawEllipse(c.x-2.6f,c.y-2.6f,5.2f,5.2f,1.7f);
 }
private:const Theme::Palette&palette;
};
class HeaderBar final:public juce::Component {
public:
 explicit HeaderBar(SkellAdlibLabProcessor&p,const Theme::Palette&pal)
 :processor(p),palette(pal),skull(pal),settings(pal),mixAtt(p.parameters,"mix",mix),outputAtt(p.parameters,"outputGain",output){
  addAndMakeVisible(skull);skull.setBounds(11,9,48,48);
  skull.onClick=[this]{if(onThemeCycle)onThemeCycle();};
  skull.onThemeChosen=[this](int theme){if(onThemeSelect)onThemeSelect(theme);};
  setup(mix,palette.stack);setup(output,palette.melt);
  addAndMakeVisible(mix);mix.setBounds(906,6,64,55);
  addAndMakeVisible(output);output.setBounds(980,6,64,55);
  preset.setButtonText(processor.parameters.state.getProperty("presetName","INIT PATCH").toString());preset.setBounds(353,18,326,34);preset.onClick=[this]{if(onPresetClicked)onPresetClicked();};addAndMakeVisible(preset);
  shuffle.setButtonText("SHUFFLE");shuffle.setBounds(690,18,100,34);shuffle.onClick=[this]{shufflePatch();};addAndMakeVisible(shuffle);
  settings.setBounds(1190,17,42,36);settings.onClick=[this]{showSettings();};addAndMakeVisible(settings);
 }
 std::function<void()> onThemeCycle;
 std::function<void(int)> onThemeSelect;
 std::function<void()> onPresetClicked;
 std::function<void()> onShowTutorial;
 void refreshPresetLabel(){preset.setButtonText(processor.parameters.state.getProperty("presetName","INIT PATCH").toString());}
 juce::String getPresetLabel()const{return preset.getButtonText();}
 void refreshTheme(){setup(mix,palette.stack);setup(output,palette.melt);skull.repaint();repaint();}
 void paint(juce::Graphics&g)override{
  const auto r=getLocalBounds().toFloat();
  g.setGradientFill(juce::ColourGradient(palette.raised.withAlpha(.82f),0.f,0.f,
                                        palette.surface,r.getRight(),r.getBottom(),false));
  g.fillRoundedRectangle(r.reduced(.5f),11.f);
  g.setColour(palette.border.withAlpha(.85f));g.drawRoundedRectangle(r.reduced(.9f),10.5f,1.f);
  g.setColour(palette.stack.withAlpha(.88f));g.fillRoundedRectangle(69.f,58.f,180.f,2.f,1.f);
  g.setColour(palette.muted);g.setFont(Theme::mono(10));
  g.drawText("SKELL AUDIO   /   VOCAL DESIGN",72,7,270,17,juce::Justification::left);
  // Explicit geometry is stable even when the headless font resolver has no
  // glyph advance metrics (which previously made ADLIB disappear entirely).
  g.setFont(Theme::display(28.f));g.setColour(palette.text);
  g.drawText("ADLIB",70,23,110,34,juce::Justification::left);
  g.setColour(palette.stack);g.drawText("LAB",155,23,76,34,juce::Justification::left);
  g.setColour(palette.muted);g.setFont(Theme::mono(9));
  g.drawText(palette.name,242,36,86,16,juce::Justification::left);
  g.setColour(palette.border.withAlpha(.85f));
  g.drawVerticalLine(347,15.f,55.f);g.drawVerticalLine(802,15.f,55.f);g.drawVerticalLine(1064,15.f,55.f);
  g.setColour(palette.muted);g.setFont(Theme::mono(10));
  g.drawText("MIX",911,53,54,12,juce::Justification::centred);
  g.drawText("OUTPUT",982,53,60,12,juce::Justification::centred);
  g.drawText("INPUT",1076,9,72,15,juce::Justification::centredLeft);
  const int lit=juce::roundToInt(12.f*juce::jlimit(0.f,1.f,processor.getInputLevel()));
  for(int i=0;i<12;++i){
   g.setColour((i<lit?(i>9?palette.chaos:palette.melt):palette.border).withAlpha(i<lit?1.f:.5f));
   g.fillRoundedRectangle(1076.f+i*7.25f,31.f,5.2f,7.f,1.4f);
  }
 }


 // Used by both the real dropdown and the JUCE renderer interaction tests.
 void applyPresetSelection(int result){
  if(result==20){savePreset();return;}
  if(result==21){loadPreset();return;}
  if(result<1||result>10)return;
  resetFactoryPatch();
  juce::String name="INIT PATCH";
  switch(result){
   case 2: name="WIDE ADLIB STACK";
    parameter("stackVoices",4);parameter("stackSpread",370);parameter("stackTime",35);
    parameter("stackWidth",.94f);parameter("stackChaosAmt",.52f);
    parameter("charOn",0);parameter("throwOn",0);parameter("mix",.88f);break;
   case 3: name="ROBOT INCOMING";
    parameter("charType",1);parameter("charIntensity",.91f);parameter("charFormant",-3);
    parameter("charCrush",6);parameter("charDetune",.16f);
    parameter("stackOn",0);parameter("throwOn",0);parameter("mix",.96f);break;
   case 4: name="GHOST THROWS";
    parameter("stackOn",0);parameter("charType",4);parameter("charIntensity",.72f);
    parameter("throwDiv",3);parameter("throwFeedback",.78f);parameter("throwRepeats",8);
    parameter("throwFilterStart",11000);parameter("throwFilterEnd",950);
    parameter("throwDuck",.33f);parameter("mix",.9f);break;
   case 5: name="DEMON DOUBLES";
    parameter("stackVoices",2);parameter("stackSpread",220);parameter("stackTime",27);
    parameter("charType",2);parameter("charIntensity",.73f);parameter("charCrush",10);
    parameter("mix",.85f);break;
   case 6: name="CARTOON PUNCH";
    parameter("stackOn",0);parameter("charType",3);parameter("charIntensity",.78f);
    parameter("charGlide",20);parameter("charDetune",.32f);
    parameter("throwMode",1);parameter("throwDiv",2);parameter("throwFeedback",.34f);
    parameter("mix",.9f);break;
   case 7: name="BONE CHOP";
    processor.setRackTypes({adlib::RackType::Stack,adlib::RackType::Throw,
       adlib::RackType::Chop,adlib::RackType::Filter});
    parameter("slot2Amount",.94f);parameter("slot2Rate",.79f);parameter("slot2Tone",.77f);
    parameter("slot3Tone",.29f);parameter("slot3Amount",.68f);
    parameter("mix",.88f);break;
   case 8: name="NEON MOTION";
    processor.setRackTypes({adlib::RackType::Stack,adlib::RackType::Motion,
       adlib::RackType::Throw,adlib::RackType::Saturate});
    parameter("slot1Amount",.72f);parameter("slot1Rate",.39f);
    parameter("slot3Amount",.28f);parameter("slot3Tone",.74f);
    parameter("mix",.9f);break;
   case 9: name="CRUSHED RADIO";
    processor.setRackTypes({adlib::RackType::Filter,adlib::RackType::Character,
       adlib::RackType::Throw,adlib::RackType::Saturate});
    parameter("slot0Amount",.9f);parameter("slot0Tone",.24f);
    parameter("slot3Amount",.82f);parameter("charType",1);parameter("charIntensity",.66f);
    parameter("mix",.95f);break;
   case 10: name="STUTTER GHOST";
    processor.setRackTypes({adlib::RackType::Chop,adlib::RackType::Motion,
       adlib::RackType::Throw,adlib::RackType::Filter});
    parameter("slot0Rate",.98f);parameter("slot0Tone",.7f);
    parameter("slot1Amount",.31f);parameter("slot1Rate",.29f);
    parameter("throwFeedback",.72f);parameter("throwRepeats",7);
    parameter("mix",.96f);break;
  }
  processor.parameters.state.setProperty("presetName",name,nullptr);
  preset.setButtonText(name);
 }
 void shufflePatch(){
  // Message thread only. Undo restores rack types, order and APVTS state.
  processor.getStateInformation(undoShuffleSnapshot);
  auto& random=juce::Random::getSystemRandom();
  std::array<adlib::RackType,7> library{{adlib::RackType::Stack,
    adlib::RackType::Character,adlib::RackType::Throw,adlib::RackType::Chop,
    adlib::RackType::Motion,adlib::RackType::Filter,adlib::RackType::Saturate}};
  for(int i=6;i>0;--i)std::swap(library[(size_t)i],library[(size_t)random.nextInt(i+1)]);
  adlib::RackTypes next{{library[0],library[1],library[2],library[3]}};
  if(next==processor.getRackTypes())std::swap(next[0],next[1]);
  processor.setRackTypes(next);
  std::array<int,4> order{{0,1,2,3}};
  for(int i=3;i>0;--i)std::swap(order[(size_t)i],order[(size_t)random.nextInt(i+1)]);
  if(order==processor.getModuleOrder())std::swap(order[0],order[1]);
  processor.setModuleOrder(order);
  parameter("stackOn",1);parameter("charOn",1);parameter("throwOn",1);
  parameter("stackVoices",(float)random.nextInt(7));
  parameter("stackSpread",110.f+random.nextFloat()*780.f);
  parameter("stackTime",6.f+random.nextFloat()*53.f);
  parameter("stackWidth",.32f+random.nextFloat()*.68f);
  parameter("stackChaosAmt",.4f+random.nextFloat()*.6f);
  parameter("charType",(float)random.nextInt(6));
  parameter("charIntensity",.45f+random.nextFloat()*.5f);
  parameter("charCrush",(float)(4+random.nextInt(10)));
  parameter("charDetune",random.nextFloat()*.65f);
  parameter("throwDiv",(float)random.nextInt(7));
  parameter("throwFeedback",.25f+random.nextFloat()*.57f);
  parameter("throwRepeats",(float)(2+random.nextInt(9)));
  parameter("throwDuck",random.nextFloat()*.55f);
  for(int i=0;i<4;++i){
   const juce::String prefix="slot"+juce::String(i);
   parameter((prefix+"On").toRawUTF8(),1.f);
   parameter((prefix+"Amount").toRawUTF8(),.25f+random.nextFloat()*.73f);
   parameter((prefix+"Rate").toRawUTF8(),random.nextFloat());
   parameter((prefix+"Tone").toRawUTF8(),random.nextFloat());
   parameter((prefix+"Mix").toRawUTF8(),.5f+random.nextFloat()*.5f);
  }
  parameter("chaos",.35f+random.nextFloat()*.5f);
  processor.reRoll();
  processor.parameters.state.setProperty("presetName","SHUFFLED PATCH",nullptr);
  preset.setButtonText("SHUFFLED PATCH");
 }
 void undoLastShuffle(){
  if(undoShuffleSnapshot.isEmpty())return;
  processor.setStateInformation(undoShuffleSnapshot.getData(),(int)undoShuffleSnapshot.getSize());
  undoShuffleSnapshot.reset();
  refreshPresetLabel();
 }

private:
 static void setup(juce::Slider&s,juce::Colour colour){s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);s.setColour(juce::Slider::rotarySliderFillColourId,colour);}
 void parameter(const char*id,float value){
  if(auto*r=dynamic_cast<juce::RangedAudioParameter*>(processor.parameters.getParameter(id))){r->beginChangeGesture();r->setValueNotifyingHost(r->convertTo0to1(value));r->endChangeGesture();}
 }
 // Restore the actual APVTS defaults, not just a few cosmetic controls.
 // Keep samples and the chosen visual skin, which belong to project/UI state.
 void resetFactoryPatch(){
  for(auto* base:processor.getParameters()){
   if(auto* parameter=dynamic_cast<juce::RangedAudioParameter*>(base)){
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->getDefaultValue());
    parameter->endChangeGesture();
   }
  }
  processor.setModuleOrder({{0,1,2,3}});
  processor.setRackTypes({adlib::RackType::Stack,adlib::RackType::Character,
                         adlib::RackType::Throw,adlib::RackType::Chop});
 }
 void savePreset(){
  chooser=std::make_unique<juce::FileChooser>("Save AdlibLab patch",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("AdlibLab.skad"),"*.skad");
  juce::Component::SafePointer<HeaderBar>safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::warnAboutOverwriting,[safe](const juce::FileChooser&f){if(!safe)return;auto file=f.getResult();if(file==juce::File{})return;
   juce::MemoryBlock state;safe->processor.getStateInformation(state);auto stream=file.createOutputStream();if(stream){stream->setPosition(0);stream->truncate();stream->write(state.getData(),state.getSize());}
  });
 }
 void loadPreset(){
  chooser=std::make_unique<juce::FileChooser>("Load AdlibLab patch",juce::File{},"*.skad");
  juce::Component::SafePointer<HeaderBar>safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser&f){if(!safe)return;auto file=f.getResult();if(!file.existsAsFile()||file.getSize()>10000000)return;juce::MemoryBlock data;if(file.loadFileAsData(data)){safe->processor.setStateInformation(data.getData(),(int)data.getSize());safe->refreshPresetLabel();}});
 }
 void showSettings(){
  juce::PopupMenu menu;
  menu.addSectionHeader("ADLIBLAB / SETTINGS");
  menu.addItem(1,"About Skell AdlibLab");
  menu.addItem(2,"MIDI keyboard routing");
  menu.addSeparator();
  menu.addItem(3,"Quality modes (not yet implemented)",false,false);
  menu.addItem(4,"Undo last shuffle",!undoShuffleSnapshot.isEmpty());
  juce::Component::SafePointer<HeaderBar>safe(this);
  menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&settings).withMinimumWidth(255),
   [safe](int result){
    if(!safe)return;
    if(result==1)juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
      "Skell AdlibLab","Windows VST3. Quality mode selection is not connected to DSP yet.");
    if(result==4)safe->undoLastShuffle();
    if(result==2)juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
      "MIDI routing","Route MIDI to AdlibLab in your DAW. Note keys trigger loaded clip pads; SHIFT+click a pad records the next phrase.");
   });
 }
 juce::MemoryBlock undoShuffleSnapshot;
 SkellAdlibLabProcessor&processor;const Theme::Palette&palette;SkullThemeButton skull;GearButton settings;juce::Slider mix,output;
 juce::AudioProcessorValueTreeState::SliderAttachment mixAtt,outputAtt;
 juce::TextButton preset,shuffle;std::unique_ptr<juce::FileChooser>chooser;
};
