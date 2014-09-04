/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
/*
 * Complex binary operations, FFT and convolution algorithms.
 *
 * This is an implementation with simplistic interfaces in order
 * to serve as a placeholder for more complex fft libraries like
 * fftw.
 *
 * @deplib: This library depicts part of the algorithmic requirements
 * posed by the WFS runtime filtering algorithms.
 *
 * \author Gregor Heinrich, Arbylon / for Fraunhofer IGD
 * \date 20061812
 *
 */


#ifndef MYMATH_H
#define MYMATH_H

/********** type definitions **************/

/*!
 * real data type (float or double)
 */
typedef double real;
//typedef float real;

/*!
 * complex data type
 */
typedef struct {
    real re;
    real im;
} complex;

/********** complex operations **************/

/*!
 * complex multiplication
 *
 * \param x
 * \param y
 * \return x * y
 */
complex cadd(complex x, complex y);

/*!
 * complex subtraction
 *
 * \param x
 * \param y
 * \return x * y
 */
complex csub(complex x, complex y);

/*!
 * complex multiplication
 *
 * \param x
 * \param y
 * \return x * y
 */
complex cmult(complex x, complex y);

/*!
 * complex division
 *
 * \param x
 * \param y
 * \return x * y
 */
complex cdiv(complex x, complex y);

/*!
 * magnitude of a complex number
 *
 * \param x
 * \return |x|
 */
real cmag(complex x);

/*!
 * phase of a complex number
 *
 * \param x
 * \return arg(x)
 */
real mycarg(complex x);

/*!
 * complex number from magnitude and phase
 *
 * \param mag magnitude
 * \return mag * exp(j arg)
 */
complex mycexp(real mag, real arg);


/*!
 * conversion from real to complex
 *
 * \param in real input
 * \param out complex output
 * \param len buffer lengths
 * \return error code <0 or 0.
 */
int r2c(real* in, complex* out, int len);


/*!
 * conversion from complex to real
 *
 * \param in complex input
 * \param out real output
 * \param len buffer lengths
 * \param mode uses Re(z) if 0, Im(z) if 1, |z| if 2, arg(z) if 3
 * \return error code <0 or 0.
 */
int c2r(complex* in, real* out, int len, int mode);

/********** fft interface **************/

/*!
 * bit reversal
 *
 * \param i number with bits
 * \param m number of total bits (=ld of size of fft n=2^m).
 */
int bitrev(int i,int m);

/*!
 * calculate phase factors
 *
 * \param w [out] array of phase factors (fft size / 2)
 * \param idir direction (forward = 1, backward = -1)
 * \param size of fft
 */
void ffw(complex *w,int idir,int n);

/*!
 * permutation from data order to butterfly order,
 *
 * \param x [in/out] data
 * \param m ld of fft size
 */
void perm(complex* x, int m);

/*!
 * permutation from data order to butterfly order using reversed half-
 * frames
 *
 * \param x [in/out] data whose actual order is
 *        x[2^(m-1):2^m-1]x[0:2^(m-1)]
 * \param m ld of fft size
 */
void permrev(complex* x, int m);

/*!
 * permutation from data order to butterfly order into
 * provided array.
 *
 * \param x [in] data
 * \param y [out] data
 * \param m ld of fft size
 */
void permcpy(complex* x, complex* y, int m);


/*!
 * permutation from data order to butterfly order using reversed half-
 * frames, copies into the provided array.
 *
 * \param x [in/out] data whose actual order is
 *        x[2^(m-1):2^m-1]x[0:2^(m-1)]
 * \param m ld of fft size
 */
void permrevcpy(complex* x, complex* y, int m);

/*!
 * fft core routine.
 * (must be prepended by a permutation)
 *
 * \param [in/out] time/freq domain vector
 * \param direction (forward = 1, backward = -1)
 * \param size of fft (should be 2^N)
 */
void fft1d(complex *x,complex *w,int idir,int n);

/*!
 * complete in-place fft
 *
 * \param x [in/out] time/freq
 * \param w phase factors for transformation
 */
void fft(complex* x, complex* w, int m);

/*!
 * complete in-place fft with reversed half-frame order
 *
 * \param x [in/out] time (with reversed half-frame order)/freq
 * \param w phase factors for transformation
 */
void fftrev(complex* x, complex* w, int m);

/*!
 * complete in-place ifft
 *
 * \param x [in/out] freq/time
 * \param w phase factors for inverse transformation
 */
void ifft(complex* x, complex* w, int m);

/*!
 * complete fft preserving input
 *
 * \param x [in] time
 * \param y [out] freq
 * \param w phase factors for transformation
 */
void fftcpy(complex* x, complex* y, complex* w, int m);

/*!
 * complete ifft preserving input
 *
 * \param x [in] freq
 * \param y [out] time
 * \param w phase factors for transformation
 */
void ifftcpy(complex* x, complex* y, complex* w, int m);

/********** convolution interface **************/

/*!
 * frequency-domain convolution
 *
 * \param x [in] time-domain signal x, length 2^(m-1)
 * \param h [in] impulse response h, length 2^(m-1)
 * \param y [out] convolved time-domain signal y = x ** h, length 2^m
 * \param wf phase factors for fft
 * \param wf phase factors for inverse fft
 * \param m ld of fft size
 */
void fconv(complex* x, complex* h, complex* y, complex* wf, complex* wi, int m);

/*!
 * frequency-domain convolution with internal weights calculation
 *
 * \param x [in] time-domain signal x, length 2^(m-1) plus zero padding to length 2^m
 * \param h [in] impulse response h, length 2^(m-1) plus zero padding to length 2^m
 * \param y [out] convolved time-domain signal y = x ** h, length 2^m
 * \param m ld of fft size
 */
void fwconv(complex* x, complex* h, complex* y, int m);

/*!
 * time-domain convolution
 *
 * \param x [in] time-domain signal x
 * \param xlen length of x
 * \param h [in] impulse response h
 * \param hlen length of h
 * \param y [out] convolved time-domain signal y = x ** h,
 *        length ylen = xlen + hlen - 1
 */
void tconv(complex* x, int xlen, complex* h, int ylen, complex* y);

/********** debug output **************/

/*!
 * print complex vector of length n
 */
void printcvec(complex* x, int n);

/*!
 * print real vector of length n
 */
void printrvec(real* x, int n);

/*!
 * print 8 lowest significant bits
 */
void printbits(int n);

#endif // MYMATH_H

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
/*
 * time-partitioned OLS convolution, preparation for real-time operation.
 *
 * \author Gregor Heinrich, Arbylon / for Fraunhofer IGD
 * \date 20061912
 */
#ifndef RTOLS_H
#define RTOLS_H

/*!
 * summons information needed to filter the signal.
 */
typedef struct {
    // input buffer, complex, size L
    // TODO: use circular buffer
    complex *x;
    // output buffer, complex, size K
    complex *y;
    // length of buffers
    int buflen;
    // fft weights
    // @parallel: use in its own struct and share between filters
    complex *wf, *wi;
    // IR frame spectra, input frame spectra
    complex **HH, **XX;
    // sharing mode (bitmasks: RTOLS_SHARED_*)
    int sharemode;
    // spectral product buffer
    complex *Y;
    // frame count
    unsigned long i;
    // frame samples sizes
    int K, L, R, S, m, xlen, hlen, ylen, xframes;
    // status fields
    int hastail, hassignal, finishing;
    // for filter update
    complex **HHnew, **HHold;
    unsigned long tstart, tend;
} filter;

// some error codes
#define RTOLS_ERROR_GENERAL -1
#define RTOLS_ERROR_RESTART_FAILED -2
#define RTOLS_ERROR_SHARED_INPUT -4
#define RTOLS_ERROR_UNSHARED_INPUT -8

// share modes
#define RTOLS_SHARED_NONE 0
#define RTOLS_SHARED_FFT 1
#define RTOLS_SHARED_INPUT 2
#define RTOLS_SHARED_FILTER 4
#define RTOLS_SHARED_OUTPUT_ADD 8


