#pragma once
#include "Theme.h"
#include "VoiceMap.h"
#include <functional>
class DragGrip final:public juce::Component {
public:
 std::function<void(int)>onDrop;
 void paint(juce::Graphics&g)override{g.setColour(Theme::muted);for(int y=0;y<3;++y)for(int x=0;x<2;++x)g.fillEllipse((float)x*6+6,(float)y*6+5,2.4f,2.4f);}
 void mouseDown(const juce::MouseEvent&e)override{start=e.getScreenX();}
 void mouseUp(const juce::MouseEvent&e)override{int delta=e.getScreenX()-start;if(std::abs(delta)>65&&onDrop)onDrop(delta);}
private:int start=0;
};
class ModulePanel:public juce::Component {
public:
 using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
 using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 ModulePanel(SkellAdlibLabProcessor&p,int id,juce::String name,juce::Colour accent,const char*enabled)
  :processor(p),moduleId(id),title(std::move(name)),colour(accent){
   grip.onDrop=[this](int x){if(requestReorder)requestReorder(moduleId,x);};addAndMakeVisible(grip);grip.setBounds(247,10,25,24);
   on.setButtonText("");on.setColour(juce::ToggleButton::tickColourId,colour);addAndMakeVisible(on);on.setBounds(209,13,18,18);
   toggles.push_back(std::make_unique<ButtonAttachment>(p.parameters,enabled,on));
  }
 std::function<void(int,int)>requestReorder;
 void paint(juce::Graphics&g)override{
  auto r=getLocalBounds().toFloat().reduced(1.f);
  g.setColour(Theme::panel);g.fillRoundedRectangle(r,11);
  g.setColour(colour.withAlpha(.40f));g.drawRoundedRectangle(r,11,1.1f);
  g.setColour(colour.withAlpha(.12f));g.fillRoundedRectangle(r.withHeight(51),10);
  g.setColour(Theme::text);g.setFont(Theme::display(17));g.drawText(title,19,13,190,27,juce::Justification::left);
  g.setColour(Theme::muted);g.setFont(Theme::mono(9));g.drawText("DRAG",235,37,35,12,juce::Justification::centred);
 }
protected:
 juce::Slider& knob(const char*id,juce::String name,int x,int y,int size=56){
  auto* s=sliders.add(new juce::Slider());s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,size+14,18);
  s->setColour(juce::Slider::rotarySliderFillColourId,colour);
  s->setColour(juce::Slider::textBoxTextColourId,Theme::text);
  s->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
  s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  s->setBounds(x,y,size,76);s->setDoubleClickReturnValue(true,dynamic_cast<juce::RangedAudioParameter*>(processor.parameters.getParameter(id))->convertFrom0to1(processor.parameters.getParameter(id)->getDefaultValue()));
  addAndMakeVisible(s);slidersAtt.push_back(std::make_unique<SliderAttachment>(processor.parameters,id,*s));
  auto*l=labels.add(new juce::Label());l->setText(name,juce::dontSendNotification);l->setJustificationType(juce::Justification::centred);l->setFont(Theme::mono(10));l->setColour(juce::Label::textColourId,Theme::muted);l->setBounds(x-11,y+72,size+22,21);addAndMakeVisible(l);
  return *s;
 }
 juce::ToggleButton& toggle(const char*id,const juce::String&name,int x,int y){
  auto*b=buttons.add(new juce::ToggleButton(name));b->setColour(juce::ToggleButton::textColourId,Theme::text);b->setColour(juce::ToggleButton::tickColourId,colour);b->setBounds(x,y,144,23);addAndMakeVisible(b);
  toggles.push_back(std::make_unique<ButtonAttachment>(processor.parameters,id,*b));return *b;
 }
 juce::ComboBox& combo(const char*id,const juce::StringArray&options,int x,int y,int w){
  auto*box=combos.add(new juce::ComboBox());int item=1;for(const auto&str:options)box->addItem(str,item++);
  box->setBounds(x,y,w,27);box->setColour(juce::ComboBox::outlineColourId,colour.withAlpha(.45f));addAndMakeVisible(box);
  combosAtt.push_back(std::make_unique<ComboAttachment>(processor.parameters,id,*box));return *box;
 }
 SkellAdlibLabProcessor&processor;int moduleId;juce::String title;juce::Colour colour;
private:
 DragGrip grip;juce::ToggleButton on;
 juce::OwnedArray<juce::Slider>sliders;juce::OwnedArray<juce::Label>labels;
 juce::OwnedArray<juce::ToggleButton>buttons;juce::OwnedArray<juce::ComboBox>combos;
 std::vector<std::unique_ptr<SliderAttachment>>slidersAtt;
 std::vector<std::unique_ptr<ButtonAttachment>>toggles;
 std::vector<std::unique_ptr<ComboAttachment>>combosAtt;
};
