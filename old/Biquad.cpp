/*
*  Biquad.cpp
*
*  Derived from Nigel Redmon's code
*  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
*
*/

#include <math.h>
#include "Biquad.h"

Biquad::Biquad() {
    type = LOWPASS;
    a0 = 1.0;
    a1 = a2 = b0 = b1 = b2 = c1 = 0.0;
    Fc = 0.50;
    Q = 0.707;
    PEAKGain = 0.0;
    z1 = z2 = 0.0;
}

// ugly, I know...
float Biquad::getA0(){
return a0;
}

float Biquad::getA1(){
return a1;
}

float Biquad::getA2(){
return a2;
}

float Biquad::getB0(){
return b0;
}

float Biquad::getB1(){
return b1;
}

float Biquad::getB2(){
return b2;
}

float Biquad::getC1(){
return c1;
}

Biquad::Biquad(int type, float Fc, float Q, float PEAKGainDB) {
    setBiquad(type, Fc, Q, PEAKGainDB);
    z1 = z2 = 0.0;
}

Biquad::~Biquad() {
}

void Biquad::setType(int type) {
    this->type = type;
    calcBiquad();
}

void Biquad::setQ(float Q) {
    this->Q = Q;
    calcBiquad();
}

void Biquad::setFc(float Fc) {
    this->Fc = Fc;
    calcBiquad();
}

void Biquad::setPEAKGain(float PEAKGainDB) {
    this->PEAKGain = PEAKGainDB;
    calcBiquad();
}
    
void Biquad::setBiquad(int type, float Fc, float Q, float PEAKGainDB) {
    this->type = type;
    this->Q = Q;
    this->Fc = Fc;
    setPEAKGain(PEAKGainDB);
}

void Biquad::calcBiquad(void) {
  b0 = c1 = 0;
    float norm;
    float V = pow(10, fabs(PEAKGain) / 20.0);
    float K = tan(M_PI * Fc);
    switch (this->type) {
        case LOWPASS:
            norm = 1 / (1 + K / Q + K * K);
            a0 = K * K * norm;
            a1 = 2 * a0;
            a2 = a0;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / Q + K * K) * norm;
            break;
            
        case HIGHPASS:
            norm = 1 / (1 + K / Q + K * K);
            a0 = 1 * norm;
            a1 = -2 * a0;
            a2 = a0;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / Q + K * K) * norm;
            break;
            
        case BANDPASS:
            norm = 1 / (1 + K / Q + K * K);
            a0 = K / Q * norm;
            a1 = 0;
            a2 = -a0;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / Q + K * K) * norm;
            break;
            
        case NOTCH:
            norm = 1 / (1 + K / Q + K * K);
            a0 = (1 + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = a0;
            b1 = a1;
            b2 = (1 - K / Q + K * K) * norm;
            break;
            
        case PEAK:
            if (PEAKGain >= 0) {    // boost
                norm = 1 / (1 + 1/Q * K + K * K);
                a0 = (1 + V/Q * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - V/Q * K + K * K) * norm;
                b1 = a1;
                b2 = (1 - 1/Q * K + K * K) * norm;
            }
            else {    // cut
                norm = 1 / (1 + V/Q * K + K * K);
                a0 = (1 + 1/Q * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - 1/Q * K + K * K) * norm;
                b1 = a1;
                b2 = (1 - V/Q * K + K * K) * norm;
            }
            break;
        case LOWSHELF:
            if (PEAKGain >= 0) {    // boost
                norm = 1 / (1 + sqrt(2) * K + K * K);
                a0 = (1 + sqrt(2*V) * K + V * K * K) * norm;
                a1 = 2 * (V * K * K - 1) * norm;
                a2 = (1 - sqrt(2*V) * K + V * K * K) * norm;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - sqrt(2) * K + K * K) * norm;
            }
            else {    // cut
                norm = 1 / (1 + sqrt(2*V) * K + V * K * K);
                a0 = (1 + sqrt(2) * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - sqrt(2) * K + K * K) * norm;
                b1 = 2 * (V * K * K - 1) * norm;
                b2 = (1 - sqrt(2*V) * K + V * K * K) * norm;
            }
            break;
        case HIGHSHELF:
            if (PEAKGain >= 0) {    // boost
                norm = 1 / (1 + sqrt(2) * K + K * K);
                a0 = (V + sqrt(2*V) * K + K * K) * norm;
                a1 = 2 * (K * K - V) * norm;
                a2 = (V - sqrt(2*V) * K + K * K) * norm;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - sqrt(2) * K + K * K) * norm;
            }
            else {    // cut
                norm = 1 / (V + sqrt(2*V) * K + K * K);
                a0 = (1 + sqrt(2) * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - sqrt(2) * K + K * K) * norm;
                b1 = 2 * (K * K - V) * norm;
                b2 = (V - sqrt(2*V) * K + K * K) * norm;
            }
            break;
    }
    
    return;
}

    float Biquad::process(float in) {
    float out = in * a0 + z1;
    z1 = in * a1 + z2 - b1 * out;
    z2 = in * a2 - b2 * out;
    return out;
}