/*!
 * time-partitioned OLS convolution, streaming mode.
 *
 * Prepares the filtering by allocating internal memory of a filter and setting up the transformations. The
 * input and output buffers are complex arrays inbuf and outbuf. Therefore, the streaming framework that
 * calls trigger must call the appropriate conversion methods rtols_*2*(in, out, len). TODO: fix this to avoid
 * this inner-loop conversion.
 *
 * In the system, two internal i/o buffers of type complex are generated, one of size 2^m for the input data,
 * and one of size 2^(m-1) for the output data.
 *
 * \param filter the structure that contains all information for filtering
 * \param h impulse response
 * \param hlen length of h
 * \param m ld of fft size used to partition the signals, note that input and output buffers are fftsize/2
 * \return error code <0 or 0
 */
int rtols_prepare(filter* filt, complex* h, int hlen, int m);

/*!
 * time-partitioned OLS convolution, streaming mode with shared resources for fft and input buffer. Sharing the
 * input buffer requires that the rtols_filter_sigshare method be used instead of rtols_filter, which will return
 * -1 and ignore the call.
 *
 * Prepares the filtering by allocating internal memory of a filter and setting up the transformations. The
 * input and output buffers are complex arrays inbuf and outbuf. Therefore, the streaming framework that
 * calls trigger must call the appropriate conversion methods rtols_*2*(in, out, len). TODO: fix this to avoid
 * this inner-loop conversion.
 *
 * In the system, two internal i/o buffers of type complex are generated, one of size 2^m for the input data,
 * and one of size 2^(m-1) for the output data.
 *
 * \param filter the structure that contains all information for filtering
 * \param h impulse response
 * \param hlen length of h
 * \param wf forward fft structure
 * \param wi inverse fft structure
 * \param XX circular frame spectrum buffer of size R x 2^m,
 *        R is checked if it is compatible with hlen: R = ceil(hlen * 2^(-m+1)),
 *        if not, -2 as the error code is returned.
 * \param m ld of fft size used to partition the signals, note that input and output buffers are fftsize/2
 * \param addout 0 if outgoing signals overwrite the output buffer, 1 if they are added.
 * \return error code <0 or 0
 */
int rtols_prepare_sigshare(filter* filt, complex* h, int hlen, complex* wf, complex* wi,
                           complex** XX, int R, int m, int addout);

/*!
 * time-partitioned OLS convolution, streaming mode with shared resources for fft and filter transfer function.
 *
 * Prepares the filtering by allocating internal memory of a filter and setting up the transformations. The
 * input and output buffers are complex arrays inbuf and outbuf. Therefore, the streaming framework that
 * calls trigger must call the appropriate conversion methods rtols_*2*(in, out, len). TODO: fix this to avoid
 * this inner-loop conversion.
 *
 * In the system, two internal i/o buffers of type complex are generated, one of size 2^m for the input data,
 * and one of size 2^(m-1) for the output data.
 *
 * \param filter the structure that contains all information for filtering
 * \param wf forward fft structure
 * \param wi inverse fft structure
 * \param HH filter transfer functions (frame spectra), size R x 2^m
 * \param R number of filter frames
 * \param hlen original length of filter impulse response
 * \param m ld of fft size used to partition the signals, note that input and output buffers are fftsize/2
 * \return error code <0 or 0
 */
int rtols_prepare_filtshare(filter* filt, complex* wf, complex* wi, complex** HH, int R, int hlen, int m);

/*!
 * time-partitioned OLS convolution, streaming mode.
 *
 * Triggers the algorithm that a new frame has been written into inbuf and last result will be read from outbuf.
 * Important: Do not use for sharing inputs between filters, otherwise this is ignored and -4 returned.
 *
 * \param filters the filters to be triggered.
 * \param inbuf real input buffer where the frame is read from (size 2^(m-1) = fftsize/2 expected). After calling
 *        rtols_finish and the returned number of frames, inbuf is ignored (as there is no signal left).
 * \param outbuf real output buffer where the result is written to (size 2^(m-1) expected). If not the full
 *        buffer is filled with signal, it is zero-padded.
 * \return number of samples in the output buffer that are the signal or -1 if there is no signal
 *        left or error code <-1. This is especially useful after calls to rtols_finish().
 */
int rtols_filter(filter* filt, real* inbuf, real* outbuf);

/*!
 * time-partitioned OLS convolution, streaming mode.
 *
 * Triggers the algorithm that a new frame has been written into inbuf and last result will be read from outbuf.
 *
 * \param filters the filters to be triggered.
 * \param nfilt number of filters (and output buffers)
 * \param inbuf real input buffer where the frame is read from (size 2^(m-1) = fftsize/2 expected). After calling
 *        rtols_finish and the returned number of frames, inbuf is ignored (as there is no signal left).
 * \param outbuf real output buffers where the result is written to (size 2^(m-1) expected).
 * \param outoffset offset to each output buffer (internal: pointer addition on each single buffer)
 * \return number of samples in the output buffer that are the signal or -1 if there is no signal
 *        left or error code <-1. This is especially useful after calls to rtols_finish().
 */
int rtols_filter_sigshare(filter* filters, int nfilt, real* inbuf, real** outbuf, int outoffset);

/*!
 * time-partitioned OLS convolution, streaming mode.
 *
 * Updates the filter coefficients now
 *
 * \param h new impulse response
 * \param hlen length of new IR
 * \param transition number of frames that the IR will be crossfaded from the old one
 *        (frequency-domain interpolation)
 * \return error code <0 or 0
 */
int rtols_filter_update(filter* filt, complex* h, int hlen, int transition);

/*!
 * time-partitioned OLS convolution, streaming mode.
 *
 * finishes the filter operation. Afterwards, the filter can be reused by calling rtols_restart.
 * If the filter is shared by its input, finishes all filters that belong to the signal.
 *
 * \param filter filter to be finished
 * \param len the length of the input signal that remains to be filtered
 * \return number of times that rtols_filter must be called before the signal ends (and can be
 *         restarted).
 */
int rtols_finish(filter* filt, int len);

/*!
 * after a call to rtols_finish, the filter is restarted by a new signal at its input buffer.
 *
 * \param filter the filter to be reinitialised
 * \return error code <0 or 0, -1 is returned if the signal is still finishing and the restart is ignored
 */
int rtols_restart(filter* filt);

/*!
 * deallocates the internal filter memory (inbuf and outbuf must be freed).
 *
 * \param filter to be destoyed
 * \return error code <0 or 0
 */
int rtols_cleanup(filter* filt);

#endif // RTOLS_H

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
/*
 * overlap-save convolution, time-partitioned implementation
 *
 * \author Gregor Heinrich, Arbylon / for Fraunhofer IGD
 * \date 20061812
 *
 */
#ifndef TOLS_H
#define TOLS_H

/*
 * OLS convolution
 *
 * \param x input signal (time domain)
 * \param xlen length of x
 * \param h impulse response
 * \param hlen length of h
 * \param y output (length >= xlen + hlen)
 * \param m ld of fft size (2^m should be significantly > hlen, typically hlen == 2^(m-1))
 */
void ols(complex* x, int xlen, complex* h, int hlen, complex* y, int ylen, int m);

/*!
 * time-partitioned OLS convolution
 *
 * \param x input signal (time domain)
 * \param xlen length of x
 * \param h impulse response
 * \param hlen length of h
 * \param y output (length >= xlen + hlen)
 * \param m ld of fft size used to partition the signals.
 * \return error code <0 or 0.
 */
int tols(complex* x, int xlen, complex* h, int hlen, complex* y, int ylen, int m);

/*!
void stols_finish();

/* private for tols: */

/*!
 * segment the impulse response into ceil(hlen/K) frames, where K is half the length
 * of the FFT used.
 *
 * \param h complex impulse response
 * \param hlen length of complex impulse response
 * \param m ld of fft size (ld L); the impulse response frames fill only half of this (K)
 * \param wf fft weights (ffw)
 * \param HH [out] resulting ceil(hlen/K) transfer functions where K = 2^(m-1) = 1 << (m - 1)
 */
void segment_ir(complex* h, int hlen, int m, complex* wf, complex** HH);

#endif // TOLS_H

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
 /*
 * Simple implementation of complex binary operations, FFT and
 * convolution algorithms.
 *
 * \author Gregor Heinrich, Arbylon / for Fraunhofer IGD
 * \date 20061812
 *
 * Note: FFT implementation based on code by Numerical Recipes
 * (Cambrige Univ. Press) and Martin Neumann's computational
 * physics notes (Univ. Vienna). Also, see Bronstein-Semendyayev.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define TWOPI 6.28318530717959

#define SWAP(a,b) temp = (a); (a) = (b); (b) = temp

/******************* complex stuff *******************/

