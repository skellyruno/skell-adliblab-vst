#pragma once
#include "EngineTypes.h"
namespace adlib {
class PhraseTracker {
public:
 void prepare(double sampleRate) { sr=sampleRate;active=false;above=below=0; }
 void tick(Events& e) {
  constexpr float open=0.012f,close=0.006f;
  if(e.envelope>open) { ++above;below=0;if(!active && above>(int)(sr*.028)) active=true; }
  else if(e.envelope<close) {++below;above=0;}
  if(active && below>(int)(sr*.09)) { e.wordEnd=true;e.syllableEnd=true;active=false;below=0; }
 }
private:double sr=44100;bool active=false;int above=0,below=0;
};
}
