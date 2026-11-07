#include "PluginEditor.h"
SkellAdlibLabEditor::Canvas::Canvas(SkellAdlibLabProcessor&p,SkellAdlibLabEditor&ed)
 :processor(p),editor(ed),header(p),stack(p),character(p),throwPanel(p),melt(p),chaos(p),bank(p),keyboard(p),
 widthAtt(p.parameters,"width",width),gainAtt(p.parameters,"outputGain",gain),
 limiterAtt(p.parameters,"safetyLimiter",limiter),formantAtt(p.parameters,"kbFormantLock",formant),
 velIntensityAtt(p.parameters,"kbVelIntensity",velIntensity),velTailAtt(p.parameters,"kbVelTail",velTail),reverseAtt(p.parameters,"kbAltReverse",reverse),
 modules{{&stack,&character,&throwPanel,&melt}} {
 setSize(Theme::designWidth,Theme::designHeight);
 addAndMakeVisible(header);header.setBounds(15,8,1250,65);
 for(auto* module:modules){addAndMakeVisible(module);module->requestReorder=[this](int id,int displacement){reorder(id,displacement);};}
 layoutModules();
 addAndMakeVisible(chaos);chaos.setBounds(26,570,236,218);
 addAndMakeVisible(bank);bank.setBounds(286,588,708,70);
 addAndMakeVisible(keyboard);keyboard.setBounds(286,665,708,81);
 for(auto*s:{&width,&gain}){s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,17);s->setColour(juce::Slider::rotarySliderFillColourId,Theme::melt);s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);addAndMakeVisible(s);}
 width.setBounds(1020,595,96,95);gain.setBounds(1137,595,96,95);
 limiter.setButtonText("SOFT CLIP");formant.setButtonText("FORMANT LOCK");velIntensity.setButtonText("VEL → INT");velTail.setButtonText("VEL → TAIL");reverse.setButtonText("ALT REVERSE");
 for(auto*toggle:{&limiter,&formant,&velIntensity,&velTail,&reverse}){toggle->setColour(juce::ToggleButton::textColourId,Theme::text);addAndMakeVisible(toggle);}
 limiter.setBounds(1060,700,155,24);formant.setBounds(286,750,150,25);velIntensity.setBounds(438,750,125,25);velTail.setBounds(567,750,125,25);reverse.setBounds(703,750,155,25);
}
void SkellAdlibLabEditor::Canvas::paint(juce::Graphics&g){
 g.fillAll(Theme::background);
 auto r=getLocalBounds().toFloat();
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff211026),r.getCentreX(),30.f,Theme::background,r.getCentreX(),r.getBottom(),true));
 g.fillRect(r);
 g.setColour(Theme::muted.withAlpha(.65f));g.setFont(Theme::mono(12));
 g.drawText("PERFORM ADLIBS   •   DRAG AUDIO TO PADS   •   SHIFT+CLICK TO RECORD",286,560,720,22,juce::Justification::left);
 g.setColour(Theme::text);g.setFont(Theme::display(14));g.drawText("OUTPUT",1022,557,220,26,juce::Justification::centred);
 g.setColour(Theme::muted);g.setFont(Theme::mono(10));g.drawText("WIDTH",1021,684,93,13,juce::Justification::centred);g.drawText("GAIN",1137,684,93,13,juce::Justification::centred);
 g.setColour(Theme::muted);g.drawText("OUT",1030,735,47,14,juce::Justification::left);
 g.setColour(juce::Colour(0xff312e3b));g.fillRoundedRectangle(juce::Rectangle<float>(1077,738,155,7),2);
 g.setColour(Theme::melt);g.fillRoundedRectangle(juce::Rectangle<float>(1078,739,juce::jlimit(0.f,153.f,processor.getOutputLevel()*153.f),5),2);
 g.setColour(Theme::muted.withAlpha(.7f));g.drawText("SKELL ADLIBLAB   /   v0.2",26,783,260,14,juce::Justification::left);
}
void SkellAdlibLabEditor::Canvas::layoutModules(){
 auto order=processor.getModuleOrder();constexpr int positions[]={30,342,654,966};
 for(int i=0;i<4;++i){int id=order[(size_t)i];if(juce::isPositiveAndBelow(id,4))modules[(size_t)id]->setBounds(positions[i],90,290,455);}
}
void SkellAdlibLabEditor::Canvas::reorder(int id,int displacement){
 auto order=processor.getModuleOrder();
 int from=0;for(int i=0;i<4;++i)if(order[(size_t)i]==id)from=i;
 int to=juce::jlimit(0,3,from+(int)std::round(displacement/312.f));
 if(from==to)return;
 auto moved=order[(size_t)from];
 if(from<to)for(int i=from;i<to;++i)order[(size_t)i]=order[(size_t)(i+1)];
 else for(int i=from;i>to;--i)order[(size_t)i]=order[(size_t)(i-1)];
 order[(size_t)to]=moved;processor.setModuleOrder(order);layoutModules();
}
void SkellAdlibLabEditor::Canvas::refreshMeters(){header.repaint();keyboard.repaint();repaint(1020,730,230,30);bank.repaint();}
SkellAdlibLabEditor::SkellAdlibLabEditor(SkellAdlibLabProcessor&p):AudioProcessorEditor(&p),canvas(p,*this),processor(p){
 setResizable(true,true);getConstrainer()->setFixedAspectRatio((double)Theme::designWidth/Theme::designHeight);
 setResizeLimits(800,500,1920,1200);
 canvas.setLookAndFeel(&lookAndFeel);addAndMakeVisible(canvas);
 auto currentWidth=(int)processor.parameters.state.getProperty("editorWidth",1024);
 setSize(juce::jlimit(800,1920,currentWidth),(int)std::round(juce::jlimit(800,1920,currentWidth)*800.0/1280.0));
 startTimerHz(30);
}
SkellAdlibLabEditor::~SkellAdlibLabEditor(){stopTimer();canvas.setLookAndFeel(nullptr);}
void SkellAdlibLabEditor::timerCallback(){processor.collectRecordedClip();canvas.refreshMeters();}
void SkellAdlibLabEditor::paint(juce::Graphics&g){g.fillAll(Theme::background);}
void SkellAdlibLabEditor::resized(){
 float s=juce::jmin(getWidth()/1280.f,getHeight()/800.f);
 canvas.setTransform(juce::AffineTransform::identity);
 canvas.setBounds(0,0,1280,800);
 canvas.setTransform(juce::AffineTransform::scale(s));
 processor.parameters.state.setProperty("editorWidth",getWidth(),nullptr);
}