complex cadd(complex x, complex y) {
    complex z;
    z.re = x.re + y.re;
    z.im = x.im + y.im;
    return z;
}

complex csub(complex x, complex y) {
    complex z;
    z.re = x.re - y.re;
    z.im = x.im - y.im;
    return z;
}

complex cmult(complex x, complex y) {
    complex z;
	real ac, bd;
	ac = x.re * y.re;
	bd = x.im * y.im;
	z.re = ac - bd;
	z.im = (x.re + x.im) * (y.re + y.im) - ac - bd;
    return z;
}

complex cdiv(complex x, complex y) {
    complex z;
    real cd2;
    cd2 = y.re * y.re + y.im * y.im;
    z.re = (x.re * y.re + x.im * y.im) / cd2;
    z.im = (x.im * y.re - x.re * y.im) / cd2;
    return z;
}

real cmag(complex x) {
    return sqrt(x.re * x.re + x.im * x.im);
}

real mycarg(complex x) {
    return atan(x.im / x.re);
}

complex mycexp(real mag, real arg) {
    complex z;
    z.re = mag * cos(arg);
    z.im = mag * sin(arg);
    return z;
}

int r2c(real* in, complex* out, int len) {
    int i;
    for (i = 0; i < len; i++) {
        out[i].re = in[i];
        out[i].im = 0.;
    }
    return 0;
}

int c2r(complex* in, real* out, int len, int mode) {
    int i;
    for (i = 0; i < len; i++) {
        if (mode == 0) {
            out[i] = (real) in[i].re;
        } else if (mode == 1) {
            out[i] = (real) in[i].im;
        } else if (mode == 2) {
            out[i] = (real) cmag(in[i]);
        } else if (mode == 3) {
            out[i] = (real) mycarg(in[i]);
        } else {
            // error: \unknown mode
            return -1;
        }
    }
    return 0;
}

/********************* fft implementation *******************/

void ffw(complex *w,int idir,int n) {
    int i;
    real arg;

    for(i=0; i<n/2; i++) {
        arg = i * idir * TWOPI / n;
        w[i].re = cos(arg);
        w[i].im = -sin(arg);
    }

}

int bitrev(int i,int m) {
    int ir,k;
    ir = 0;

    for(k=1; k<=m; k++) {
        ir = ir | ((i >> (k-1)) & 1) << (m-k);
    }
    return ir;
}

// @deplib: this should be done in hardware
void perm(complex* x, int m) {
    int n, i, ir;
    real temp;
    n = 1 << m;
    for(i=0; i<n; i++) {
        ir = bitrev(i, m);
        if(ir > i) {
            SWAP(x[ir].re, x[i].re);
            SWAP(x[ir].im, x[i].im);
        }
    }
}

/*
 * rationale: Because in the (R/T)OLS algorithm with half-frame overlap the input
 * would have to be copied twice for every signal frame, reusing an existing
 * copy of the previous input data saves 50% of input buffer copying. Nonreversed
 * and reversed permutation are being used alternatingly.
 */
// @deplib: this should be done in hardware
void permrev(complex* x, int m) {
    int n, i, ir, s;
    real temp;
    n = 1 << m;
    // TODO: integrate with bit reversal (becomes unsymmetric!)
    for (i=0; i<n/2; i++) {
        SWAP(x[i].re, x[i + n/2].re);
        SWAP(x[i].im, x[i + n/2].im);
    }
    for(i=0; i<n; i++) {
        ir = bitrev(i, m);
        if(ir > i) {
            SWAP(x[ir].re, x[i].re);
            SWAP(x[ir].im, x[i].im);
        }
    }
}

void permcpy(complex* x, complex* y, int m) {
    memcpy(y, x, (1 << m) * sizeof(complex));
    perm(y, m);
    // TODO: replace this by a direct assignment
    // from x to y
}

void permrevcpy(complex* x, complex* y, int m) {
    memcpy(y, x, (1 << m) * sizeof(complex));
    permrev(y, m);
    // TODO: replace this by a direct assignment
    // from x to y
}

void fft1d(complex *x, complex *w, int idir, int n) {
    int i, u, v, inca, incb, incn, j, k, ell;
    complex z;

    incn = n;
    inca = 1;
    // in-place butterfly (Danielson-Lanczos)
    while(incn > 1) {
        incn /= 2;
        incb = 2 * inca;
        i = 0;
        for(j=0; j<incn; j++) {
            k = 0;
            for(ell=0; ell<inca; ell++) {
                u = i + ell;
                v = u + inca;
                z.re = w[k].re * x[v].re - w[k].im * x[v].im;
                z.im = w[k].re * x[v].im + w[k].im * x[v].re;
                x[v].re = x[u].re - z.re;
                x[v].im = x[u].im - z.im;
                x[u].re = x[u].re + z.re;
                x[u].im = x[u].im + z.im;
                k += incn;
            }
            i += incb;
        }
        inca = incb;
    }

    // multiply by 1/n for inverse transform
    if(idir<0) {
        for(i=0; i<n; i++) {
            x[i].re = x[i].re / n;
            x[i].im = x[i].im / n;
        }
    }
}

void fftrev(complex* x, complex* w, int m) {
    // permute input
    permrev(x, m);

    // perform forward transform
    fft1d(x, w, 1, 1 << m);
}

void fft(complex* x, complex* w, int m) {
    // permute input
    perm(x, m);

    // perform forward transform
    fft1d(x, w, 1, 1 << m);
}

void ifft(complex* x, complex* w, int m) {
    // permute input
    perm(x, m);

    // perform inverse transform
    fft1d(x, w, -1, 1 << m);
}


void fftcpy(complex* x, complex* y, complex* w, int m) {
    // permute input
    permcpy(x, y, m);

    // perform forward transform
    fft1d(y, w, 1, 1 << m);
}

void ifftcpy(complex* x, complex* y, complex* w, int m) {
    // permute input
    permcpy(x, y, m);

    // perform inverse transform
    fft1d(y, w, -1, 1 << m);
}

/************************ convolution *************************/

void fconv(complex* x, complex* h, complex* y, complex* wf, complex* wi, int m) {
    complex *xspec, *hspec;
    int n = 1 << m, i;

    xspec = (complex*) malloc(n * sizeof(complex));
    hspec = (complex*) malloc(n * sizeof(complex));

    fftcpy(x, xspec, wf, m);
    fftcpy(h, hspec, wf, m);

    for(i = 0; i<n; i++) {
         xspec[i] = cmult(xspec[i], hspec[i]);
    }

    ifftcpy(xspec, y, wi, m);

    free(xspec);
    free(hspec);
}

void fwconv(complex* x, complex* h, complex* y, int m) {
    complex *wf, *wi;
    int n = 1 << m;

    wf = (complex*) malloc((n/2) * sizeof(complex));
    wi = (complex*) malloc((n/2) * sizeof(complex));

    // calculate phase factors for forward and inverse transform
    ffw(wf,1,n);
    ffw(wi,-1,n);

    fconv(x, h, y, wf, wi, m);

    free(wf);
    free(wi);
}

void tconv(complex* x, int xlen, complex* h, int hlen, complex* y) {
	int i, j;
	int ylen = xlen + hlen - 1;

    for (i = 0; i < ylen; i++) {
        y[i].re = 0.;
        y[i].im = 0.;
    }

	// for all signal samples
	for (i = 0; i < xlen; i++) {
		// for all impulse response samples
		for (j = 0; j < hlen; j++) {
			y[i + j] = cadd(y[i + j], cmult(x[i], h[j]));
		}
	}
}

/********************* utilities ************************/

void printcvec(complex* x, int n) {
/*    int i;
    for(i=0;i<n;i++) {
        printf(" %5d %10.5f %10.5f\n",i,x[i].re,x[i].im);
    }
    printf("\n");*/
}

void printrvec(real* x, int n) {
/*    int i;
    for(i=0;i<n;i++) {
        printf(" %5d %10.5f\n",i,x[i]);
    }
    printf("\n");*/
}

