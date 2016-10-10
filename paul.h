#pragma once 

// Standard headers

#include <sal.h>  // c.f. sal.txt

#define NOMINMAX
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h> // OutputDebugStringA, DebugBreak

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <memory.h> // just memset, but not malloc/free (no (standard) new/delete either
#include <string.h>

#include <iostream>
#include <unordered_map>
#include <type_traits> // syntax hacks
#include <vector>
#include <functional>
#include <algorithm>
using namespace std;

// from now on, be very strict
#pragma warning(push, 4)
#pragma warning(error : 4717) // recursive on all control paths


#ifndef _WIN64
#error Must be compiled in 64 bits.
#endif



















/*

CUDA/C agnostic framework begins

FUNCTION declared functions can be called from CPU and GPU. Use this whenever possible.
FUNCTION(OUTPUT, NAME, INPUTS, DOCUMENTATION, [PURITY])
where PURITY is one of PURITY_

if missing, the function is a procedure in the classical sense, with arbitrary side-effects
and affected by arbitrary system state

if present, the function is pure in that the collection of input bits uniquely determine the
output bits. The qualifiers specify how output bits are returned (directly or via PURITY_OUTPUT_POINTERS) and whether some of
the input bits are implicit (via PURITY_ENVIRONMENT_DEPENDENT)
functions that use assert or logging are not technically pure, but considered as such because
these features can be turned off after development

CPU_FUNCTION and CUDA_FUNCTION denote functions that use features only available on either platform
(e.g. dues to CUDA API limitations or extensions).

*/

// The declared function depends only on the input arguments and does not affect global state just returns a value.
// Such a function cannot be void.
// Like __attribute__((const)): Does not examine any values except their arguments, and have no effects except the return value */
// Such functions, when given all of its dependencies (which should also be PURITY_PURE function)
// can be reused easily in other projects. When not actually used, there is no cost involved in them.
// -
// The reasons to use and create any impure functions are input-output and optimization.
// This may be used for functions taking const pointers/references, though technically then these are dependent on the state of 
// memory.
// All parameters should be const.
// Note that custom operators etc. might still make this function behave in an impure way.
#define PURITY_PURE

// The declared function depends only on the input arguments and 
// affects only memory pointed to by its _(In)out_ arguments (directly or indirectly).
// If these arguments are used elsewhere, such a function can be considered having an (user initiated, obvious) side effect.
#define PURITY_OUTPUT_POINTERS

// The operation of the function depends on some parts of the global environment (on which it thus implicitly depends)
// but the function does not modify this environment.
#define PURITY_ENVIRONMENT_DEPENDENT


#ifdef __CUDA_ARCH__
// use with #if -within- CUDA (__device__) functions -- always 0 outside? Nah, then assert would not work properly
#define GPU_CODE                1
#define CPU_AND_GPU_CONSTANT    __constant__ const
#else
#define GPU_CODE                0
#define CPU_AND_GPU_CONSTANT const
#endif

#define GPU_ONLY                __device__

// Declares a pointer to device-only memory.
// Used instead of the h_ <-> d_ convention for naming pointers
#define DEVICEPTR(mem)          mem

// Declares a pointer to device-only memory which is __shared__ among the threads of the current block
#define SHAREDPTR(mem)          mem

#define KERNEL                  __global__ void

#if defined(__CUDACC__) && defined(__CUDA_ARCH__)
#define CPU_AND_GPU __device__
#else
#define CPU_AND_GPU 
#endif


#ifdef __CUDACC__
#define GLOBAL(type, name, defaultValue, usage) __managed__ type name = defaultValue
#define GLOBALDYNAMICARRAY(elementtype, name, sizevar, usage)  __managed__ int sizevar = 0; /*lengths are int because that's what WSTP expects -- it doesn't seem to support larger arrays (at least not sent at once) -- should be size_t*/ __managed__ elementtype* name = 0;

#define GLOBALDYNAMICARRAY_SHAREDLENGTH(elementtype, name, sizevar, usage) __managed__ elementtype* name = 0; // TODO would be nice if it was detected that the variable already exists
#else
#define GLOBAL(type, name, defaultValue, usage) type name = defaultValue
#define GLOBALDYNAMICARRAY(elementtype, name, sizevar, usage)  int sizevar = 0; /*lengths are int because that's what WSTP expects -- it doesn't seem to support larger arrays (at least not sent at once) -- should be size_t*/ elementtype* name = 0;

