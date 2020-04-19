#include <WString.h>
#include <Switch.h>

Switch::Switch() {}

Switch::Switch(String id, int channel, int type)
{
  Switch::id(id);
  channel_ = channel;
  type_ = type;
  previous_ = false;
  hold_ = false;
  gain_ = 0.0f;
}

int Switch::channel()
{
  return channel_;
}

int Switch::type()
{
  return type_;
}

void Switch::on(bool on)
{
  on_ = on;
}

void Switch::previous(bool previous)
{ // TODO refactor
  previous_ = previous;
}

bool Switch::on()
{
  return on_;
}

bool Switch::previous()
{
  return previous_;
}

float Switch::gain() // TODO refactor
{
  return gain_;
}

void Switch::gain(float gain)
{
  if(gain > 1) gain = 1;
  if(gain < 0) gain = 0;
  gain_ = gain;
}

bool Switch::hold()
{
  return hold_;
}

void Switch::hold(bool hold) // TODO refactor/rename
{
  hold_ = hold;
}


