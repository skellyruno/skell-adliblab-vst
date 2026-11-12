// Real JUCE processor/editor; SkellCore does the image capture.
#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"
#include <SkellCore/HeadlessUIRenderer.h>
#include <array>
#include <cstdio>
#include <functional>
namespace {
struct Size {const char* name;int width,height;};
constexpr std::array<Size,5> sizes {{{"minimum",800,500},{"compact",1024,640},
 {"design",1280,800},{"large",1600,1000},{"maximum",1920,1200}}};
constexpr std::array<const char*,4> themes {{"cosmic","neon","ivory","crimson"}};
constexpr std::array<const char*,7> states {{"main","bypass","selected","preset","picker","empty","drag"}};
void param(SkellAdlibLabProcessor& p,const char* name,float value){
 if(auto* r=dynamic_cast<juce::RangedAudioParameter*>(p.parameters.getParameter(name)))
  r->setValueNotifyingHost(r->convertTo0to1(value));
}
juce::StringArray inspect(const juce::Component& editor){
 juce::StringArray issues;
 std::function<void(const juce::Component&,int)> visit=[&](const juce::Component& parent,int depth){
  if(depth>6)return;
  const auto b=parent.getLocalBounds();
  for(int i=0;i<parent.getNumChildComponents();++i){
   const auto* c=parent.getChildComponent(i);
   if(!c->isVisible())continue;
   const auto r=c->getBounds();
   if(r.getWidth()<=0||r.getHeight()<=0||r.getX()<-1||r.getY()<-1
      ||r.getRight()>b.getWidth()+1||r.getBottom()>b.getHeight()+1)
     issues.add("JUCE child outside parent: "+c->getName()+
       " at "+r.toString()+" parent "+b.toString());
   visit(*c,depth+1);
  }
 };
 // Canvas is transformed to fit the actual editor; inspect its native
 // 1280x800 design-space children, not its transformed parent bounds.
 for(int i=0;i<editor.getNumChildComponents();++i){
  auto* child=editor.getChildComponent(i);
  if(child->getWidth()==1280&&child->getHeight()==800)visit(*child,0);
 }
 return issues;
}
}
int main(int argc,char** argv){
 const juce::String name=argc>=3&&juce::String(argv[1])=="--output"
   ?juce::String::fromUTF8(argv[2]):juce::String("ui-screenshots");
 const auto out=juce::File::getCurrentWorkingDirectory().getChildFile(name);
 juce::ScopedJuceInitialiser_GUI gui;
 SkellAdlibLabProcessor processor;
 processor.prepareToPlay(48000.,512);
 int failures=0, captured=0;
 juce::Image overview(juce::Image::RGB,4*800,4*(500+28),true);
 juce::Graphics sheet(overview);
 std::array<juce::Image,4> mainShots{};
 for(int theme=0;theme<4;++theme){
  processor.setThemeIndex(theme);
  param(processor,"stackOn",1);param(processor,"charOn",1);
  param(processor,"throwOn",1);param(processor,"meltOn",0);
  SkellAdlibLabEditor editor(processor);
  editor.dismissGuidedTourForScreenshot();
  for(const auto& issue:editor.runUIInteractionTests()){
   std::fprintf(stderr,"UI interaction: %s\n",issue.toRawUTF8());++failures;
  }
  const auto folder=out.getChildFile(themes[(size_t)theme]);
  skellcore::ui::headless::Options options;
  options.project="Skell AdlibLab actual JUCE AudioProcessorEditor";
  for(const auto& size:sizes){
   skellcore::ui::headless::Scene scene;
   scene.id=juce::String("main-")+size.name;
   scene.width=size.width;scene.height=size.height;scene.theme=theme;
   options.scenes.push_back(scene);
  }
  for(int state=1;state<7;++state)
   for(const auto& size:std::array<Size,2>{{sizes[0],sizes[2]}}){
    skellcore::ui::headless::Scene scene;
    scene.id=juce::String(states[(size_t)state])+"-"+size.name;
    scene.width=size.width;scene.height=size.height;scene.theme=theme;scene.mode=state;
    options.scenes.push_back(scene);
   }
  // Cover the other shared effects with actual editor screenshots.
  for(int effect=8;effect<=10;++effect){
   const char* type=effect==8?"motion":effect==9?"filter":"saturate";
   for(const auto& size:std::array<Size,2>{{sizes[0],sizes[2]}}){
    skellcore::ui::headless::Scene scene;
    scene.id=juce::String(type)+"-"+size.name;
    scene.width=size.width;scene.height=size.height;scene.theme=theme;scene.mode=effect;
    options.scenes.push_back(scene);
   }
  }
  options.scenes.push_back({"guided-tour-minimum",800,500,theme,7,false});
  options.scenes.push_back({"guided-tour-design",1280,800,theme,7,false});
  options.configure=[&](const auto& scene){
   editor.dismissGuidedTourForScreenshot();
   if(scene.mode==7 && !editor.showGuidedTourForScreenshot()){
    std::fprintf(stderr,"Guided tour failed to display on actual AdlibLab editor\n");
    ++failures;
   }
   // The renderer uses real processor state and real JUCE overlays.
   const auto effect=scene.mode==5?adlib::RackType::Empty:
     scene.mode==8?adlib::RackType::Motion:
     scene.mode==9?adlib::RackType::Filter:
     scene.mode==10?adlib::RackType::Saturate:adlib::RackType::Chop;
   processor.setRackType(3,effect);
   editor.refreshRackForScreenshot();
   editor.setPresetMenuForScreenshot(scene.mode==3);
   editor.setEffectPickerForScreenshot(scene.mode==4);
   editor.setDragPreviewForScreenshot(scene.mode==6);
   param(processor,"charOn",scene.mode==1?0.f:1.f);
   param(processor,"throwOn",scene.mode==1?0.f:1.f);
   param(processor,"charType",scene.mode==3?1.f:0.f);
   param(processor,"charIntensity",scene.mode==3?.82f:.35f);
   juce::MidiBuffer midi;
   midi.addEvent(scene.mode==2
      ?juce::MidiMessage::noteOn(1,60,(juce::uint8)100)
      :juce::MidiMessage::noteOff(1,60),0);
   juce::AudioBuffer<float> audio(2,64);audio.clear();processor.processBlock(audio,midi);
  };
  options.inspect=[&]{
   auto issues=inspect(editor);
   // Verify the actual SkellCore tour control bounds, not a mock layout.
   for(int i=0;i<editor.getNumChildComponents();++i){
    const auto* overlay=editor.getChildComponent(i);
    if(overlay->getComponentID()!="skellcore-guided-tour"||!overlay->isVisible())continue;
    auto* tour=dynamic_cast<const skellcore::ui::GuidedTourOverlay*>(overlay);
    if(tour==nullptr){issues.add("Guided Tour must use shared SkellCore overlay");break;}
    const auto card=tour->informationBounds();
    juce::Array<juce::Rectangle<int>> bounds;
    for(int n=0;n<overlay->getNumChildComponents();++n){
     const auto* control=dynamic_cast<const juce::TextButton*>(overlay->getChildComponent(n));
     if(control==nullptr)continue;
     const auto box=control->getBounds();
     if(box.getWidth()>63||box.getHeight()>26||box.getHeight()<19||!card.contains(box))
      issues.add("Guided Tour button is oversized or outside card: "+box.toString());
     for(const auto& previous:bounds)if(previous.intersects(box))
      issues.add("Guided Tour buttons overlap");
     bounds.add(box);
    }
    if(bounds.size()!=3)issues.add("Expected BACK, SKIP, NEXT controls");
   }
   return issues;
  };
  options.mustDiffer={{"main-minimum","bypass-minimum"},
                      {"main-design","selected-design"},
                      {"main-design","preset-design"},
                      {"main-design","picker-design"},
                      {"main-design","empty-design"},
                      {"main-design","drag-design"}};
  options.mustDiffer.push_back({"main-minimum","motion-minimum"});
  options.mustDiffer.push_back({"main-design","motion-design"});
  options.mustDiffer.push_back({"main-design","filter-design"});
  options.mustDiffer.push_back({"main-design","saturate-design"});
  options.mustDiffer.push_back({"motion-design","filter-design"});
  options.mustDiffer.push_back({"filter-design","saturate-design"});
  options.mustDiffer.push_back({"main-minimum","guided-tour-minimum"});
  options.mustDiffer.push_back({"main-design","guided-tour-design"});
  const auto report=skellcore::ui::headless::render(editor,folder,options);
  captured+=report.captured;
  editor.dismissGuidedTourForScreenshot();
  for(const auto& err:report.errors){
   std::fprintf(stderr,"%s: %s\n",themes[(size_t)theme],err.toRawUTF8());++failures;
  }
  for(int col=0;col<4;++col){
   skellcore::ui::headless::Scene state;
   state.mode=col;options.configure(state);
   const auto snap=skellcore::ui::headless::capture(editor,800,500);
   if(!snap.image.isValid()||snap.distinctColours<12){++failures;continue;}
   const int x=col*800,y=theme*528;
   sheet.setColour(juce::Colour(0xff16161e));sheet.fillRect(x,y,800,28);
   sheet.setColour(juce::Colours::white);
   sheet.setFont(juce::Font(juce::FontOptions(14.f)));
   sheet.drawText(juce::String(themes[(size_t)theme]).toUpperCase()+
     " / "+states[(size_t)col],x+12,y,776,28,juce::Justification::centredLeft);
   sheet.drawImageAt(snap.image,x,y+28);
  }
 }
 if(!skellcore::ui::headless::writePng(overview,out.getChildFile("contact-sheet.png")))++failures;
 if(captured!=100){std::fprintf(stderr,"Expected 100 actual UI PNGs, got %d\n",captured);++failures;}
 std::printf("SkellCore real editor: %d scenes, %d errors\n",captured,failures);
 processor.releaseResources();
 return failures==0?0:1;
}
