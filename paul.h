#pragma once 
/*
like strtod in stdlib, but supports only decimal numbers of the form

-?[0-9]*.[0-9]*

stops parsing when
* endptr is reached
* an invalid character is read

and returns the current intermediate result
*/
double strtod_fast(const char* str, char** endptr);
float strtof_fast(const char* str, char** endptr);

const int dtostr_fast_extra_chars = 1 + 1 + 2 + 3; //  7;
/* Writes exactly 
1 (+-) + digits + 1 (.) + 2 (e +/-) + 3 (exponent)
i.e. digits + 7 characters in the format
    %e, that is 
to str

returns pointer to string after final position 
*/
void dtostr_fast(
    double value,
    int digits,
    char *str,
    char** endptr
    );

void ftostr_fast(
    float value,
    int digits,
    char *str,
    char** endptr
    );

// writes digits + 1 characters with the last character being at last
// pads left with 0
void itoa_backwards_signed(int digits, int i, char* last);


// writes digits + 1 characters, inserting a dot before the final digit
void ulltoa_backwards_dotted(int digits, long long i, char* last);

// exercises ftostr_fast etc.
void demo1();

#ifdef __CUDACC__
#define PAULFUNC inline __host__ __device__
#else
#define PAULFUNC inline
#endif

// yields True if n is divisible by m, and yields False if it is not. 
template<typename T1, typename T2>
PAULFUNC bool divisible(T1 n, T2 m) {
    return n % m == 0;
}

PAULFUNC bool aligned(void* p, unsigned int m) {
    return divisible((unsigned long long) p, m);
}

// TODO 32-bit version could be more efficient
PAULFUNC bool after(void* a, void* b) {
    return (unsigned long long)a >= (unsigned long long)b;
}
PAULFUNC bool strictly_after(void* a, void* b) {
    return (unsigned long long)a > (unsigned long long)b;
}

PAULFUNC int nextEven(int i) {
    if (i % 2 == 0) return i;
    return i + 1;
}

#include <assert.h>
#include <float.h>

// TODO better device implementation
PAULFUNC float assertFinite(float value) {
#ifdef _DEBUG
#ifdef __CUDACC__
    assert(1.f * value == value);
#else
    assert(_fpclass(value) == _FPCLASS_PD || _fpclass(value) == _FPCLASS_PN || _fpclass(value) == _FPCLASS_PZ ||
        _fpclass(value) == _FPCLASS_ND || _fpclass(value) == _FPCLASS_NN || _fpclass(value) == _FPCLASS_NZ);
        //, "value = %f is not finite", value);
#endif
#endif
    return value;
}
