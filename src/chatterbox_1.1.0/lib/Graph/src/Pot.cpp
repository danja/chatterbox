#include <WString.h>
#include <Pot.h>

Pot::Pot() {}

Pot::Pot(String id, int channel)
{
  Pot::id(id);
  channel_ = channel;
}

int Pot::channel()
{
  return channel_;
}

void Pot::raw(int raw)
{
  rawFloat_ = (float)raw;
}

void Pot::previous(int previous){
  previous_ = previous;
}

int Pot::previous(){
  return previous_;
}

float Pot::value()
{
  return (offset_ + scale_ * rawFloat_);
}

//   inputOffset[POT_P0] = F1F_LOW;
//  inputScale[POT_P0] = (float)(F1F_HIGH - F1F_LOW) / (float)ADC_TOP;
void Pot::range(int inputRange, float min, float max){
  offset_ = min;
  scale_ = (max-min)/(float)inputRange;
}

/*
void Pot::offset(float offset)
{
  offset_ = offset;
}

void Pot::scale(float scale)
{
  scale_ = scale;
}
*/
