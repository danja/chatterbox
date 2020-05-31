#include <math.h>

//  SvfLinearTrapOptimised2.hpp
//
//  Created by Fred Anton Corvest (FAC) on 26/11/2016.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#ifndef SvfLinearTrapOptimised2_hpp
#define SvfLinearTrapOptimised2_hpp

/*!
 @class SvfLinearTrapOptimised2
 @brief A ready to use C++ port of Andy Simper's implementation of State Variable Filters described in the technical paper 
 provided at http://www.cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf Thank you Andy for the time you spent on making those technical papers.
 */
class SvfLinearTrapOptimised2 {
public:
    /*!
     @class SvfLinearTrapOptimised2
     @enum	FILTER_TYPE
     @brief The different states of the filter.
     */
    enum FLT_TYPE {LOW_PASS_FILTER, BAND_PASS_FILTER, HIGH_PASS_FILTER, NOTCH_FILTER, PEAK_FILTER, ALL_PASS_FILTER, BELL_FILTER, LOW_SHELF_FILTER, HIGH_SHELF_FILTER, NO_FLT_TYPE};

    // SvfLinearTrapOptimised2::SvfLinearTrapOptimised2() {}

    
    SvfLinearTrapOptimised2() {
        _ic1eq = _ic2eq = _v1 = _v2 = _v3 = 0;
    }
    
    /*!
     @class SvfLinearTrapOptimised2
     @param gainDb
        Gain in dB to boost or cut the cutoff point of the Low shelf filter
     */
    void setGain(float gainDb) {
        _coef.setGain(gainDb);
    }
    
    /*!
     @class FacAbstractFilter
     @brief Updates the coefficients of the filter for the given cutoff, q, type and sample rate
     @param cutoff
        cutoff frequency in Hz that should be clamped into the range [16hz, NYQUIST].
     @param Q
        Q factor that should be clamped into the range [0.025f, 40.f]. Default value is 0.5
     @param sampleRate
        Sample rate. Default value is 44100hz. Do not forget to call resetState before changing the sample rate
     */
    inline void updateCoefficients(float cutoff, float q = 0.5, FLT_TYPE type = LOW_PASS_FILTER, float sampleRate = 44100) {
        _coef.update(cutoff, q, type, sampleRate);
    }
    
    /*!
     @class FacAbstractFilter
     @brief Resets the state of the filter
            Do not forget to call resetState before changing the sample rate
     */
    void resetState() {
        _ic1eq = _ic2eq = 0;
    }
    
    /*!
     @class FacAbstractFilter
     @brief Tick method. Apply the filter on the provided material
     */
    inline float tick(float v0) {
        _v3 = v0 - _ic2eq;
        _v1 = _coef._a1*_ic1eq + _coef._a2*_v3;
        _v2 = _ic2eq + _coef._a2*_ic1eq + _coef._a3*_v3;
        _ic1eq = 2*_v1 - _ic1eq;
        _ic2eq = 2*_v2 - _ic2eq;
        
        return _coef._m0*v0 + _coef._m1*_v1 + _coef._m2*_v2;
    }
    
private:
    struct Coefficients {
        Coefficients() {
            _a1 = _a2 = _a3 = _m0 = _m1 = _m2 = 0;
            _A = _ASqrt = 1;
        }
        
        void update(float cutoff, float q = 0.5, SvfLinearTrapOptimised2::FLT_TYPE type = LOW_PASS_FILTER, float sampleRate = 44100) {
            float g = tan((cutoff / sampleRate) * M_PI);
            float k = computeK(q, type == BELL_FILTER /*USE GAIN FOR BELL FILTER ONLY*/);
            
            switch (type) {
                case LOW_PASS_FILTER:
                    computeA(g, k);
                    _m0 = 0;
                    _m1 = 0;
                    _m2 = 1;
                    break;
                case BAND_PASS_FILTER:
                    computeA(g, k);
                    _m0 = 0;
                    _m1 = 1;
                    _m2 = 0;
                    break;
                case HIGH_PASS_FILTER:
                    computeA(g, k);
                    _m0 = 1;
                    _m1 = -k;
                    _m2 = -1;
                    break;
                case NOTCH_FILTER:
                    computeA(g, k);
                    _m0 = 1;
                    _m1 = -k;
                    _m2 = 0;
                    break;
                case PEAK_FILTER:
                    computeA(g, k);
                    _m0 = 1;
                    _m1 = -k;
                    _m2 = -2;
                    break;
                case ALL_PASS_FILTER:
                    computeA(g, k);
                    _m0 = 1;
                    _m1 = -2*k;
                    _m2 = 0;
                    break;
                case BELL_FILTER:
                    computeA(g, k);
                    _m0 = 1;
                    _m1 = k*(_A*_A - 1);
                    _m2 = 0;
                    break;
                case LOW_SHELF_FILTER:
                    computeA(g /= _ASqrt, k);
                    _m0 = 1;
                    _m1 = k*(_A-1);
                    _m2 = _A*_A - 1;
                    break;
                case HIGH_SHELF_FILTER:
                    computeA(g *= _ASqrt, k);
                    _m0 = _A*_A;
                    _m1 = k*(1-_A)*_A;
                    _m2 = 1-_A*_A;
                    break;
                case NO_FLT_TYPE:
                    // nothing todo
                    break;
              //  default:
                //    assert(false);
            }
        }
        
        void setGain(float gainDb) {
            _A = pow(10.0, gainDb / 40.0);
            _ASqrt = sqrt(_A);
        }
        
        float computeK(float q, bool useGain=false) {
            return 1.f / (useGain ? (q*_A) : q);
        }
        
        void computeA(float g, float k) {
            _a1 = 1/(1 + g*(g + k));
            _a2 = g*_a1;
            _a3 = g*_a2;
        }
        
        float _a1, _a2, _a3;
        float _m0, _m1, _m2;
        
        float _A;
        float _ASqrt;
    } _coef;
    
    float _ic1eq;
    float _ic2eq;
    float _v1, _v2, _v3;
};

#endif /* SvfLinearTrapOptimised2_hpp */
