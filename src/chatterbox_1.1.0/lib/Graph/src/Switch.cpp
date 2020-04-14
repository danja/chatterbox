#include <WString.h>
#include <Switch.h>

Switch::Switch() {}

Switch::Switch(String string, int ch, int t)
{
  Switch::setID(string);
  channel = ch;
  type = t;
  value = false;
  previousValue = false;
  hold = false;
  gain = 0.0f;
}

int Switch::getChannel()
{
  return channel;
}

int Switch::getType()
{
  return type;
}

void Switch::setValue(bool val)
{
  value = val;
}

void Switch::setPreviousValue(bool val)
{ // TODO refactor
  previousValue = val;
}

bool Switch::getValue()
{
  return value;
}

bool Switch::getPreviousValue()
{
  return previousValue;
}

float Switch::getGain() // TODO refactor
{
  return gain;
}

void Switch::setGain(float g)
{
  if(g > 1) g = 1;
  if(g < 0) g = 0;
  gain = g;
}

void Switch::setHold(bool h) // TODO refactor/rename
{
  hold = h;
}

bool Switch::getHold()
{
  return hold;
}