void printbits(int n) {
    int u, i;
    n = n & 0xFF;
    for (i = 1; i <= 8; i++) {
        u = n & (1 << (8 - i));
        //printf("%i", u ? 1 : 0);
    }
    printf("\n");
}

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
/*
 * overlap-save convolution
 *
 * \author Gregor Heinrich, Arbylon / for Fraunhofer IGD
 * \date 20061912
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// TODO: handle pre-delay and non-minimal phase systems

void ols(complex* x, int xlen, complex* h, int hlen, complex* y, int ylen, int m) {
    //function y = ols(x, h, L)
    int L, M, step, k, i, frames;
    complex *frame, *H, *wf, *wi;

    L = 1 << m;

    frame = (complex*) calloc(L, sizeof(complex));
    H = (complex*) calloc(L, sizeof(complex));
    wf = (complex*) malloc(L/2 * sizeof(complex));
    wi = (complex*) malloc(L/2 * sizeof(complex));

    //    H = fft(h, L);
    ffw(wf, 1, L);
    ffw(wi, -1, L);

    // zero padding and transformation
    memcpy(H, h, hlen * sizeof(complex));

    fft(H, wf, m);

    //printf("H transformed:\n");
    //printcvec(H, L);

    M = hlen;
    step = L - M;

    frames = (int) ceil((double)xlen/(double)step);
    //printf("frames: %i (xlen=%i, step=%i)\n", frames, xlen, step);
    for (k = 0; k < frames + 1; k++) {
        if (k == 0) {
            // zero-padding of first frame (by calloc, above)
            memcpy(frame + M, x, step * sizeof(complex));
            //printf("frame with zero padding:\n");
            //printcvec(frame, L);
        } else if ((k + 1) * step - M >= xlen) {
            // zero-padding of last frame
            memcpy(frame, x + k * step - M, (xlen + M - k * step) * sizeof(complex));
            for (i = xlen + M - k * step; i < L; i++) {
                frame[i].re = 0.;
                frame[i].im = 0.;
            }
        } else {
            // regular case
            memcpy(frame, x + k * step - M, L * sizeof(complex));
        }
        // in-place transform
        fft(frame, wf, m);
        for(i = 0; i < L; i++) {
            frame[i] = cmult(frame[i], H[i]);
        }
        // in-place transform
        ifft(frame, wi, m);
        //printf("aperiodic frame after %i steps:\n", k);
        //printcvec(frame + M, step);
        if ((k + 1) * step < ylen) {
            // truncate periodic portion
            memcpy(y + k * step, frame + M, step * sizeof(complex));
        } else {
            // end of y reached
            memcpy(y + k * step, frame + M, (ylen - k * step) * sizeof(complex));
        }
    }
    //printf("y after %i steps:\n", k);
    //printcvec(y, ylen);


    // FIXME: already solved? sometimes caused exception
    free(frame);
    free(H);
    free(wf);
    free(wi);
}

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
/*
 * time-partitioned OLS convolution, preparation for real-time operation.
 *
 * TODO: fully implement
 *
 * \author Gregor Heinrich, Arbylon / for Fraunhofer IGD
 * \date 20061912
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// TODO: implement zero-frames handling, group delay and fractional delays

// for index debugging: disable fft
//#define fft //fft

// could be as well filter* create(buflen, h, hlen, m), but then status
// handling is more difficult
int rtols_prepare(filter* filt, complex* h, int hlen, int m) {
    int i, j, r;

    // sharing mode: unshared
    filt->sharemode = RTOLS_SHARED_NONE;
    // m
    filt->m = m;
    // L: fft and signal frame size
    filt->L = 1 << m;
    // K: impulse response frame size (zero padded to L,
    // always = L/2, this is for notational convenience)
    filt->K = filt->L / 2;
    // S: signal stepping (always = K, this is for
    // notational convenience)
    filt->S = filt->L - filt->K;

    // @deplib: if purely complex fft is used, extra buffering is necessary,
    // otherwise signals can be assigned directly
    // x contains the current signal frame (with half-frame stepping = K)
    filt->x = (complex*) calloc(filt->L, sizeof(complex));
    // y contains the current convolution frame (full-frame stepping = K)
    // (not used)
    //filt->y = (complex*) calloc(filt->K, sizeof(complex));


    // make fft / ifft weights
    // @deplib: fft initialisation method
    filt->wf = (complex*) malloc(filt->K * sizeof(complex));
    filt->wi = (complex*) malloc(filt->K * sizeof(complex));
    // @parallel: share wf and wi for several filters
    ffw(filt->wf, 1, filt->L);
    ffw(filt->wi, -1, filt->L);

    // number of IR frames
    filt->R = (int) ceil(hlen/(double)filt->K);

    // allocate internal buffers
    filt->Y = (complex*) calloc(filt->L, sizeof(complex));
    filt->HH = (complex**) malloc(filt->R * sizeof(complex*));
    filt->XX = (complex**) malloc(filt->R * sizeof(complex*));
    for (i = 0; i < filt->R; i++) {
        // calloc for zero-padding
        filt->HH[i] = (complex*) calloc(filt->L, sizeof(complex));
        filt->XX[i] = (complex*) calloc(filt->L, sizeof(complex));
    }

    // TODO: handle filter transition fields HHnew and HHold

    // IR length
    filt->hlen = hlen;

    // segment IR
    segment_ir(h, hlen, filt->m, filt->wf, filt->HH);

    /*
    printf("HH:\n");
    for (r = 0; r < filt->R; r++) {
        printf("%i:\t", r);
        for (j = 0; j < filt->L; j++) {
            printf("%i\t", (int) filt->HH[r][j].re);
        }
        printf("\n");
    }
    */

    filt->finishing = 0;
    filt->hastail = 1;
    filt->hassignal = 1;
    filt->ylen = -1;
    filt->i = 0;
    return 0;
}

int rtols_prepare_sigshare(filter* filt, complex* h, int hlen, complex* wf, complex* wi,
                           complex** XX, int R, int m, int addout) {

    filt->sharemode = RTOLS_SHARED_FFT + RTOLS_SHARED_INPUT + (addout ? RTOLS_SHARED_OUTPUT_ADD : 0);
    // TODO: implement
    return -1;
}

int rtols_prepare_filtshare(filter* filt, complex* wf, complex* wi, complex** HH, int R, int hlen, int m) {

    filt->sharemode = RTOLS_SHARED_FFT + RTOLS_SHARED_FILTER;
    // TODO: implement
    return -1;

}

