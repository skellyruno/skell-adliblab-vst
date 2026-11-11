#pragma once
#include "Theme.h"
#include "../PluginProcessor.h"
#include "VoiceMap.h"
#include <functional>
class DragGrip final:public juce::Component {
public:
 std::function<void(int)>onDrop;
 explicit DragGrip(const Theme::Palette&pal):palette(pal){}
 void paint(juce::Graphics&g)override{
  g.setColour(palette.muted);
  for(int y=0;y<3;++y)for(int x=0;x<2;++x)g.fillEllipse((float)x*6+6,(float)y*6+5,2.5f,2.5f);
 }
 void mouseDown(const juce::MouseEvent&e)override{start=e.getScreenX();}
 void mouseUp(const juce::MouseEvent&e)override{int delta=e.getScreenX()-start;if(std::abs(delta)>45&&onDrop)onDrop(delta);}
private:int start=0;const Theme::Palette&palette;
};
// Retains ToggleButton/APVTS behavior, but renders a clean power symbol.
class ModulePowerButton final:public juce::ToggleButton {
public:
 ModulePowerButton(const Theme::Palette& pal,int id):palette(pal),moduleId(id){setTooltip("Turn effect on or off");}
 void paintButton(juce::Graphics&g,bool hover,bool down)override{
  auto r=getLocalBounds().toFloat().reduced(1.f);
  auto accent=palette.accent(moduleId);
  g.setColour(palette.raised.brighter(hover?.12f:0.f));g.fillRoundedRectangle(r,7.f);
  g.setColour(hover?accent:palette.border);g.drawRoundedRectangle(r,7.f,1.f);
  auto m=r.getCentre();g.setColour(getToggleState()?accent:palette.muted);
  juce::Path arc;
  for(int i=0;i<=32;++i){
   float a=-2.4f+i*4.8f/32.f,x=m.x+6.f*std::sin(a),y=m.y-6.f*std::cos(a);
   if(i==0)arc.startNewSubPath(x,y);else arc.lineTo(x,y);
  }
  g.strokePath(arc,juce::PathStrokeType(2.f));
  g.drawLine(m.x,m.y-8.f,m.x,m.y-1.f,2.f);
  if(down){g.setColour(accent.withAlpha(.15f));g.fillEllipse(m.x-9.f,m.y-9.f,18.f,18.f);}
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
   grip.onDrop=[this](int x){if(requestReorder)requestReorder(moduleId,x);};addAndMakeVisible(grip);grip.setBounds(249,11,29,26);
   on.setButtonText("");addAndMakeVisible(on);on.setBounds(209,11,29,29);
   on.onClick=[this]{updateBypassVisual();};
   toggles.push_back(std::make_unique<ButtonAttachment>(p.parameters,enabled,on));
  }
 ~ModulePanel()override{for(auto*slider:sliders)slider->removeListener(this);}
 std::function<void(int,int)>requestReorder;
 // The whole exposed header can be dragged horizontally except its buttons.
 void mouseDown(const juce::MouseEvent&e)override{
  if(e.mods.isPopupMenu()||e.position.y>53.f)return;
  dragStart=e.getScreenX();dragging=true;repaint();
 }
 void mouseDrag(const juce::MouseEvent&e)override{
  if(dragging){dragOffset=e.getScreenX()-dragStart;repaint();}
 }
 void mouseUp(const juce::MouseEvent&e)override{
  if(!dragging)return;
  int delta=e.getScreenX()-dragStart;dragging=false;dragOffset=0;repaint();
  if(std::abs(delta)>=36&&requestReorder)requestReorder(moduleId,delta);
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
   if(child!=&on&&child!=&grip)child->setAlpha(enabled?1.f:.58f);
  }
  repaint();
 }
 void refreshTheme(){
  updateBypassVisual();
  on.setColour(juce::ToggleButton::tickColourId,accent());
  on.setColour(juce::ToggleButton::textColourId,palette.text);
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
  g.setColour(palette.border.withAlpha(.85f));g.drawRoundedRectangle(r,11.f,1.f);
  g.setGradientFill(juce::ColourGradient(accent().withAlpha(.15f),r.getTopLeft(),
                                        accent().withAlpha(.055f),juce::Point<float>(r.getRight(),53.f),false));
  g.fillRoundedRectangle(r.withHeight(52.f),10.f);
  if(dragging){g.setColour(accent().withAlpha(.20f));g.fillRoundedRectangle(r.withHeight(52.f),10.f);}
  g.setColour(palette.border.withAlpha(.62f));g.drawHorizontalLine(53,4.f,286.f);
  g.setColour(accent());g.fillRoundedRectangle(2.f,4.f,3.f,46.f,1.3f);
  g.fillRoundedRectangle(18.f,52.f,35.f,2.f,1.f);
  g.setColour(palette.text);g.setFont(Theme::display(17));
  g.drawText(title,19,11,163,28,juce::Justification::left);
  if(on.getToggleState()){
   g.setColour(accent().withAlpha(.83f));g.setFont(Theme::mono(10));
   g.drawText(juce::String(moduleId+1).paddedLeft('0',2),186,18,18,16,juce::Justification::centred);
  }else{
   g.setColour(palette.background.withAlpha(.14f));g.fillRoundedRectangle(1.f,54.f,288.f,399.f,9.f);
   g.setColour(palette.muted);g.setFont(Theme::mono(10));
   g.drawText("OFF",157,17,43,18,juce::Justification::right);
  }
  g.setColour(palette.muted);g.setFont(Theme::mono(9));
  g.drawText(dragging?"MOVING":"DRAG",237,38,45,12,juce::Justification::centred);
 }

protected:
 static juce::String formatReadout(const juce::String&param,double v){
  const auto whole=[](double n){return juce::String(juce::roundToInt(n));};
  if(param=="throwFilterStart"||param=="throwFilterEnd")
   return v>=1000.?whole(v/1000.)+"k":whole(v)+"Hz";
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
 DragGrip grip;ModulePowerButton on;int lastEnabled=-1;
 bool dragging=false;int dragStart=0,dragOffset=0;
 juce::OwnedArray<juce::Slider>sliders;juce::OwnedArray<juce::Label>labels,readouts;
 std::vector<juce::String> readoutParams;
 juce::OwnedArray<juce::ToggleButton>buttons;juce::OwnedArray<juce::ComboBox>combos;
 std::vector<std::unique_ptr<SliderAttachment>>slidersAtt;
 std::vector<std::unique_ptr<ButtonAttachment>>toggles;
 std::vector<std::unique_ptr<ComboAttachment>>combosAtt;
};