#define GLOBALDYNAMICARRAY_SHAREDLENGTH(elementtype, name, sizevar, usage) elementtype* name = 0; // TODO would be nice if it was detected that the variable already exists

#endif

// Read-only global data
#ifdef __CUDA_ARCH__
#define CONSTANTD const __constant__ 
#else
#define CONSTANTD const 
#endif

#ifdef __CUDA_ARCH__
#define CONSTANT(type, name, value, usage) const __constant__ type name = value
#else
#define CONSTANT(type, name, value, usage) const type name = value
#endif

// TODO support template parameters
// Prefer this for new functions
// declarative, CUDA agnostic definition of functions
// use only for arguments of types that compile for CUDA (i.e. not std:: types)
// usage intended for automatic extraction by tools and for runtime availability (inspection)
#ifdef __CUDACC__
#define FUNCTION(ret, name, args, usage) /** usage */ __host__ __device__ ret name args
#define MEMBERFUNCTION(ret, name, args, usage) /** usage */ __host__ __device__ ret name args // memberfunctions are not straightforward to call from mathematica/from outside, so special declaration
#else
#define FUNCTION(ret, name, args, usage, ...) /** usage */ ret name args
#define MEMBERFUNCTION(ret, name, args, usage, ...) /** usage */ ret name args
#endif


// Special:

// Denotes a function that uses GPU features (threadIdx, __shared__)
// TODO: maybe code can be made portable as follows: threadIdx is defined as 1 for CPU functions, __shared__ is ignored
#define CUDA_FUNCTION(ret, name, args, usage, ...) __device__ ret name args 
#define CUDA_MEMBERFUNCTION(ret, name, args, usage, ...) __device__ ret name args 

// Alternative declaration for CPU only functions
// shows that this function doesn't work on CUDA for one reason or other (e.g. using std types)
// can call LAUNCH_KERNEL
#define CPU_FUNCTION(ret, name, args, usage, ...) ret name args
#define CPU_MEMBERFUNCTION(ret, name, args, usage, ...) ret name args























// Sequence@@p
#define xyz(p) p.x, p.y, p.z
#define comp012(p) p[0], p[1], p[2]
#define xy(p) p.x, p.y













// Linearize xyz coordinate of thread into 3 arguments
#define threadIdx_xyz xyz(threadIdx)























#ifndef PAUL_NO_ASSERT

// Better assert
#pragma warning(disable : 4003) // assert does not need "commentFormat" and its arguments
#undef assert
#if GPU_CODE
// #include <assert.h>
#define assert(x,commentFormat,...) {if(!(x)) {printf("%s(%i) : Assertion failed : %s.\n\tblockIdx %d %d %d, threadIdx %d %d %d\n\t<" commentFormat ">\n", __FILE__, __LINE__, #x, xyz(blockIdx), xyz(threadIdx), __VA_ARGS__); *(int*)0 = 0;/* asm("trap;"); illegal instruction*/} }
#else

#define assert(x,commentFormat,...) {if(!(x)) {char s[10000]; sprintf_s(s, "%s(%i) : Assertion failed : %s.\n\t<" commentFormat ">\n", __FILE__, __LINE__, #x, __VA_ARGS__); /*notify on all channels*/ {puts(s);MessageBoxA(0,s,"Assertion failed",0);OutputDebugStringA(s);} /*flushStd();*/  DebugBreak();  }}
#endif

// assert(false) - wrapper
#define FATAL_ERROR false // make assertion more readable
#define fatalError(commentFormat, ...) \
    __pragma(warning(push))\
    __pragma(warning(disable:4127)) /*Conditional Expression is Constant in assert(false)*/\
    assert(FATAL_ERROR, "Fatal Error: " commentFormat, __VA_ARGS__); \
    __pragma(warning(pop))



#endif


































// Custom C-extensions
// FOR with const iterating variable
// for (int i = 0; i < 10; i++) i+=10; is legal but bad
// FOR(int,i,0,10) i+=10; is illegal

// Prefixes a block executed once with decl in place, prefix version of {decl; ...}
// e.g. BLOCK_DECLARE(const auto var = 1) {...}
#define BLOCK_DECLARE(decl) \
    if (bool a__ = true) for (decl;a__;a__=false)

