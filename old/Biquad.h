/*
*  Biquad.h
*
*  Derived from Nigel Redmon's code
*  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
*
*/

#ifndef Biquad_h
#define Biquad_h

enum {
    LOWPASS = 0,
    HIGHPASS,
    BANDPASS,
    NOTCH,
    PEAK,
    LOWSHELF,
    HIGHSHELF
};

class Biquad {
public:
    Biquad();
    Biquad(int type, float Fc, float Q, float PEAKGainDB);
    ~Biquad();
    void setType(int type);
    void setQ(float Q);
    void setFc(float Fc);
    void setPEAKGain(float PEAKGainDB);
    void setBiquad(int type, float Fc, float Q, float PEAKGain);
    float process(float in);

    // ugly, I know...
    float getA0();
    float getA1();
    float getA2();
    float getB0();
    float getB1();
    float getB2();
    float getC1();
    
protected:
    void calcBiquad(void);

    int type;
    float a0, a1, a2, b0, b1, b2, c1;
    float Fc, Q, PEAKGain;
    float z1, z2;
};

/*
inline float Biquad::process(float in) {
    float out = in * a0 + z1;
    z1 = in * a1 + z2 - b1 * out;
    z2 = in * a2 - b2 * out;
    return out;
  
}
  */

#endif // Biquad_h
