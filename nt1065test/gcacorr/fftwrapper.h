#ifndef FFTWRAPPER_H
#define FFTWRAPPER_H

#include "inc/fftw3.h"
#include "mathTypes.h"

class FFTWrapper {
public:
    FFTWrapper( unsigned int N  );
    ~FFTWrapper();
    void Transform( float* in,       float_cpx_t* out );
    void Transform( float_cpx_t* in, float_cpx_t* out, bool is_inverse );

private:
    void convert( float_cpx_t* dst, fftwf_complex* src, int len );
    float* in_float;
    fftwf_complex* in_complex;
    fftwf_complex* out_complex;

    fftwf_plan plan_real;
    fftwf_plan plan_complex[ 2 ];
    unsigned int N;
};

#endif // FFTWRAPPER_H