// @parallel: can be run in separate thread that has private locals and
// a critical section on outbuf writing
int rtols_filter(filter* filt, real* inbuf, real* outbuf) {
    // index variables
    int icirc, r, rcirc, j;
    // frame bounds
    int xxstart, xxend, rstart, rend, yylen, yyend;

    // if input frame buffers are shared, return with error
    if (filt->sharemode & RTOLS_SHARED_INPUT) {
        return RTOLS_ERROR_SHARED_INPUT;
    }

    /** BEGIN process frame **/
    if (!filt->finishing || filt->hassignal || filt->hastail) {
//         printf("+++++++++++++ loop %i ++++++++++++\n", filt->i);

        // circular index into array of signal spectra XX
        icirc = filt->i % filt->R;

        // index into x where current signal frame starts and ends
        xxstart = (filt->i) * filt->S;
        xxend = (filt->i + 1) * filt->S - 1;
        /*printf*/("xxstart = %i, xxend = %i\n", xxstart, xxend);

        if (!filt->finishing || xxend < filt->xlen) {

            // first half of the signal comes from the saved frame
            memcpy(filt->XX[icirc], filt->x, filt->K * sizeof(complex));

            // convert real input to complex matrix and save for next frame
            r2c(inbuf, filt->x, filt->K);

            // second half is current frame
            memcpy(filt->XX[icirc] + filt->K, filt->x, filt->K * sizeof(complex));

        } else if (!filt->finishing || xxstart < filt->xlen) {

            // incomplete signal frame -> must be zero-padded for transformation
            // @realtime: signal ending is started
            // first half of the signal comes from the saved frame
            memcpy(filt->XX[icirc], filt->x, filt->K * sizeof(complex));

            // convert real input to complex matrix and save for next frame
            r2c(inbuf, filt->x, filt->K);

            // second half is current, incomplete frame
            memcpy(filt->XX[icirc] + filt->K, filt->x, (filt->xlen - xxstart) * sizeof(complex));

            // fill up with zeros
            for (j = filt->K + filt->xlen - xxstart; j < filt->L; j++) {
                filt->XX[icirc][j].re = 0.;
                filt->XX[icirc][j].im = 0.;
            }

            /*printf*/("****************************************************\n");
            /*printf*/(" - loop %i: signal incomplete, padding %i zeroes\n", filt->i+1, xxend - filt->xlen + 1);
        } else {
            // @realtime: there are no remaining samples from the input
            if (filt->hassignal) {
                // all signal frames processed
                filt->hassignal = 0;
                // xframes should be exactly = fullframes
                filt->xframes = filt->i - 1;
                /*printf*/("****************************************************\n");
                /*printf*/(" - loop %i: signal finished, processing tail only.\n", filt->i + 1);
            }
        }

        if (filt->i >= filt->R) {
            // buffer already filled
            rend = filt->R;
        } else {
            // buffer being filled
            rend = filt->i + 1;
        }

        // @parallel: management of XX can be shared between filters if
        // they use the same signal
        if (filt->hassignal) {
            rstart = 0;
            // generate spectrum from new signal frame
            // @deplib: in-place transform
            fft(filt->XX[icirc], filt->wf, filt->m);
        } else {
            // @realtime: finishing convolution
            // -1 because rstart is 0-offset
            rstart = filt->i - filt->xframes;
        }

        if (filt->finishing) {
            yyend = filt->S * (filt->i + 1) - 1;
            /*printf*/("yyend = %i\n", yyend);
            if (yyend >= filt->ylen) {
                filt->hassignal = 0;
                filt->hastail = 0;
            }
        }

        if (filt->hastail) {
            yylen = filt->S;
        } else {
            // + 1 because of yyend
            yylen = filt->S - (yyend + 1 - filt->ylen);
        }

        /*
        printf("XX:\n");
        for (r = 0; r < filt->R; r++) {
            printf("%i:%s\t", r, r == icirc ? ">": " ");
            for (j = 0; j < filt->L; j++) {
                printf("%i\t", (int) filt->XX[r][j].re);
            }
            printf("\n");
        }
        */

        // @parallel: loops over r can be done in parallel, result can
        // be collected by adding in y
        //TODO: #pragma omp parallel for default(private) shared(XX, filt, y, wr, wi)
        for (r = rstart; r < rend; r++) {
            // circular index into XX
            rcirc = (filt->i - r) % filt->R;

            //printf("convolve XX[%i] ** HH[%i]\n", rcirc, r);
            //printcvec(filt->XX[rcirc], filt->L);
            //printcvec(filt->HH[r], filt->L);

            // partial convolution
            // @deplib: use SIMD
            for(j = 0; j < filt->L; j++) {
                filt->Y[j] = cmult(filt->XX[rcirc][j], filt->HH[r][j]);
            }

            // in-place transform
            ifft(filt->Y, filt->wi, filt->m);

            //printf("ifft(Y):\n");
            //printcvec(filt->Y, filt->L);

            // add last L-K points to calloced y (L-K = K)
            for (j = 0; j < yylen; j++) {
                //filt->y[j] = cadd(filt->y[j], filt->Y[j + filt->K]);
                if (r == rstart) {
                    outbuf[j] = filt->Y[j + filt->K].re;
                } else {
                    outbuf[j] += filt->Y[j + filt->K].re;
                }
            }
        }
        // fill up output with zeros (would an if clause be faster?)
        for (j = yylen; j < filt->K; j++) {
            outbuf[j] = 0.;
        }

        filt->i++;
    } else {
        // fill output buffer with zeros
        for (j = 0; j < filt->K; j++) {
            outbuf[j] = 0.;
        }
        return -1;
    } // if !finishing || hassignal || hastail

    /** END process frame **/

    // convert output (real part) --> no need as we can directly access outbuf
    //c2r(filt->y, outbuf, filt->K, 0);

    return yylen;
}

int rtols_filter_sigshare(filter* filters, int nfilt, real* inbuf, real** outbuf, int outoffset) {
    int i, q;

    // TODO: implement
    for (i = 0; i < nfilt; i++) {
        // if input frame buffers are not shared, return with error
        if (!(filters[i].sharemode & RTOLS_SHARED_INPUT)) {
            return RTOLS_ERROR_UNSHARED_INPUT;
        }
    }

    // TODO: modularise rtols_filter() to make subroutines from here

    // create new frame from input

    for (q = 0; q < nfilt; q++) {
        // perform partial convolution on filter

        // ...

        // write to associated output buffer
        if (filters[q].sharemode & RTOLS_SHARED_OUTPUT_ADD) {
            // TODO: before calling the first rtols_filter_sigshare, output buffer must
            // be reset (e.g., first filter in loop could have unset OUTPUT_ADD flag).

            // @parallel: BEGIN critical section here between calls to rtols_filter_sigshare
            //#pragma omp parallel section

            // add to buffer content
            // TODO: check whether omp reduction processing applies here

            // @parallel: END critical section

        } else {
            // replace buffer content
        }
    }
    return 0;
}

int rtols_filter_update(filter* filt, complex* h, int hlen, int transition) {
    // TODO: implement: frequency-domain interpolation
    return -1;
}

int rtols_finish(filter* filt, int len) {
    int framesleft;

    // @realtime: begin finishing sequence
    filt->finishing = 1;
    // @realtime: now the signal length is known
    filt->xlen = filt->i * filt->S + len;
    filt->ylen = filt->hlen + filt->xlen - 1;

    framesleft = (int) ceil((double)filt->ylen / filt->S) - filt->i;
    return framesleft;
}

int rtols_restart(filter* filt) {
    int i;
    if (!filt->finishing || filt->hassignal || filt->hastail) {
        return -1;
    }
    for (i = 0; i < filt->K; i++) {
        filt->x[i].re = 0.;
        //filt->x[i].im = 0.;
    }
    filt->finishing = 0;
    filt->hassignal = 1;
    filt->hastail = 1;
    filt->ylen = -1;
    filt->i = 0;
    return 0;
}

int rtols_cleanup(filter* filt) {
    int r;

    free(filt->x);
    //free(filt->y);
    free(filt->Y);

    // @deplib: cleanup fft-specific data (e.g., fftw_destroy in fftw3)
    if (!(filt->sharemode & RTOLS_SHARED_FFT)) {
        free(filt->wf);
        free(filt->wi);
    }


    if (!(filt->sharemode & RTOLS_SHARED_INPUT)) {
        for (r = 0; r < filt->R; r++) {
            free(filt->XX[r]);
        }
        free(filt->XX);
    }

    if (!(filt->sharemode & RTOLS_SHARED_FILTER)) {
        for (r = 0; r < filt->R; r++) {
            free(filt->HH[r]);
        }
        free(filt->HH);
    }

    // TODO: handle transitional filters, HHnew and HHold

    return 0;
}

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*!
 * driver function for a filter that can be hooked
 * to audio i/o buffers and callback: When the audio i/o framework
 * has finished filling a new input buffer, one step in the
 * while loop of this method is done, after the while step has been
 * finished (after a call to rtols_filter), the i/o framework is
 * notified and can play its output buffer.
 *
 * A note about conversion from tolstoy: Both versions of the program
 * are included in the toy application in order to pursue the development
 * process, which started at non-realtime algorithm development and then
 * was structured to be converted into a real-time capable approach. The
 * annotations made hint at the conversion steps.
 *
 * Variable name conventions: used common sense and avoided naming rule mania,
 * instead document with comments what's being done. Some conventions:
 * array sizes and other dimensions -- capital (mid alphabet) or *len;
 * constant -- uppercase; signals -- time domain lowercase, frequency domain
 * uppercase (h and end of alphabet): names after symbols common in associated
 * math, e.g., h, x, y, X, Y; arrays -- duplicate symbols, e.g., hh is an array
 * of h, i.e., impulse responses.
 *
 * Task annotation conventions: TODO: planned task, FIXME: known bug.
 *
 */
