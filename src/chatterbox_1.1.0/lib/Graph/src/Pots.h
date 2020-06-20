#ifndef Pots_h
#define Pots_h

#include <Pot.h>
#include <Pots.h>
// #include <WString.h>
#include <array>

#define TABLESIZE 2048 // 1024 MOVE!

#define ADC_TOP 4096

#define POT_ID_F1F "f1f"
#define POT_ID_NASAL "nasal"
#define POT_ID_F2F "f2f"
#define POT_ID_F3F "f3f"
#define POT_ID_F3Q "f3q"
#define POT_ID_LARYNX "larynx"
#define POT_ID_PITCH "pitch"
#define POT_ID_GROWL "growl"

// Variable parameter ranges
#define PITCH_MIN 20
#define PITCH_MAX 500

#define LARYNX_MIN 5 // % of wave is larynx open
#define LARYNX_MAX 95

#define F1F_LOW 150 // formant filter 1 centre frequency lowest value
#define F1F_HIGH 1400

#define NASAL_LOW 1000 // nasal filter 1 centre frequency lowest value
#define NASAL_HIGH 3000

#define F2F_LOW 500
#define F2F_HIGH 5000

#define F3F_LOW 50 // F3 is auxiliary filter, may or may not be a formant
#define F3F_HIGH 7000
#define F3Q_MIN 1.0f
#define F3Q_MAX 10.0f

#define GROWL_MIN 0.0f
#define GROWL_MAX 2.0f


class Pots
{

public:
  Pots();

  Pot &getPot(int n);
  void init();

private:
  // std::array<Pot, N_POTS_VIRTUAL> potArray;
  Pot potArray[N_POTS_VIRTUAL] ;
};

#endif