// Creates a copy of var which is const, to ensure var is not accidentally modified in the block that follows
// e.g. MAKE_CONST(x) {...}
#define MAKE_CONST(var) \
    BLOCK_DECLARE(auto var##_ = var) BLOCK_DECLARE(const auto var = var##_)

// for (type var = start; var < maxExclusive; var += inc) {...}
// such that var is const within {...}
#define FOR(type, var, start, maxExclusive, inc) \
    for (type var##_ = (start); var##_ < (maxExclusive); var##_ += (inc)) BLOCK_DECLARE(const type var = var##_)

// Variants of FOR with fixed types and iteration boundaries

#define FOR1(type, var, start, maxExclusive) \
    FOR(type, var, start, maxExclusive, 1)
#define FOR01(type, var, maxExclusive) \
    FOR(type, var, 0, maxExclusive, 1) 
#define FOR01S(var, maxExclusive) \
    FOR(unsigned int, var, 0, maxExclusive, 1) // any reason not to use size_t here?
#define FOR1NS(var, maxInclusive) \
    FOR(unsigned int, var, 1, (maxInclusive)+1, 1)

// Like Mathematica's Do[..., {var, max}], but starts at 0 and goes to maxExclusive-1
// var is const within the loop body and is of unspecified integral type.
#define DO(var, maxExclusive)   FOR01S(var, maxExclusive) 

// Exactly like Do[..., {var, max}]: The block that follows is called with var from 1 to maxInclusive.
#define DO1(var, maxInclusive)  FOR1NS(var, maxInclusive)

// Exactly like Do[..., {var, min, max}]: The block that follows is called with integral var from min to maxInclusive.
#define DOINTERVAL(var, minInclusive, maxInclusive) static_assert(minInclusive <= maxInclusive, #minInclusive ", min must be <= max, " #maxInclusive);FOR1(int, var, minInclusive, (maxInclusive)+1)

// Repeatedly execute code with side-effects without any explicit counter
#define REPEAT(times) \
    DO(__i /*use uncommon/implementation reserved variable to avoid conflicts*/, (times)) \
    __pragma(warning(push)) \
    __pragma(warning(disable:4127)) /*Conditional Expression is Constant*/\
    if (false) DBG_UNREFERENCED_LOCAL_VARIABLE(__i); else /* get rid of unreferenced variable */\
    __pragma(warning(pop))

// for (auto& var : arr) for an arr with size given by arsz and with loop/index i
#define FOREACHi(i, var, arr, arsz) \
    DO(i, arsz) BLOCK_DECLARE(auto& var = arr[i])

// FOREACHi with i
#define FOREACH(var, arr, arsz)     FOREACHi(i, var, arr, arsz)

// for (const auto& var : arr), loop counter i
#define FOREACHCi(i, var, arr, arsz) \
    DO(i, arsz) BLOCK_DECLARE(auto const & var = arr[i]) 

#define FOREACHC(var, arr, arsz)    FOREACHCi(i, var, arr, arsz)































// Testing framework
// Note: CANNOT USE std::vector to track all listed tests (not yet initialized?)
// this ad-hoc implementation works
const unsigned int max_tests = 10000;
unsigned int _ntests = 0;
typedef void(*Test)(void);
Test _tests[max_tests] = {0};
const char* _test_names[max_tests] = {0};

// Purity: Has side effects, depends on environment.
CPU_FUNCTION(void, addTest, (_In_z_ const char*const n, void f(void)), "Purity: Has side effects, depends on environment.") {
    assert(_ntests < max_tests);
    _test_names[_ntests] = n;
    _tests[_ntests++] = f;
}


#define TESTMSGPREFIX() "======================== "

// Purity: Has side effects, depends on environment.
CPU_FUNCTION(void, runTests, (), "Purity: Has side effects, depends on environment."){
    DO(i, _ntests) {
        cout << TESTMSGPREFIX() << "Test " << i + 1 << "/" << _ntests << ": " << _test_names[i] << endl;
        _tests[i]();
    }
    cout << TESTMSGPREFIX() << "all tests passed" << endl;
}

// TODO could duplicate tests to CPU and GPU code. Should not matter for most.
#define TEST(name) void name(); struct T##name {T##name() {addTest(#name,name);}} _T##name; void name() 










TEST(testdofor) {
    DO(i, 10) {
        assert(i >= 0 && i < 10);
    }

    DO1(i, 10) {
        assert(i >= 1 && i <= 10);
    }
}




























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


template<typename T1, typename T2>
FUNCTION(
    bool
    , divisible
    , (T1 n, T2 m)
    , "C102... yields True if n is divisible by m, and yields False if it is not. TODO this makes sense only when % is defined appropriately for the given types - state this more abstractly, as a relation to * and integers maybe:"
    "n~divisible~m iff exists integer x such that x*m = n"
    "Integer might then be defined via the unit (1) element of * and addition (which must have the standard relation to *). Then we might have 0.5~divisible~0.25."
    "Undefined when m == 0"
    , PURITY_PURE){
    return n % m == 0;
}

TEST(divisible1) {
    assert(divisible(8u, 8u));
    assert(divisible(8, 8));
    assert(!divisible(7, 8));
    assert(divisible(8000000, 8));
    assert(!divisible(28, 8));
}


FUNCTION(unsigned int, restrictSize, (const size_t s), "Convert s to unsigned int, undefined (assertion error) if s is too large for this (loss of information would occur)", PURITY_PURE) {
    assert(s <= 0xffffffffu);
    return (unsigned int)s;
}


FUNCTION(bool, aligned,(void* p, unsigned int m), "Whether the pointer is aligned by the given size") {
    return divisible((unsigned long long) p, m);
}

TEST(aligned1) {
    assert(aligned((void*)0x4, 4u));
    assert(!aligned((void*)0x8, 6u));
    assert(aligned((void*)0x8, 8u));
}

FUNCTION(bool, after, (void* a, void* b), "C57") {
    return (unsigned long long)a >= (unsigned long long)b;
}

FUNCTION(bool, strictly_after, (void* a, void* b), "C58", PURITY_PURE) {
    return (unsigned long long)a > (unsigned long long)b;
}

FUNCTION(unsigned int, mod, (int n, unsigned int m), "mod(n, m) Mathematical mod function, returning positive values such that n = x*m + mod(n, m) for some x"
    "TODO extend, along with floor and ceil and mod to be more general (floor and ceil require < too...)", PURITY_PURE) {
    return ((n % m) + m) % m; // TODO could this lead to an overflow for large m? Restrict the range of m?
}

TEST(mod1) {
    assert(mod(-2, 4) == 2);
    assert(mod(2, 4) == 2);
    assert(mod(2, 2) == 0);
}

FUNCTION(int, nextEven, (int i), "C101 Gives the next even signed (two's complement) leint32, i.e. i if this is even, i+1 otherwise. Undefined for 2^31-1. TODO generalize this in a generalized ceil, c.f. Mathematica.", PURITY_PURE) {
    assert(i != INT_MAX);
    if (i % 2 == 0) return i;
    return i + 1;
}


// TODO better device implementation
FUNCTION(float, assertFinite, (float value)
    , "C56: The function that is the identity for finite single-precision (32 bit little endian) floating point values and undefined otherwise") {
#ifdef _DEBUG
#if GPU_CODE
    assert(1.f * value == value);
#else
    assert(_fpclass(value) == _FPCLASS_PD || _fpclass(value) == _FPCLASS_PN || _fpclass(value) == _FPCLASS_PZ ||
        _fpclass(value) == _FPCLASS_ND || _fpclass(value) == _FPCLASS_NN || _fpclass(value) == _FPCLASS_NZ);
        //, "value = %f is not finite", value);
#endif
#endif
    return value;
}


template<typename In, typename Out>
CPU_FUNCTION(
    bool
    ,definedQ
    ,(_In_ const unordered_map<In, Out>& f, _In_ const In& in, _Out_opt_ Out& out)
    ,"false if f is not defined for in.\
    out is undefined in that case\
    true and out is f[in] otherwise\
    In and Out must be valid unordered_map parameters."
    ,PURITY_OUTPUT_POINTERS
    )
    {
    auto outi = f.find(in);
    if (f.end() == outi) return false;
    out = outi->second;
    return true;
}

template<typename In, typename Out>
CPU_FUNCTION(
    bool
    ,definedQ
    ,(_In_ const unordered_map<In, Out>& f, _In_ const In& in)
    ,"true iff f is defined for in.\
    In and Out must be valid unordered_map parameters."
    ,PURITY_PURE) {
    auto outi = f.find(in);
    if (f.end() == outi) return false;
    return true;
}



FUNCTION(unsigned int, highest_bit_position, (unsigned int x), "C199", PURITY_PURE) {
    assert(x);
    unsigned int p = 0;
    unsigned int cp = 16;
    unsigned int c = 1 << cp;
    for (; cp; ((cp /= 2), (c = 1 << cp))) {
        if (x >= c) {
            p += cp;
            x >>= cp;
        }
    }
    return p;
}