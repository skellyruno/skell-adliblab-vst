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
class ModulePanel:public juce::Component {
public:
 using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
 using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 ModulePanel(SkellAdlibLabProcessor&p,const Theme::Palette&pal,int id,juce::String name,const char*enabled)
  :processor(p),palette(pal),moduleId(id),title(std::move(name)),grip(palette){
   grip.onDrop=[this](int x){if(requestReorder)requestReorder(moduleId,x);};addAndMakeVisible(grip);grip.setBounds(249,11,29,26);
   on.setButtonText("");addAndMakeVisible(on);on.setBounds(211,15,22,22);
   on.onClick=[this]{updateBypassVisual();};
   toggles.push_back(std::make_unique<ButtonAttachment>(p.parameters,enabled,on));
  }
 std::function<void(int,int)>requestReorder;
 // Keep module bypass visible under both mouse input and host automation.
 // The processor's APVTS attachment owns the actual enable state.
 void updateBypassVisual(){
  const bool enabled=on.getToggleState();
  if(lastEnabled==(enabled?1:0))return;
  lastEnabled=enabled?1:0;
  for(int i=0;i<getNumChildComponents();++i){
   auto* child=getChildComponent(i);
   if(child!=&on&&child!=&grip)child->setAlpha(enabled?1.f:.36f);
  }
  repaint();
 }
 void refreshTheme(){
  updateBypassVisual();
  on.setColour(juce::ToggleButton::tickColourId,accent());
  on.setColour(juce::ToggleButton::textColourId,palette.text);
  for(auto*s:sliders){s->setColour(juce::Slider::rotarySliderFillColourId,accent());s->setColour(juce::Slider::textBoxTextColourId,palette.text);}
  for(auto* l:labels)l->setColour(juce::Label::textColourId,palette.muted);
  for(auto* b:buttons){b->setColour(juce::ToggleButton::textColourId,palette.text);b->setColour(juce::ToggleButton::tickColourId,accent());}
  for(auto*c:combos)c->setColour(juce::ComboBox::outlineColourId,accent().withAlpha(.5f));
  repaint();
 }
 void paint(juce::Graphics&g)override{
  auto r=getLocalBounds().toFloat().reduced(1.f);
  g.setColour(palette.panel);g.fillRoundedRectangle(r,12);
  g.setColour(palette.border);g.drawRoundedRectangle(r,12,1.1f);
  g.setColour(accent().withAlpha(.11f));g.fillRoundedRectangle(r.withHeight(52),12);
  g.setColour(accent());g.fillRoundedRectangle(2.f,1.f,3.f,50.f,1.f);
  g.setColour(palette.text);g.setFont(Theme::display(17));g.drawText(title,19,12,178,28,juce::Justification::left);
  g.setColour(palette.muted);g.setFont(Theme::mono(10));g.drawText("MOVE",236,38,45,12,juce::Justification::centred);
  if(!on.getToggleState()){
   g.setColour(palette.background.withAlpha(.12f));g.fillRoundedRectangle(1.f,53.f,288.f,400.f,10.f);
   g.setColour(palette.muted);g.setFont(Theme::mono(10));g.drawText("BYPASS",120,16,80,19,juce::Justification::right);
  }
 }
protected:
 juce::Colour accent() const {return palette.accent(moduleId);}
 juce::Slider& knob(const char*id,juce::String name,int x,int y,int size=56){
  auto*s=sliders.add(new juce::Slider());
  s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,size+21,21);
  s->setColour(juce::Slider::rotarySliderFillColourId,accent());
  s->setColour(juce::Slider::textBoxTextColourId,palette.text);
  s->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
  s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  const juce::String param(id);
  s->textFromValueFunction=[param](double v){
   if(param=="throwFilterStart"||param=="throwFilterEnd")return v>=1000.0?juce::String(v/1000.,1)+"k":juce::String((int)std::round(v))+"Hz";
   if(param=="stackSpread")return juce::String((int)std::round(v))+"ct";
   if(param=="stackTime"||param=="charGlide")return juce::String(v,0)+"ms";
   if(param=="charCrush"||param=="throwRepeats"||param=="meltRoot")return juce::String((int)std::round(v));
   if(param=="stackFormant"||param=="charFormant"||param=="throwPitchPerRepeat")return juce::String(v,1)+"st";
   return juce::String(v,2);
  };
  s->setBounds(x-3,y,size+6,78);
  if(auto*r=dynamic_cast<juce::RangedAudioParameter*>(processor.parameters.getParameter(id)))
   s->setDoubleClickReturnValue(true,r->convertFrom0to1(r->getDefaultValue()));
  addAndMakeVisible(s);slidersAtt.push_back(std::make_unique<SliderAttachment>(processor.parameters,id,*s));
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
 DragGrip grip;juce::ToggleButton on;int lastEnabled=-1;
 juce::OwnedArray<juce::Slider>sliders;juce::OwnedArray<juce::Label>labels;
 juce::OwnedArray<juce::ToggleButton>buttons;juce::OwnedArray<juce::ComboBox>combos;
 std::vector<std::unique_ptr<SliderAttachment>>slidersAtt;
 std::vector<std::unique_ptr<ButtonAttachment>>toggles;
 std::vector<std::unique_ptr<ComboAttachment>>combosAtt;
};
