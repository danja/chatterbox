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
