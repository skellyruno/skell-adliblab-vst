#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
#include "VoiceMap.h"
#include <functional>
class DragGrip final:public juce::Component {
public:
 std::function<void(int)>onDrop;
 std::function<void(int,bool)>onPreview;
 explicit DragGrip(const Theme::Palette&pal):palette(pal){}
 void paint(juce::Graphics&g)override{
  g.setColour(palette.text.withAlpha(.64f));
  for(int y=0;y<3;++y)for(int x=0;x<2;++x)g.fillEllipse((float)x*6+6,(float)y*6+5,2.5f,2.5f);
 }
 void mouseDown(const juce::MouseEvent&e)override{start=e.getScreenX();if(onPreview)onPreview(0,true);}
 void mouseDrag(const juce::MouseEvent&e)override{if(onPreview)onPreview(e.getScreenX()-start,true);}
 void mouseUp(const juce::MouseEvent&e)override{int delta=e.getScreenX()-start;
  if(onPreview)onPreview(delta,false);
  if(std::abs(delta)>45&&onDrop)onDrop(delta);}
private:int start=0;const Theme::Palette&palette;
};
// Retains ToggleButton/APVTS behavior, but renders a clean power symbol.
// Automatable on/off control; the lit ring is visibly different from OFF.
class ModulePowerButton final:public juce::ToggleButton {
public:
 ModulePowerButton(const Theme::Palette&pal,int id):palette(pal),moduleId(id){
  setTooltip("Power: enable or bypass this effect");
 }
 void paintButton(juce::Graphics&g,bool hover,bool pressed)override{
  auto r=getLocalBounds().toFloat().reduced(1.f);
  const bool enabled=getToggleState();
  const auto accent=palette.accent(moduleId);
  g.setColour(pressed?palette.background:palette.raised);
  g.fillRoundedRectangle(r,8.f);
  g.setColour(enabled?accent.withAlpha(hover?1.f:.77f):palette.muted.withAlpha(.70f));
  g.drawRoundedRectangle(r.reduced(.5f),7.5f,enabled?1.4f:1.f);
  const auto centre=r.getCentre();
  const float radius=7.f;
  juce::Path powerArc;
  constexpr int segments=36;
  for(int i=0;i<=segments;++i){
   const float radians=.76f+(juce::MathConstants<float>::twoPi-1.52f)*(float)i/(float)segments;
   const float px=centre.x+radius*std::sin(radians);
   const float py=centre.y-radius*std::cos(radians);
   if(i==0)powerArc.startNewSubPath(px,py);else powerArc.lineTo(px,py);
  }
  g.setColour(enabled?accent:palette.muted.withAlpha(.82f));
  g.strokePath(powerArc,juce::PathStrokeType(enabled?2.5f:2.1f,
                        juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  g.drawLine(centre.x,centre.y-9.f,centre.x,centre.y-1.5f,enabled?2.5f:2.1f);
  if(enabled){g.setColour(accent.withAlpha(hover?.28f:.16f));g.fillEllipse(centre.x-3.f,centre.y-2.f,6.f,6.f);}
 }
private:const Theme::Palette&palette;int moduleId;
};
class ModulePanel:public juce::Component,private juce::Slider::Listener {
public:
 using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
 using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 ModulePanel(SkellAdlibLabProcessor&p,const Theme::Palette&pal,int id,juce::String name,const char*enabled)
  :processor(p),palette(pal),moduleId(id),title(std::move(name)),grip(palette),on(palette,id){
   grip.onDrop=[this](int x){if(requestReorder)requestReorder(moduleId,x);};
   grip.onPreview=[this](int dx,bool active){if(requestPreview)requestPreview(moduleId,dx,active);};
   addAndMakeVisible(grip);grip.setBounds(249,11,29,28);
   on.setButtonText("");addAndMakeVisible(on);on.setBounds(207,10,31,31);
   on.onClick=[this]{updateBypassVisual();};
   remove.setButtonText("X");remove.setBounds(179,14,20,22);
   remove.setTooltip("Remove effect (shows + empty slot)");
   remove.onClick=[this]{if(requestRemove)requestRemove(moduleId);};
   addAndMakeVisible(remove);
   toggles.push_back(std::make_unique<ButtonAttachment>(p.parameters,enabled,on));
  }
 ~ModulePanel()override{for(auto*slider:sliders)slider->removeListener(this);}
 std::function<void(int,int)>requestReorder;
 std::function<void(int)>requestReplace,requestRemove;
 std::function<void(int,int,bool)>requestPreview;
 void setEffectTitle(const juce::String& value){if(title!=value){title=value;repaint();}}
 // The whole exposed header can be dragged horizontally except its buttons.
 void mouseDown(const juce::MouseEvent&e)override{
  if(e.mods.isPopupMenu()||e.position.y>53.f)return;
  dragStart=e.getScreenX();dragging=true;
  if(requestPreview)requestPreview(moduleId,0,true);
  repaint();
 }
 void mouseDrag(const juce::MouseEvent&e)override{
  if(dragging){dragOffset=e.getScreenX()-dragStart;
   if(requestPreview)requestPreview(moduleId,dragOffset,true);repaint();}
 }
 void mouseUp(const juce::MouseEvent&e)override{
  if(!dragging)return;
  int delta=e.getScreenX()-dragStart;dragging=false;dragOffset=0;repaint();
  if(requestPreview)requestPreview(moduleId,delta,false);
  if(std::abs(delta)>=36){if(requestReorder)requestReorder(moduleId,delta);}
  else if(requestReplace)requestReplace(moduleId);
 }
 void mouseMove(const juce::MouseEvent&e)override{
  setMouseCursor(e.position.y<=53.f?juce::MouseCursor::DraggingHandCursor:juce::MouseCursor::NormalCursor);
 }
 void mouseExit(const juce::MouseEvent&)override{if(!dragging)setMouseCursor(juce::MouseCursor::NormalCursor);}

 // Keep module bypass visible under both mouse input and host automation.
 // The processor's APVTS attachment owns the actual enable state.
 void updateBypassVisual(){
  const bool enabled=on.getToggleState();
  if(lastEnabled==(enabled?1:0))return;
  lastEnabled=enabled?1:0;
  for(int i=0;i<getNumChildComponents();++i){
   auto* child=getChildComponent(i);
   if(child!=&on&&child!=&grip&&child!=&remove)child->setAlpha(enabled?1.f:.80f);
  }
  repaint();
 }
 void refreshTheme(){
  updateBypassVisual();
  on.setColour(juce::ToggleButton::tickColourId,accent());
  on.setColour(juce::ToggleButton::textColourId,palette.text);
  remove.setColour(juce::TextButton::buttonColourId,palette.raised);
  remove.setColour(juce::TextButton::textColourOffId,palette.muted);
  for(auto*s:sliders){s->setColour(juce::Slider::rotarySliderFillColourId,accent());s->setColour(juce::Slider::textBoxTextColourId,palette.text);}
  for(auto* l:labels)l->setColour(juce::Label::textColourId,palette.muted);
  for(auto* v:readouts)v->setColour(juce::Label::textColourId,palette.text);
  for(auto* b:buttons){b->setColour(juce::ToggleButton::textColourId,palette.text);b->setColour(juce::ToggleButton::tickColourId,accent());}
  for(auto*c:combos)c->setColour(juce::ComboBox::outlineColourId,accent().withAlpha(.5f));
  repaint();
 }
 void paint(juce::Graphics&g)override{
  const auto r=getLocalBounds().toFloat().reduced(1.f);
  g.setGradientFill(juce::ColourGradient(palette.panel.brighter(.035f),r.getTopLeft(),
                                        palette.surface,r.getBottomRight(),false));
  g.fillRoundedRectangle(r,11.f);
  g.setColour(palette.background.darker(.3f));g.drawRoundedRectangle(r,11.f,2.f);
  g.setColour(palette.border.brighter(.20f).withAlpha(.98f));
  g.drawRoundedRectangle(r.reduced(1.25f),10.f,1.2f);
  g.setGradientFill(juce::ColourGradient(accent().withAlpha(.15f),r.getTopLeft(),
                                        accent().withAlpha(.055f),juce::Point<float>(r.getRight(),53.f),false));
  g.fillRoundedRectangle(r.withHeight(52.f),10.f);
  if(dragging){g.setColour(accent().withAlpha(.20f));g.fillRoundedRectangle(r.withHeight(52.f),10.f);}
  g.setColour(palette.border.withAlpha(.62f));g.drawHorizontalLine(53,4.f,286.f);
  g.setColour(accent());g.fillRoundedRectangle(2.f,4.f,3.f,46.f,1.3f);
  g.fillRoundedRectangle(18.f,52.f,35.f,2.f,1.f);
  g.setColour(palette.text);g.setFont(Theme::display(17));
  g.drawFittedText(title,19,11,157,28,juce::Justification::left,1);
  if(on.getToggleState()){
   // Effect name is a replace action; X independently removes the slot.
  }else{
   g.setColour(palette.background.withAlpha(.07f));g.fillRoundedRectangle(1.f,54.f,288.f,399.f,9.f);
   g.setColour(palette.muted);g.setFont(Theme::mono(10));
   g.drawText("OFF",148,36,44,14,juce::Justification::right);
  }
  g.setColour(palette.muted);g.setFont(Theme::mono(9));
  // Six-dot grip remains the drag affordance; extra microtext removed.
 }

protected:
 static juce::String formatReadout(const juce::String&param,double v){
  const auto whole=[](double n){return juce::String(juce::roundToInt(n));};
  if(param=="throwFilterStart"||param=="throwFilterEnd")
   return v>=999.5?whole(v/1000.)+"k":whole(v)+"Hz";
  if(param=="stackSpread")return whole(v)+"ct";
  if(param=="stackTime"||param=="charGlide")return whole(v)+"ms";
  if(param=="charCrush"||param=="throwRepeats"||param=="meltRoot")return whole(v);
  if(param=="stackFormant"||param=="charFormant"||param=="throwPitchPerRepeat")return whole(v)+"st";
  return whole(v*100.)+"%";
 }
 juce::Colour accent() const {return palette.accent(moduleId);}
 juce::Slider& knob(const char*id,juce::String name,int x,int y,int size=56){
  auto*s=sliders.add(new juce::Slider());
  s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
  s->setColour(juce::Slider::rotarySliderFillColourId,accent());
  s->setColour(juce::Slider::textBoxTextColourId,palette.text);
  s->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
  s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  const juce::String param(id);
  s->textFromValueFunction=[param](double v){return formatReadout(param,v);};
  s->setBounds(x-3,y,size+6,60);
  if(auto*r=dynamic_cast<juce::RangedAudioParameter*>(processor.parameters.getParameter(id)))
   s->setDoubleClickReturnValue(true,r->convertFrom0to1(r->getDefaultValue()));
  addAndMakeVisible(s);slidersAtt.push_back(std::make_unique<SliderAttachment>(processor.parameters,id,*s));
  // Dedicated value readout: SliderAttachment replaces its native text
  // formatter, so an independent label is required for user-facing units.
  auto* value=readouts.add(new juce::Label());
  value->setJustificationType(juce::Justification::centred);
  value->setFont(Theme::mono(11.f));
  value->setColour(juce::Label::textColourId,palette.text);
  value->setBounds(x-12,y+57,size+24,17);
  addAndMakeVisible(value);
  readoutParams.push_back(param);
  value->setText(formatReadout(param,s->getValue()),juce::dontSendNotification);
  s->addListener(this);
  auto*l=labels.add(new juce::Label());l->setText(name,juce::dontSendNotification);
  l->setJustificationType(juce::Justification::centred);l->setFont(Theme::mono(10));
  l->setColour(juce::Label::textColourId,palette.muted);l->setBounds(x-12,y+74,size+24,22);addAndMakeVisible(l);
  return *s;
 }
 juce::ToggleButton& toggle(const char*id,const juce::String&name,int x,int y){
  auto*b=buttons.add(new juce::ToggleButton(name));b->setColour(juce::ToggleButton::textColourId,palette.text);
  b->setColour(juce::ToggleButton::tickColourId,accent());b->setBounds(x,y,144,24);addAndMakeVisible(b);
  toggles.push_back(std::make_unique<ButtonAttachment>(processor.parameters,id,*b));return *b;
 }
 juce::ComboBox& combo(const char*id,const juce::StringArray&options,int x,int y,int w){
  auto*box=combos.add(new juce::ComboBox());int item=1;for(const auto&str:options)box->addItem(str,item++);
  box->setBounds(x,y,w,29);box->setColour(juce::ComboBox::outlineColourId,accent().withAlpha(.45f));addAndMakeVisible(box);
  combosAtt.push_back(std::make_unique<ComboAttachment>(processor.parameters,id,*box));return *box;
 }
 SkellAdlibLabProcessor&processor;const Theme::Palette&palette;int moduleId;juce::String title;
private:
 void sliderValueChanged(juce::Slider* changed)override{
  for(int i=0;i<sliders.size();++i)if(sliders[i]==changed){
   readouts[i]->setText(formatReadout(readoutParams[(size_t)i],changed->getValue()),juce::dontSendNotification);
   return;
  }
 }
 DragGrip grip;ModulePowerButton on;juce::TextButton remove;int lastEnabled=-1;
 bool dragging=false;int dragStart=0,dragOffset=0;
 juce::OwnedArray<juce::Slider>sliders;juce::OwnedArray<juce::Label>labels,readouts;
 std::vector<juce::String> readoutParams;
 juce::OwnedArray<juce::ToggleButton>buttons;juce::OwnedArray<juce::ComboBox>combos;
 std::vector<std::unique_ptr<SliderAttachment>>slidersAtt;
 std::vector<std::unique_ptr<ButtonAttachment>>toggles;
 std::vector<std::unique_ptr<ComboAttachment>>combosAtt;
};