int rtconv() {
    const int NEWFILT = 20, STOP = 12, END = 20;
    int hlen, status, m, i, K, L, xlen, ylen;
    int running, frames, samples;
    complex *h;
    //complex *hnew;
    filter *filt;
    real *x, *y, *xpos, *ypos;

    // fft size setting
    m = 3;
    L = m << 1;

    ////////// define signals //////////////////
    // ``very long'' input signal (which will be switched off)
    // @realtime: it must be ensured from outside that the buffer exists
    // here: ensure that the signal is stopped before its end is reached
    xlen = 50;
    hlen = 12;//L * 4 + 1;
    ylen = xlen + hlen - 1;

    x = (real*) malloc(xlen * sizeof(real));
    h = (complex*) malloc(hlen * sizeof(complex));
    y = (real*) malloc(ylen * sizeof(real));
    xpos = x;
    ypos = y;

    // test data
    for(i=0;i<xlen;i++) {
        x[i] = i + 1;
        x[i] = 1;
    }

    for(i=0;i<hlen;i++) {
        h[i].re = i + 1;
        h[i].re = 1;
        h[i].im = 0;
    }

    /*printf*/("x sig:\n");
    printrvec(x, xlen);

    /*printf*/("h sig:\n");
    printcvec(h, hlen);
    /////////////////////////////////////////////////


    // determine buffer lengths according to fft size
    L = 1 << m;
    K = L / 2;

    filt = (filter*) malloc(sizeof(filter));
    status = rtols_prepare(filt, h, hlen, m);

    running = 0;


    // TODO: hook to audio i/o buffers and callbacks.
    // Input and output buffers are best a power of 2, one lower than
    // the power used for the fft.
    while (running < END) {

        // update filter coefficients, 3 frames transition time
        // TODO: what happens if the filter is still changing and
        // rtols_filter_update is called again?
        if (running == NEWFILT) {
            //hnew = (complex*) calloc(hlen, sizeof(complex));
            //status = rtols_filter_update(filt, hnew, hlen, 3);
        }

        // stop the signal, 5 more input samples
        if (running == STOP) {
            /*printf*/("************* stop filter after 2 samples *************\n");
            frames = rtols_finish(filt, 3);
            /*printf*/("************* %i output frames remaining ****************\n", frames);
            frames += running;
            //free(hnew);
        }

        // restart the signal 2 frames after it has finished
        //if (running == frames + 2) {
        //    status = rtols_restart(filt);
        //}

        if (filt->hassignal) {
            //printf("BEGIN filter step\n");
            //printf("frame %i, xbuf\n", running);
            //printrvec(xpos, K);
        }

        // @realtime: entry point for audio i/o: "input buffer ready, [K] samples"
        // @realtime: after signal end, this must be called until the signal tail
        // has been played completely.
        samples = rtols_filter(filt, xpos, ypos);

        /*printf*/(">>> filter returned %i samples output\n", samples);

        // @realtime: calling point for audio i/o: "output buffer ready, [samples] samples"

        //printf("END filter step\n");

        if (samples > 0) {
            /*printf*/(">>> frame %i, ybuf:\n", running + 1);
            printrvec(ypos, samples);
        }

        // increment signal positions
        xpos += K;
        ypos += K;

        running++;
    }

    // deallocate
    rtols_cleanup(filt);
    free(filt);

    free(x);
    // FIXME: invalid block
    free(h);
    free(y);

    return 0;
}


/*!
 * for rtwfs(): make dummy irs (here: time domain, but easy to change to
 * transfer functions)
 */
// @realtime: wfs operator calculates primary-to-secondary impulse responses, here
// the format might be as well a transfer function and a delay.
void make_irs(complex*** hh, int P, int Q, int hlen) {
    int p, q, i;
    for (p = 0; p < P; p++) {
        for (q = 0; q < Q; q++) {
            for (i = 0; i < hlen; i++) {
                hh[p][q][i].re = i + 1;
                hh[p][q][i].im = 0.;
            }
        }
    }
}

/*!
 * outline for wave field synthesis run-time.
 *
 * @realtime: This function only shows the organisation principle of filters
 * and parallel handling requirements. An actual implementation needs to handle
 * other things in addition, e.g., calculation of impulse responses. Special
 * care must be taken in interaction between the hardware architecture (esp.
 * memory interaction) and the software.
 *
 */
// TODO: bundle filter matrix and i/o in a struct
int rtwfs() {
    const int STOP = 15, END = 25;
    int status, frames, samples;
    // number of primary sources and secondary sources (=at virtual object and speaker positions)
    int P, Q;
    // indices
    int i, p, q, running;
    // primary and secondary source signals (=input and output signals)
    real **xx, **yy;
    // signal offsets (here: ints because equal for all signals)
    int xxpos, yypos;
    // shared input spectrum buffer
    complex** XX;
    // filter impulse responses, P x Q x hlen
    complex*** hh;
    // spectrum frame buffer size (in frames)
    int R;
    // frame and fft sizes
    int K, L, m;
    // signal and ir lengths
    int xlen, ylen, hlen;
    // fft stuff
    complex *wf, *wi;
    // P x Q filters
    filter** ff;
    // temp filter
    filter* f;

    /** BEGIN allocation and initialisation **/

    //xlen = 100;
    //hlen = 10;
    xlen = 100;
    hlen = 10;
    P = 2;
    Q = 5;

    // create P x Q impulse responses
    // @realtime: updated after change in model geometry
    hh = (complex***) malloc(P * sizeof(complex**));
    for (p = 0; p < P; p++) {
        hh[p] = (complex**) malloc(Q * sizeof(complex*));
        for(q = 0; q < Q; q++) {
            hh[p][q] = (complex*) malloc(hlen * sizeof(complex));
        }
    }


    // create P x Q filters
    // @realtime: updated after change in model geometry
    ff = (filter**) malloc(P * sizeof(filter*));
    for (p = 0; p < P; p++) {
        ff[p] = (filter*) malloc(Q * sizeof(filter));
    }

    // make IRs
    make_irs(hh, P, Q, hlen);

    // @realtime: unknown
    ylen = xlen + hlen - 1;

    // create (dummy) signals
    // for each primary source
    // @realtime: buffers hooked to audio i/o
    xx = (real**) malloc(P * sizeof(real*));
    xxpos = 0;
    for (p = 0; p < P; p++) {
        xx[p] = (real*) malloc(xlen * sizeof(real));
    }
    // for each secondary source
    yy = (real**) malloc(Q * sizeof(real*));
    yypos = 0;
    for (q = 0; q < Q; q++) {
        yy[q] = (real*) malloc(ylen * sizeof(real));
    }

    // set (dummy) input signals
    for (p = 0; p < P; p++) {
        for (i = 0; i < xlen; i++) {
            xx[p][i] = i + 1;
        }
    }

    // fft size
    m = 4;
    L = 1 << m;
    K = L / 2;
    R = (int) ceil(hlen/K);

    // allocate
    wf = (complex*) malloc(L/2 * sizeof(complex));
    wi = (complex*) malloc(L/2 * sizeof(complex));


    // create fft coefficients (reused across filters)
    ffw(wf, 1, L);
    ffw(wi, -1, L);

    // for each primary source
    // @parallel: all preparation can be done in parallel
    // @realtime: depending on the audio i/o implementation, there might be
    // different triggers for input buffers, also output buffers need to be
    // blocked (appropriate delay!) until the last convolution result has been
    // added.
    //#pragma omp parallel for
    for (p = 0; p < P; p++) {
        // for each secondary source (we need one filter for every combination)
        // TODO: shadowed sources handling (all-zero IRs, dependent on geometry)

        // create input frame spectrum buffer shared by Q filters
        XX = (complex**) malloc(R * sizeof(complex*));
        for (i = 0; i < R; i++) {
            XX[i] = (complex*) malloc(L * sizeof(complex));
        }

        // for each seondary source
        for (q = 0; q < Q; q++) {
            // create filter from p to q with from impulse response from p to q
            status = rtols_prepare_sigshare(&ff[p][q], hh[p][q], hlen, wf, wi, &XX[p], R, m, 1);
        }
    }

    /** END allocation and initialisation **/


    /** BEGIN main loop **/

    running = 0;

    while (running < END) {

        // stop the signal, 30 more input samples
        if (running == STOP) {
            for (p = 0; p < P; p++) {
                for (q = 0; q < Q; q++) {
                    frames = rtols_finish(&ff[p][q], 30);
                }
            }
            frames += running;
        }


        /** BEGIN filtering step **/
        // additive output -> set output buffer to zero
        // TODO: handle this by setting the first filter sharemode to addout=false
        // for each secondary source
        for (q = 0; q < Q; q++) {
            // for each buffer sample
            for (i = 0; i < K; i++) {
                yy[q][i] = 0.;
            }
        }

        // run the filters
        // for each primary source
        // @parallel: all filters can run in parallel with the constraint of
        // a critical section on the output buffer addition routine and a
        // barrier at the end of the sequence that waits for the yy signal
        // to be output.
        for (p = 0; p < P; p++) {
            // determine influence of primary source p on each of the Q secondary sources
            samples = rtols_filter_sigshare(ff[p], Q, xx[p] + xxpos, yy, yypos);
        }
        /** END filtering step **/

        /*printf*/("frame %i, xbuf:\n", running + 1);
        printrvec(xx[0] + xxpos, K);
        /*printf*/("frame %i, ybuf:\n", running + 1);
        printrvec(yy[0] + yypos, K);


        // increment signal positions
        xxpos += K;
        yypos += K;

        running++;
    }

    /** END main loop **/

    // free fft resources
    free(wf);
    free(wi);

    /** BEGIN deallocation **/

    // free input and output signals
    // @realtime: handled by audio i/o
    for (p = 0; p < P; p++) {
        free(xx[p]);
    }
    for (q = 0; q < Q; q++) {
        free(yy[q]);
    }

    // free IRs
    for (p = 0; p < P; p++) {
        for(q = 0; q < Q; q++) {
            free(hh[p][q]);
        }
        free(hh[p]);
    }
    free(hh);

    // free shared input spectral frame buffers
    for (p = 0; p < P; p++) {
        f = ff[p];
        for (i = 0; i < R; i++) {
            free(f->XX[i]);
        }
        free(f->XX);
    }

    // free filters
    for (p = 0; p < P; p++) {
        free(ff[p]);
    }
    free(ff);

    /** END deallocation **/

    return 0;
}

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
/*!
 * Tolstoy: Time-partitioned OverLap-Save TOY application for the hArtes
 * project.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*!
 * test test overlap / save algorithm and its time-partitioned
 * equivalent.
 *
 */
