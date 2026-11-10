#pragma once
#include "EngineTypes.h"
namespace adlib {
class PhraseTracker {
public:
 void prepare(double sampleRate){sr=sampleRate;active=false;above=below=0;syllableSent=false;}
 void tick(Events&e){
  constexpr float open=.012f,close=.006f;
  if(e.envelope>open){
   if(active&&below>0&&syllableSent)syllableSent=false;
   ++above;below=0;
   if(!active&&above>(int)(sr*.024)){active=true;syllableSent=false;}
  }else if(e.envelope<close){
   ++below;above=0;
  }
  // 50ms breath/pause closes a syllable; 135ms closes a word/phrase.
  // They are intentionally not identical events.
  if(active&&!syllableSent&&below>(int)(sr*.050)){
   e.syllableEnd=true;syllableSent=true;
  }
  if(active&&below>(int)(sr*.135)){
   e.wordEnd=true;active=false;below=0;above=0;syllableSent=false;
  }
 }
private:double sr=44100;bool active=false,syllableSent=false;int above=0,below=0;
};
}
