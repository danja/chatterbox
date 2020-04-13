#include <Switch.h>

Switch::Switch() {}

void Switch::setChannel(int ch)
{
  channel = ch;
}

int Switch::getChannel()
{
  return channel;
}

void Switch::setRawValue(bool val)
{
  rawValue = val;
}

void Switch::setPreviousRawValue(bool val)
{ // TODO refactor
  previousRawValue = val;
}

bool Switch::getRawValue()
{
  return rawValue;
}

bool Switch::getPreviousRawValue()
{
  return previousRawValue;
}
