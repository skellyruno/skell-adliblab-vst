#include "PluginEditor.h"
SkellAdlibLabEditor::Canvas::Canvas(SkellAdlibLabProcessor&p,SkellAdlibLabEditor&ed)
 :processor(p),editor(ed),palette(Theme::makePalette(p.getThemeIndex())),themeIndex(p.getThemeIndex()),
 header(p,palette),stack(p,palette),character(p,palette),throwPanel(p,palette),melt(p,palette),
 effectPicker(palette),chaos(p,palette),bank(p,palette),keyboard(p,palette),presetMenu(palette),
 widthAtt(p.parameters,"width",width),gainAtt(p.parameters,"outputGain",gain),
 limiterAtt(p.parameters,"safetyLimiter",limiter),formantAtt(p.parameters,"kbFormantLock",formant),
 velIntensityAtt(p.parameters,"kbVelIntensity",velIntensity),velTailAtt(p.parameters,"kbVelTail",velTail),reverseAtt(p.parameters,"kbAltReverse",reverse),
 modules{{&stack,&character,&throwPanel,&melt}} {
 setSize(Theme::designWidth,Theme::designHeight);
 addAndMakeVisible(header);header.setBounds(15,8,1250,65);
 header.onThemeCycle=[this]{int next=(themeIndex+1)%4;processor.setThemeIndex(next);applyTheme(next);};
 header.onThemeSelect=[this](int next){processor.setThemeIndex(next);applyTheme(next);};
 header.onPresetClicked=[this]{showPresetMenu(!presetMenu.isVisible());};
 for(int id=0;id<4;++id){
  auto* module=modules[(size_t)id];addAndMakeVisible(module);
  module->requestReorder=[this,id](int,int dx){const int slot=slotForLegacyPanel(id);if(slot>=0)reorder(slot,dx);};
  module->requestRemove=[this,id](int){const int slot=slotForLegacyPanel(id);
    if(slot>=0){processor.setRackType(slot,adlib::RackType::Empty);layoutModules();}};
  module->requestReplace=[this,id](int){openEffectPicker(slotForLegacyPanel(id));};
  module->requestPreview=[this,id](int,int dx,bool active){const int slot=slotForLegacyPanel(id);
    if(slot>=0)previewDrag(slot,dx,active);};
 }
 for(int slot=0;slot<4;++slot){
  extraPanels[(size_t)slot]=std::make_unique<RackExtraPanel>(p,palette,slot);
  emptySlots[(size_t)slot]=std::make_unique<EmptyRackSlot>(palette,slot);
  auto& panel=*extraPanels[(size_t)slot];
  panel.requestReorder=[this,slot](int,int dx){reorder(slot,dx);};
  panel.requestRemove=[this,slot](int){processor.setRackType(slot,adlib::RackType::Empty);layoutModules();};
  panel.requestReplace=[this,slot](int){openEffectPicker(slot);};
  panel.requestPreview=[this,slot](int,int dx,bool active){previewDrag(slot,dx,active);};
  emptySlots[(size_t)slot]->onAdd=[this](int id){openEffectPicker(id);};
  addAndMakeVisible(panel);addAndMakeVisible(*emptySlots[(size_t)slot]);
 }
 layoutModules();
 addAndMakeVisible(chaos);chaos.setBounds(27,567,233,214);
 addAndMakeVisible(bank);bank.setBounds(286,590,708,68);
 addAndMakeVisible(keyboard);keyboard.setBounds(286,666,708,79);
 for(auto*s:{&width,&gain}){
  s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
  s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  addAndMakeVisible(s);
 }
 width.textFromValueFunction=[](double d){return juce::String(juce::roundToInt(d*100.))+"%";};
 gain.textFromValueFunction=[](double d){return juce::String(juce::roundToInt(d))+"dB";};
 width.setBounds(1036,603,76,65);gain.setBounds(1140,603,76,65);
 for(auto* label:{&widthReadout,&gainReadout}){
  label->setJustificationType(juce::Justification::centred);
  label->setFont(Theme::mono(11.f));
  label->setColour(juce::Label::textColourId,palette.text);
  addAndMakeVisible(label);
 }
 widthReadout.setBounds(1028,668,91,19);gainReadout.setBounds(1132,668,91,19);
 width.addListener(this);gain.addListener(this);
 sliderValueChanged(&width);sliderValueChanged(&gain);
 limiter.setButtonText("SOFT CLIP");formant.setButtonText("FORMANT LOCK");
 velIntensity.setButtonText("VEL > INT");velTail.setButtonText("VEL > TAIL");reverse.setButtonText("ALT REVERSE");
 for(auto*toggle:{&limiter,&formant,&velIntensity,&velTail,&reverse})addAndMakeVisible(toggle);
 limiter.setBounds(1053,713,161,25);
 formant.setBounds(286,753,147,25);velIntensity.setBounds(447,753,131,25);
 velTail.setBounds(583,753,130,25);reverse.setBounds(726,753,148,25);
 header.refreshTheme();for(auto*module:modules)module->refreshTheme();
 addAndMakeVisible(presetMenu);presetMenu.setBounds(0,0,Theme::designWidth,Theme::designHeight);
 presetMenu.setVisible(false);
 addAndMakeVisible(effectPicker);effectPicker.setBounds(0,0,Theme::designWidth,Theme::designHeight);
 effectPicker.setVisible(false);
 effectPicker.onChoose=[this](adlib::RackType type){
  if(pickerSlot>=0)processor.setRackType(pickerSlot,type);
  pickerSlot=-1;effectPicker.setVisible(false);layoutModules();
 };
 effectPicker.onDismiss=[this]{pickerSlot=-1;effectPicker.setVisible(false);};
 presetMenu.onChoice=[this](int id){showPresetMenu(false);header.applyPresetSelection(id);};
 presetMenu.onDismiss=[this]{showPresetMenu(false);};
}
void SkellAdlibLabEditor::Canvas::applyTheme(int index){
 themeIndex=juce::jlimit(0,3,index);
 palette=Theme::makePalette(themeIndex);
 header.refreshTheme();chaos.refreshTheme();
 widthReadout.setColour(juce::Label::textColourId,palette.text);
 gainReadout.setColour(juce::Label::textColourId,palette.text);
 for(auto*module:modules)module->refreshTheme();
 for(auto&panel:extraPanels)panel->refreshTheme();
 for(auto&empty:emptySlots)empty->repaint();
 effectPicker.repaint();
 for(auto*s:{&width,&gain}){
  s->setColour(juce::Slider::rotarySliderFillColourId,palette.melt);
  s->setColour(juce::Slider::textBoxTextColourId,palette.text);
 }
 for(auto*t:{&limiter,&formant,&velIntensity,&velTail,&reverse}){
  t->setColour(juce::ToggleButton::textColourId,palette.text);
  t->setColour(juce::ToggleButton::tickColourId,palette.melt);
 }
 editor.applyLookAndFeelPalette(palette);
 bank.repaint();keyboard.repaint();presetMenu.repaint();repaint();
}
std::vector<skellcore::ui::GuidedTourStep> SkellAdlibLabEditor::Canvas::tourSteps() {
  std::vector<skellcore::ui::GuidedTourStep> steps;
  steps.emplace_back(&header,"START WITH A PATCH",
      "Pick a starting patch above, or open Settings to replay this tutorial.");
  steps.emplace_back(&stack,"STACK YOUR VOICE",
      "Create layers and adjust how widely your adlibs spread.");
  steps.emplace_back(&character,"CHANGE CHARACTER",
      "Choose vocal character and tone without changing your original recording.");
  steps.emplace_back(&throwPanel,"ADLIB THROWS",
      "Set timed throws and repeats; use the panel's controls for movement.");
  steps.emplace_back(extraPanels[3].get(),"CHANGE ANY EFFECT",
      "Click its name to replace an effect; X removes it, and + adds another.");
  steps.emplace_back(&bank,"RECORD AND TRIGGER",
      "Use the clip bank and keyboard to perform or trigger captured vocal phrases.",
      std::function<void()>{},&keyboard);
  steps.emplace_back(&chaos,"CONTROL THE CHAOS",
      "Use CHAOS for creative variation; set the final WIDTH and GAIN to taste.");
  return steps;
}
skellcore::ui::GuidedTourPalette SkellAdlibLabEditor::Canvas::tourPalette()const {
  return {palette.stack,palette.surface,palette.text,palette.muted,
          juce::Colour(0xd9000000)};
}
void SkellAdlibLabEditor::Canvas::sliderValueChanged(juce::Slider* slider){
 if(slider==&width)widthReadout.setText(juce::String(juce::roundToInt(width.getValue()*100.))+"%",juce::dontSendNotification);
 if(slider==&gain)gainReadout.setText(juce::String(juce::roundToInt(gain.getValue()))+"dB",juce::dontSendNotification);
}
void SkellAdlibLabEditor::Canvas::showPresetMenu(bool visible){
 if(visible)effectPicker.setVisible(false);
 presetMenu.setVisible(visible);
 if(visible){presetMenu.toFront(false);presetMenu.grabKeyboardFocus();}
 presetMenu.repaint();
}
void SkellAdlibLabEditor::Canvas::openEffectPicker(int slot){
 if(!juce::isPositiveAndBelow(slot,4))return;
 pickerSlot=slot;showPresetMenu(false);
 effectPicker.setVisible(true);effectPicker.toFront(false);effectPicker.grabKeyboardFocus();
}
void SkellAdlibLabEditor::Canvas::showEffectPickerForScreenshot(bool visible){
 if(visible)openEffectPicker(3);
 else {pickerSlot=-1;effectPicker.setVisible(false);}
}
int SkellAdlibLabEditor::Canvas::slotForLegacyPanel(int id)const{
 const auto expected=id==3?adlib::RackType::LegacyMelt:static_cast<adlib::RackType>(id+1);
 const auto types=processor.getRackTypes();
 for(int i=0;i<4;++i)if(types[(size_t)i]==expected)return i;
 return -1;
}
void SkellAdlibLabEditor::Canvas::previewDrag(int slot,int delta,bool active){
 draggingSlot=active?slot:-1;draggingDelta=active?delta:0;
 repaint(20,82,1240,468);
}
juce::StringArray SkellAdlibLabEditor::Canvas::runControlChecks(){
 juce::StringArray errors;
 juce::MemoryBlock saved;processor.getStateInformation(saved);
 const auto value=[this](const char*id){return processor.parameters.getRawParameterValue(id)->load();};
 header.applyPresetSelection(3);
 if((int)value("charType")!=1||value("charIntensity")<.8f||value("stackOn")>.5f)
  errors.add("Robot preset did not update real controls");
 if(header.getPresetLabel()!="ROBOT INCOMING")errors.add("Preset title failed to update");
 const auto oldType=value("charType"),oldSpread=value("stackSpread"),oldChaos=value("chaos");
 header.shufflePatch();
 if(value("chaos")<.3f||value("stackSpread")<100.f||header.getPresetLabel()!="SHUFFLED PATCH")
  errors.add("Shuffle did not change real automatable parameters");
 if(std::abs(oldType-value("charType"))<.5f&&std::abs(oldSpread-value("stackSpread"))<.01f&&std::abs(oldChaos-value("chaos"))<.0001f)
  errors.add("Shuffle left sound controls unchanged");
 header.applyPresetSelection(1);
 if(std::abs(value("chaos")-.25f)>.01f||value("stackOn")<.5f)
  errors.add("Init Patch failed to restore defaults");
 if(saved.getSize()>0)processor.setStateInformation(saved.getData(),(int)saved.getSize());
 header.refreshPresetLabel();layoutModules();showPresetMenu(false);
 return errors;
}
void SkellAdlibLabEditor::Canvas::paint(juce::Graphics&g){
 g.fillAll(palette.background);
 auto r=getLocalBounds().toFloat();
 g.setGradientFill(juce::ColourGradient(palette.raised.withAlpha(.45f),r.getCentreX(),32.f,palette.background,r.getCentreX(),r.getBottom(),true));
 g.fillRect(r);
 g.setColour(palette.border);g.drawHorizontalLine(554,28.f,1250.f);
 g.setColour(palette.text);g.setFont(Theme::display(15));
 g.drawText("PERFORM / RECORD",286,558,245,27,juce::Justification::left);
 g.setColour(palette.muted);g.setFont(Theme::mono(11));
 g.drawText("CLICK PLAY  /  SHIFT+CLICK RECORD  /  DROP AUDIO",493,558,501,27,juce::Justification::right);
 g.setColour(palette.surface);g.fillRoundedRectangle(juce::Rectangle<float>(1012,569,233,207),10);
 g.setColour(palette.border);g.drawRoundedRectangle(juce::Rectangle<float>(1012,569,233,207),10,1);
 g.setColour(palette.text);g.setFont(Theme::display(15));g.drawText("OUTPUT",1030,577,188,23,juce::Justification::left);
 g.setColour(palette.muted);g.setFont(Theme::mono(10));
 g.drawText("WIDTH",1030,695,89,13,juce::Justification::centred);
 g.drawText("GAIN",1135,695,90,13,juce::Justification::centred);
 g.drawText("LEVEL",1033,746,57,14,juce::Justification::left);
 g.setColour(palette.raised);g.fillRoundedRectangle(juce::Rectangle<float>(1089,749,132,8),3);
 g.setColour(palette.melt);g.fillRoundedRectangle(juce::Rectangle<float>(1090,750,130*juce::jlimit(0.f,1.f,processor.getOutputLevel()),6),2);
 g.setColour(palette.muted);g.setFont(Theme::mono(9));g.drawText(juce::String("SKELL AUDIO    /    ADLIBLAB v")+juce::String(JucePlugin_VersionString),28,785,290,12,juce::Justification::left);
 g.drawText("DRAG TO REORDER  /  CLICK NAME TO REPLACE",885,785,360,12,juce::Justification::right);
}
void SkellAdlibLabEditor::Canvas::layoutModules(){
 const auto order=processor.getModuleOrder(), types=processor.getRackTypes();
 constexpr int positions[]={30,342,654,966};
 for(auto*panel:modules)panel->setVisible(false);
 for(auto&panel:extraPanels)panel->setVisible(false);
 for(auto&empty:emptySlots)empty->setVisible(false);
 for(int i=0;i<4;++i){
  const int slot=order[(size_t)i];
  if(!juce::isPositiveAndBelow(slot,4))continue;
  const auto type=types[(size_t)slot];
  juce::Component* panel=nullptr;
  switch(type){
   case adlib::RackType::Stack:panel=&stack;break;
   case adlib::RackType::Character:panel=&character;break;
   case adlib::RackType::Throw:panel=&throwPanel;break;
   case adlib::RackType::LegacyMelt:panel=&melt;break;
   case adlib::RackType::Empty:panel=emptySlots[(size_t)slot].get();break;
   default:extraPanels[(size_t)slot]->setType(type);
    panel=extraPanels[(size_t)slot].get();break;
  }
  if(panel){panel->setBounds(positions[i],90,290,455);panel->setVisible(true);}
 }
 repaint(20,82,1240,470);
}
void SkellAdlibLabEditor::Canvas::paintOverChildren(juce::Graphics&g){
 if(draggingSlot<0)return;
 const auto order=processor.getModuleOrder();constexpr int positions[]={30,342,654,966};
 int from=-1;for(int i=0;i<4;++i)if(order[(size_t)i]==draggingSlot)from=i;
 if(from<0)return;
 const int dest=juce::jlimit(0,3,from+juce::roundToInt((float)draggingDelta/312.f));
 const float ghostX=(float)positions[from]+(float)draggingDelta;
 g.setColour(palette.stack.withAlpha(.10f));g.fillRoundedRectangle(ghostX,91.f,289.f,453.f,11.f);
 g.setColour(palette.stack.withAlpha(.8f));g.drawRoundedRectangle(ghostX,91.f,289.f,453.f,11.f,2.f);
 const float marker=(float)positions[dest]-(draggingDelta>=0?11.f:9.f);
 g.setColour(palette.stack);g.fillRoundedRectangle(marker,101.f,5.f,425.f,2.f);
}
void SkellAdlibLabEditor::Canvas::reorder(int slot,int displacement){
 auto order=processor.getModuleOrder();int from=0;for(int i=0;i<4;++i)if(order[(size_t)i]==slot)from=i;
 // Even a short, intentional grip drag moves to the adjacent slot;
 // longer drags can skip multiple modules.
 int direction=displacement>0?1:-1;
 int distance=std::abs(displacement);
 int steps=juce::jmax(1,(distance+156)/312);
 int to=juce::jlimit(0,3,from+direction*steps);if(from==to)return;
 auto moved=order[(size_t)from];
 if(from<to)for(int i=from;i<to;++i)order[(size_t)i]=order[(size_t)(i+1)];
 else for(int i=from;i>to;--i)order[(size_t)i]=order[(size_t)(i-1)];
 order[(size_t)to]=moved;processor.setModuleOrder(order);layoutModules();
}
void SkellAdlibLabEditor::Canvas::refreshMeters(bool clipsChanged){
 for(auto*module:modules)if(module->isVisible())module->updateBypassVisual();
 for(auto&panel:extraPanels)if(panel->isVisible())panel->updateBypassVisual();
 header.repaint(1059,8,110,42);keyboard.repaint();throwPanel.repaint(20,109,249,120);repaint(1020,743,221,21);
 const int currentNote=processor.getNote();
 if(clipsChanged||currentNote!=lastDisplayedNote){lastDisplayedNote=currentNote;bank.repaint();}
}
SkellAdlibLabEditor::SkellAdlibLabEditor(SkellAdlibLabProcessor&p)
 :AudioProcessorEditor(&p),canvas(p,*this),processor(p){
 setResizable(true,true);
 getConstrainer()->setFixedAspectRatio((double)Theme::designWidth/Theme::designHeight);
 setResizeLimits(800,500,1920,1200);
 applyLookAndFeelPalette(canvas.getPalette());
 canvas.setLookAndFeel(&lookAndFeel);addAndMakeVisible(canvas);canvas.applyTheme(canvas.getThemeIndex());
 auto currentWidth=(int)processor.parameters.state.getProperty("editorWidth",1024);
 setSize(juce::jlimit(800,1920,currentWidth),(int)std::round(juce::jlimit(800,1920,currentWidth)*800.0/1280.0));
 startTimerHz(30);
 guidedTour=std::make_unique<skellcore::ui::GuidedTourController>(
     *this,"skell-adliblab-vst",1);
 showGuidedTour(true);
}
SkellAdlibLabEditor::~SkellAdlibLabEditor(){
 stopTimer();guidedTour.reset();canvas.setLookAndFeel(nullptr);
}
void SkellAdlibLabEditor::showGuidedTour(bool firstRun){
 if(!guidedTour)return;
 guidedTour->setPalette(canvas.tourPalette());
 guidedTour->setSteps(canvas.tourSteps());
 if(firstRun)guidedTour->startIfFirstRun();
 else guidedTour->replay();
}
bool SkellAdlibLabEditor::showGuidedTourForScreenshot(){
 showGuidedTour(false);
 return guidedTour && guidedTour->isShowing();
}
void SkellAdlibLabEditor::dismissGuidedTourForScreenshot(){
 if(guidedTour)guidedTour->dismiss();
}
void SkellAdlibLabEditor::applyLookAndFeelPalette(const Theme::Palette&p){lookAndFeel.applyPalette(p);canvas.repaint();}
void SkellAdlibLabEditor::setPresetMenuForScreenshot(bool visible){canvas.showPresetMenu(visible);}
void SkellAdlibLabEditor::setEffectPickerForScreenshot(bool visible){canvas.showEffectPickerForScreenshot(visible);}
juce::StringArray SkellAdlibLabEditor::runUIInteractionTests(){return canvas.runControlChecks();}
void SkellAdlibLabEditor::timerCallback(){
 const bool clipChanged=processor.collectRecordedClip();
 if(canvas.getThemeIndex()!=processor.getThemeIndex())canvas.applyTheme(processor.getThemeIndex());
 canvas.refreshMeters(clipChanged);
}
void SkellAdlibLabEditor::paint(juce::Graphics&g){g.fillAll(canvas.getPalette().background);}
void SkellAdlibLabEditor::resized(){
 float s=juce::jmin(getWidth()/1280.f,getHeight()/800.f);
 canvas.setTransform(juce::AffineTransform{});
 canvas.setBounds(0,0,1280,800);
 canvas.setTransform(juce::AffineTransform::scale(s));
 processor.parameters.state.setProperty("editorWidth",getWidth(),nullptr);
}
