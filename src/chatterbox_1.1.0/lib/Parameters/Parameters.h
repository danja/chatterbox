#pragma once

#include <Pot.h>
#include <Switch.h>

class Parameters
{

public:
    Parameters();
    void pots(Pot pots[]);
    void switches(Switch switches[]);

    // Pots - setters
        void pitch(float pitch);
    void larynx(float larynx);
    void f1f(float f1f);
    void f2f(float f2f);
    void f3f(float f3f);
    void f3q(float f3q);

    void growl(float growl);

    // Pots - getters
    float pitch();
    float larynx();
    float f1f();
    float f2f();
    float f3f();
    float f3q();

    float growl();

    // Switches - setters

    void sf1(bool sf1);
    void sf2(bool sf2);
    void sf3(bool sf3);
    void voiced(bool voiced);
    void aspirated(bool aspirated);
    void nasal(bool nasal);
    void destressed(bool destressed);
    void stressed(bool stressed);
    void hold(bool hold);
    void creak(bool creak);
    void sing(bool sing);
    void shout(bool shout);

        // Switches - getters

    bool sf1();
    bool sf2();
    bool sf3();
    bool voiced();
    bool aspirated();
    bool nasal();
    bool destressed();
    bool stressed();
    bool hold();
    bool creak();
    bool sing();
    bool shout();

private:
    Pot pots_[N_POTS_VIRTUAL];
    Switch switches_[];

    float pitch_;
    float larynx_;
    float f1f_;
    float f2f_;
    float f3f_;
    float f3q_;

    float growl_;

     bool sf1_;
    bool sf2_;
    bool sf3_;
    bool voiced_;
    bool aspirated_;
    bool nasal_;
    bool destressed_;
    bool stressed_;
    bool hold_;
    bool creak_;
    bool sing_;
    bool shout_;
};