void conv3() {
    int i, m, n, L;
    int xlen, hlen, ylen;
    complex *x, *y, *h;

    // fft size = 2^4
    m = 3;

    n = 1 << m;
    L = n/2;

    xlen = 17 ;//n * 30 - 1;
    hlen = 12;//L * 4 + 1;
    ylen = xlen + hlen;

    x = (complex*) malloc(xlen * sizeof(complex));
    h = (complex*) malloc(hlen * sizeof(complex));
    y = (complex*) calloc(ylen, sizeof(complex));

    // test data (for convolution, only half non-zero)
    for(i=0;i<xlen;i++) {
        x[i].re = i + 1;
        //x[i].re = 1;
        x[i].im = 0;
    }

    for(i=0;i<hlen;i++) {
        h[i].re = i + 1;
        //h[i].re = 1;
        h[i].im = 0;
    }

    /*printf*/("x sig:\n");
    printcvec(x, xlen);

    /*printf*/("h sig:\n");
    printcvec(h, hlen);

    tols(x, xlen, h, hlen, y, ylen, m);
    /*printf*/("TOLS convolution: y = x ** h:\n");
    printcvec(y, ylen - 1);

    free(x);
    // FIXME: already solved? sometimes caused exception
    free(h);
    free(y);
}

#define SWAP(a,b) temp = (a); (a) = (b); (b) = temp
void testperm() {
    complex* x, *y;
    int i, N, m;
    double temp;
    m = 4;
    N = 1 << m;


    x = (complex*) malloc(N * sizeof(complex));
    y = (complex*) malloc(N * sizeof(complex));
    for (i = 0; i < N; i++) {
        x[i].re = i;
        x[i].im = 0;
        y[i].re = i;
        y[i].im = 0;
    }
    for (i=0; i<N/2; i++) {
        SWAP(x[i].re, x[i + N/2].re);
    }
    //printcvec(x, N);
    printcvec(x, N);
    perm(x, m);
    printcvec(x, N);
    permrev(y, m);
    printcvec(y, N);

}


int main(int argc, char **argv) {
    //conv1();
    //conv2();
    int i = 0;
    #ifndef SHORT_BENCH
    for(i = 0; i < 5; i++){
    #endif
        conv3();
        rtconv();
        testperm();
    #ifndef SHORT_BENCH
    }
    #endif


    return 0;

}

/*************************************************************
 *
 * Copyright 2006/7 Gregor Heinrich/Arbylon and Fraunhofer IGD.
 *
 * Only for usage within hArtes tool-chain development. No
 * warranties for functional or structural features of the code
 * or resulting binary applications. Usage in any applications
 * requires the consent of copyright holders.
 *
 *************************************************************/
