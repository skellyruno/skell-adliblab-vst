#include "PluginEditor.h"
SkellAdlibLabEditor::Canvas::Canvas(SkellAdlibLabProcessor&p,SkellAdlibLabEditor&ed)
 :processor(p),editor(ed),palette(Theme::makePalette(p.getThemeIndex())),themeIndex(p.getThemeIndex()),
 header(p,palette),stack(p,palette),character(p,palette),throwPanel(p,palette),melt(p,palette),
 chaos(p,palette),bank(p,palette),keyboard(p,palette),
 widthAtt(p.parameters,"width",width),gainAtt(p.parameters,"outputGain",gain),
 limiterAtt(p.parameters,"safetyLimiter",limiter),formantAtt(p.parameters,"kbFormantLock",formant),
 velIntensityAtt(p.parameters,"kbVelIntensity",velIntensity),velTailAtt(p.parameters,"kbVelTail",velTail),reverseAtt(p.parameters,"kbAltReverse",reverse),
 modules{{&stack,&character,&throwPanel,&melt}} {
 setSize(Theme::designWidth,Theme::designHeight);
 addAndMakeVisible(header);header.setBounds(15,8,1250,65);
 header.onThemeCycle=[this]{int next=(themeIndex+1)%4;processor.setThemeIndex(next);applyTheme(next);};
 header.onThemeSelect=[this](int next){processor.setThemeIndex(next);applyTheme(next);};
 for(auto* module:modules){addAndMakeVisible(module);module->requestReorder=[this](int id,int displacement){reorder(id,displacement);};}
 layoutModules();
 addAndMakeVisible(chaos);chaos.setBounds(27,567,233,214);
 addAndMakeVisible(bank);bank.setBounds(286,590,708,68);
 addAndMakeVisible(keyboard);keyboard.setBounds(286,666,708,79);
 for(auto*s:{&width,&gain}){
  s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,83,22);
  s->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  addAndMakeVisible(s);
 }
 width.textFromValueFunction=[](double d){return juce::String(d,2);};
 gain.textFromValueFunction=[](double d){return juce::String(d,1)+"dB";};
 width.setBounds(1028,601,91,91);gain.setBounds(1132,601,91,91);
 limiter.setButtonText("SOFT CLIP");formant.setButtonText("FORMANT LOCK");
 velIntensity.setButtonText("VEL > INT");velTail.setButtonText("VEL > TAIL");reverse.setButtonText("ALT REVERSE");
 for(auto*toggle:{&limiter,&formant,&velIntensity,&velTail,&reverse})addAndMakeVisible(toggle);
 limiter.setBounds(1053,713,161,25);
 formant.setBounds(286,753,147,25);velIntensity.setBounds(447,753,131,25);
 velTail.setBounds(583,753,130,25);reverse.setBounds(726,753,148,25);
 header.refreshTheme();for(auto*module:modules)module->refreshTheme();
}
void SkellAdlibLabEditor::Canvas::applyTheme(int index){
 themeIndex=juce::jlimit(0,3,index);
 palette=Theme::makePalette(themeIndex);
 header.refreshTheme();chaos.refreshTheme();
 for(auto*module:modules)module->refreshTheme();
 for(auto*s:{&width,&gain}){
  s->setColour(juce::Slider::rotarySliderFillColourId,palette.melt);
  s->setColour(juce::Slider::textBoxTextColourId,palette.text);
 }
 for(auto*t:{&limiter,&formant,&velIntensity,&velTail,&reverse}){
  t->setColour(juce::ToggleButton::textColourId,palette.text);
  t->setColour(juce::ToggleButton::tickColourId,palette.melt);
 }
 editor.applyLookAndFeelPalette(palette);
 bank.repaint();keyboard.repaint();repaint();
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
 g.setColour(palette.muted);g.setFont(Theme::mono(9));g.drawText("SKELL AUDIO    /    ADLIBLAB 0.3",28,785,290,12,juce::Justification::left);
 g.drawText("SKULL: CLICK TO CYCLE / RIGHT-CLICK TO CHOOSE",930,785,317,12,juce::Justification::right);
}
void SkellAdlibLabEditor::Canvas::layoutModules(){
 auto order=processor.getModuleOrder();constexpr int positions[]={30,342,654,966};
 for(int i=0;i<4;++i){int id=order[(size_t)i];if(juce::isPositiveAndBelow(id,4))modules[(size_t)id]->setBounds(positions[i],90,290,455);}
}
void SkellAdlibLabEditor::Canvas::reorder(int id,int displacement){
 auto order=processor.getModuleOrder();int from=0;for(int i=0;i<4;++i)if(order[(size_t)i]==id)from=i;
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
}
SkellAdlibLabEditor::~SkellAdlibLabEditor(){stopTimer();canvas.setLookAndFeel(nullptr);}
void SkellAdlibLabEditor::applyLookAndFeelPalette(const Theme::Palette&p){lookAndFeel.applyPalette(p);canvas.repaint();}
void SkellAdlibLabEditor::timerCallback(){
 const bool clipChanged=processor.collectRecordedClip();
 if(canvas.getThemeIndex()!=processor.getThemeIndex())canvas.applyTheme(processor.getThemeIndex());
 canvas.refreshMeters(clipChanged);
}
void SkellAdlibLabEditor::paint(juce::Graphics&g){g.fillAll(canvas.getPalette().background);}
void SkellAdlibLabEditor::resized(){
 float s=juce::jmin(getWidth()/1280.f,getHeight()/800.f);
 canvas.setTransform(juce::AffineTransform::identity);
 canvas.setBounds(0,0,1280,800);
 canvas.setTransform(juce::AffineTransform::scale(s));
 processor.parameters.state.setProperty("editorWidth",getWidth(),nullptr);
}
