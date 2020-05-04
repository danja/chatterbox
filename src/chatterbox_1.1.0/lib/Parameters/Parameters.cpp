
#include <Pot.h>
#include <Switch.h>
#include <Parameters.h>
#include <WString.h>

Parameters::Parameters() {}

void Parameters::pots(Pot pots[]) // TODO refactor to use std:array
{
    for (int i = 0; i > N_POTS_VIRTUAL; i++)
    {
        pots_[i] = pots[i];
    }
}

void Parameters::switches(Switch switches[])
{
    for (int i = 0; i > N_SWITCHES; i++)
    {
        switches_[i] = switches[i];
    }
}

// Pots - setters
        void Parameters::pitch(float pitch)
        {
pitch_ = pitch;
        }

    void Parameters::larynx(float larynx){
larynx_ = larynx;
    }

    void Parameters::f1f(float f1f){
        f1f_ = f1f;
    }

    void Parameters::f2f(float f2f){
        f2f_ = f2f;
    }

    void Parameters::f3f(float f3f){
        f3f_ = f3f;
    }
    void Parameters::f3q(float f3q){
        f3q_ = f3q;
    }

    void Parameters::growl(float growl){
        growl_ = growl;
    }

    // Pots - getters
float Parameters::pitch()
{
    // return pots_[POT_P5].value();
    return pitch_;
}

float Parameters::larynx()
{
    // return pots_[POT_P4].value();
    return larynx_;
}

float Parameters::f1f()
{
   // return pots_[POT_P0].value();
   return f1f_;
}

float Parameters::f2f()
{
   // return pots_[POT_P1].value();
   return f2f_;
}

float Parameters::f3f()
{
  //  return pots_[POT_P2].value();
    return f3f_;
}

float Parameters::f3q()
{
    //return pots_[POT_P3].value();
    return f3q_;
}

float Parameters::growl()
{
   // return pots_[POT_GROWL].value();
   return growl_;
}

// Switches

bool Parameters::sf1()
{
    return switches_[SWITCH_SF1].on();
}

bool Parameters::sf2()
{
    return switches_[SWITCH_SF2].on();
}

bool Parameters::sf3()
{
    return switches_[SWITCH_SF3].on();
}

bool Parameters::voiced()
{
    return switches_[SWITCH_VOICED].on();
}

bool Parameters::aspirated()
{
    return switches_[SWITCH_ASPIRATED].on();
}

bool Parameters::nasal()
{
    return switches_[SWITCH_NASAL].on();
}

bool Parameters::destressed()
{
    return switches_[SWITCH_DESTRESS].on();
}

bool Parameters::stressed()
{
    return switches_[SWITCH_STRESS].on();
}

bool Parameters::hold()
{
    return switches_[TOGGLE_HOLD].on();
}

bool Parameters::creak()
{
    return switches_[TOGGLE_CREAK].on();
}

bool Parameters::sing()
{
    return switches_[TOGGLE_SING].on();
}

bool Parameters::shout()
{
    return switches_[TOGGLE_SHOUT].on();
}