/*
 * overlap-save convolution, time-partitioned implementation
 *
 * Annotation legend:
 * @deplib: affects the use or/requirements for dependent libraries (e.g., fft implementations)
 * @matlab: comparison to the equivalent matlab script
 * @parallel: covers aspects of parallel processing
 * @realtime: considerations for real-time development
 *
 * A note about matlab conversion: matlab has subscripts starting at 1 (1-offset),
 * therefore (1) loop-starting conditions and (2) indices need to be used diminished by 1
 * in C, whereas (1) loop ending conditions (using <) and (2) dimensions are equal.
 *
 * \author Gregor Heinrich, Arbylon / for Fraunhofer IGD
 * \date 20061912
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// for index debugging: disable fft
//#define fft //fft

// TODO: handle non-minimal phase IRs
// TODO: simplify the signal tail conditions

int tols(complex* x, int xlen, complex* h, int hlen, complex* y, int ylen, int m) {


    /** General considerations **/

    /*
     * @realtime: The largest difference to real-time code is that the length of
     * a signal is unknown a priori and therefore x and y are buffers with constant
     * length that can be used frame-wise in the main loop of the algorithm. Thus,
     * we give hints at restructuring this script in a setup, a callback and a
     * cleanup part. The callback is controlled by the surrounding i/o architecture.
     */

    /*
     * @deplib: the current implementation of the complex operations (including fft)
     * are included in a simple implementation to keep the toy application self-
     * contained. They need to be replaced by the appropriate runtime implementation
     * of the target platform, e.g., IPP or the Diopsis DSP library. Specifically,
     * the code has been built to use in-place transforms, which are more performant
     * in many implementations. The purely complex transforms used here are due to
     * their simple implementation, but in the real setting we need real signal input
     * and complex impulse responses and use real results.
     */

    /** variable declarations **/

    // total frame counter
    unsigned long i;
    // dimension variables
    int K, L, R, xframes;
    // index variables
    int icirc, r, rcirc, j;
    // frame bounds
    int xxstart, xxend, rstart, rend, yylen, yyend;
    // status variables
    int hastail, hassignal;
    // internal buffers:
    // HH: impulse response spectra (indexed by r),
    // XX: circular buffer for signal frame spectra (indexed by icirc and rcirc),
    // Y: spectral product
    // @parallel: share XX for several filters
    complex **HH, **XX, *Y;
    // @deplib: persistent fft data (e.g., fftw_plan with fftw3)
    // @parallel: share fft persistence between several filters
    complex *wf, *wi;

    /******************** @realtime: BEGIN setup phase ***************/

    // L: fft and signal frame size
    L = 1 << m;
    // K: impulse response frame size (zero padded to L,
    // always = L/2 = S)
    K = L / 2;

    // expected length of convolution
    // @realtime: unknown at this point
    ylen = xlen + hlen - 1;

    // make fft / ifft weights
    // @deplib: fft initialisation method
    wf = (complex*) malloc(K * sizeof(complex));
    wi = (complex*) malloc(K * sizeof(complex));
    ffw(wf, 1, L);
    ffw(wi, -1, L);

    // number of IR frames
    R = (int) ceil(hlen/(double)K);

    // allocate internal buffers
    Y = (complex*) calloc(L, sizeof(complex));
    HH = (complex**) malloc(R * sizeof(complex*));
    XX = (complex**) malloc(R * sizeof(complex*));
    for (i = 0; i < R; i++) {
        // calloc for zero-padding
        HH[i] = (complex*) calloc(L, sizeof(complex));
        XX[i] = (complex*) calloc(L, sizeof(complex));
    }

    // segment IR
    segment_ir(h, hlen, m, wf, HH);


    for (j = 0; j < R; j++) {
        /*printf*/("HH[%i]:\n", j);
        printcvec(HH[j], L);
    }

    hastail = 1;
    hassignal = 1;
    i = 0;

    // xxstart and xxend are positions of the current frame
    xxstart = - K;
    xxend = K - 1;

    /****************** @realtime: END setup phase **********************/

    /*************** @realtime: BEGIN callback routine ******************/
    // for each frame
    while (hassignal || hastail) {
        // circular index into array of signal spectra XX
        icirc = i % R;
        if (xxend < xlen) {
            // complete signal frame
            if (i == 0) {
                // zero-padding of first frame (by calloc, above)
                memcpy(XX[icirc] + K, x, K * sizeof(complex));
            } else {
                // copy frame into circular frame buffer
                memcpy(XX[icirc], x + xxstart, L * sizeof(complex));
            }
        } else if (xxstart < xlen) {
            // incomplete signal frame -> must be zero-padded for transformation
            // @realtime: signal ending is started
            memcpy(XX[icirc], x + xxstart, (xlen - xxstart) * sizeof(complex));
            for (j = L - (xlen - xxstart); j < L; j++) {
                XX[icirc][j].re = 0.;
                XX[icirc][j].im = 0.;
            }
            //printf("****************************************************\n");
            //printf(" - loop %i: signal incomplete, padding %i zeroes\n", i+1, xxend - xlen + 1);
        } else {
            // @realtime: there are no samples from the input left
            if (hassignal) {
                // all signal frames processed
                hassignal = 0;
                // xframes should be exactly = fullframes
                xframes = i - 1;
                //printf("****************************************************\n");
                //printf(" - loop %i: signal finished, processing tail only.\n", i + 1);
            }
        }

        if (i >= R) {
            // buffer already filled
            rend = R;
        } else {
            // buffer being filled
            rend = i + 1;
        }

        /* @matlab:
        if hassignal
            X = fft(xx, L);
            XX(:, icirc) = X;
            rstart = 1;
        else
            rstart = i - xframes + 1;
        end
        */

        // @parallel: management of XX can be shared between filters if
        // they use the same signal
        if (hassignal) {
            rstart = 0;
            // generate spectrum from new signal frame
            // @deplib: in-place transform
            fft(XX[icirc], wf, m);
        } else {
            // @realtime: finishing convolution
            // -1 because rstart is 0-offset
            rstart = i - xframes;
        }

        /* @matlab:
        % end of output larger than expected result?
        yyend = S*(i+1);
        if yyend > ylen
            hassignal = 0;
            hastail = 0;
            loops = i;
        end
        */

        // TODO: - 1?
        yyend = K * (i + 1);
        if (yyend >= ylen) {
            hassignal = 0;
            hastail = 0;
        }


        /* @matlab:
        if hastail == 1
            yylen = S;
            y = [y; zeros(S,1)];
        else
            yylen = S - (yyend - ylen);
            y = [y; zeros(yylen,1)];
        end
        */

        if (hastail) {
            yylen = K;
        } else {
            // + 1 because of yyend
            yylen = K - (yyend - ylen) + 1;
        }

        // TODO: ensure ylen = xlen + hlen - 1
        if (hassignal == 0) {
            /*printf*/("yylen = %i, yyend = %i, ylen = %i\n", yylen, yyend, ylen);
        }

        // add most recent frame to convolution result

        //printf("**********************\n");
        //printf("x[i=%i] => XX[icirc=%i]\n", i, icirc);


        /* @matlab:
        for r=rstart:rend
            rcirc = mod(i - (r - 1), R) + 1;
            Y = XX(:,rcirc) .* HH(:,r);
            // @deplib: in-place transform
            yy = ifft(Y, L);
            yy = yy(K+1:L);
            y(end-yylen+1:end) = y(end-yylen+1:end) + yy(1:yylen);
        end
        */


//         printf("XX:\n");
//         for (r = 0; r < R; r++) {
//             printf("%i:%s\t", r, r == icirc ? ">": " ");
//             for (j = 0; j < L; j++) {
//                 printf("%i\t", (int) XX[r][j].re);
//             }
//             printf("\n");
//         }

        // @parallel: loops over r can be done in parallel, result can
        // be collected by adding in y
        //#pragma omp parallel for
        for (r = rstart; r < rend; r++) {
            // circular index into XX
            rcirc = (i - r) % R;


//             printf("convolve XX[%i] ** HH[%i]\n", rcirc, r);
//             printf(" - XX[%i] = \n", rcirc);
//             printcvec(XX[rcirc], L);
//
//             printf(" - HH[%i]\n", r);
//             printcvec(HH[r],L);


            // partial convolution
            for(j = 0; j < L; j++) {
                Y[j] = cmult(XX[rcirc][j], HH[r][j]);
            }

            // in-place transform
            ifft(Y, wi, m);

            //printf("Y\n");
            //printcvec(Y, L);

            // add last L-K points to calloced y
            for (j = 0; j < yylen; j++) {
                y[i * K + j] = cadd(y[i * K + j], Y[K + j]);
            }
            //printf(" - add to y[%i..%i]\n", i*K, i*K+K-1);
        }
        i++;
        xxstart += K;
        xxend += K;
        // last tail frame
        if (!hassignal && (rstart == rend)) {
            // @real-time: convolution ending finished
            hastail = 0;
        }
    } // while
    /******************** @realtime: END callback routine *******************/

    /******************** @realtime: BEGIN cleanup routine ******************/
    // de-allocation
    for (i = 0; i < R; i++) {
        free(HH[i]);
        free(XX[i]);
    }
    free(HH);
    free(XX);
    free(Y);
    // @deplib: cleanup fft-specific data (e.g., fftw_destroy in fftw3)
    free(wf);
    free(wi);
    /********************* @realtime: END cleanup routine *******************/

    return 0;

} // tols

// TODO: here int *R could be an out parameter but then the malloc size would be unknown for HH
void segment_ir(complex* h, int hlen, int m, complex* wf, complex** HH) {
    int K, L, i, j, n, R;

	L = 1 << m;
    K = L / 2;

    R = (int) ceil(hlen/(double)K);

    /* @matlab:
    for i=1:fullframes
        ...
    end
    if mod(length(h), K) ~= 0
        % zero-pad and process last ir frame
        ...
    end
    */

    // @parallel: collect frames in HH
    //#pragma omp parallel for
/*    for (i = 0; i < R; i++) {
        if (i < R - 1) {
            // complete frames
		    memcpy(HH[i], h + K * i, K * sizeof(complex));
        } else {
            n = K * i;
            if (hlen > n) {
          	    // last, incomplete frame
                printf("hlen = %i, n = %i, hlen - n = %i \n", hlen, K*i, hlen - n);
                memcpy(HH[i], h + n, (hlen - n) * sizeof(complex));
                for (j = hlen - n; j < L; j++) {
                    HH[i][j].re = 0.;
                    HH[i][j].im = 0.;
                }
            } else {
                return;
            }
        }
        // @deplib: in-place transform
        fft(HH[i], wf, m);
    }*/
   int ret = 1;
   for (i = 0; i < R && ret; i++)
   {
      if (i < R - 1)
      {
         // complete frames
         memcpy(HH[i], h + K * i, K * sizeof(complex));
      }
      else
      {
         n = K * i;
         if (hlen > n)
         {
            // last, incomplete frame
            /*printf*/("hlen = %i, n = %i, hlen - n = %i \n", hlen, K*i, hlen - n);
            memcpy(HH[i], h + n, (hlen - n) * sizeof(complex));
            for (j = hlen - n; j < L; j++)
            {
               HH[i][j].re = 0.;
               HH[i][j].im = 0.;
            }
         }
         else
         {
            ret = 0;
         }
      }
      if(ret)
      // @deplib: in-place transform
        fft(HH[i], wf, m);
   }
} // segment_ir

