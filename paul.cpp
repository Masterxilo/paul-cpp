template<typename T>
T strtodf_fast(const char* str, char** endptr) {
    double ifac = (T)10.0, ffac = (T)1.0, fdfac = (T)1.0;
    double x = (T)0.0;
    bool negative = false;

    if (*str == '-') {
        negative = true;
        str++;
    }

    for (; ((*str > '0' && *str < '9') || *str == '.') && *str; str++) {
        if (*str == '.') {
            ifac = (T)1.0; fdfac = ffac = (T)0.1;
            continue;
        }

        x *= ifac;
        x += (*str - '0') * ffac;
        ffac *= fdfac;
    }

    *endptr = (char*)str;

    return negative ? -x : x;
}

double strtod_fast(const char* str, char** endptr) {
    return strtodf_fast<double>(str, endptr);
}

float strtof_fast(const char* str, char** endptr) {
    return strtodf_fast<float>(str, endptr);
}

#include <math.h>

// writes digits + 1 characters (adds +- sign) with the last character being at last
// pads left with 0
void itoa_backwards_signed(int digits, int i, char* last) {
    bool negative = false;
    if (i < 0) {
        negative = true; i = -i;
    }

    for (; digits; digits--) {
        *last-- = '0' + i % 10;
        i /= 10;
    }
    if (negative) *last = '-';
    else *last = '+';
}

// writes digits + 1 characters, inserting a dot before the final digit
void ulltoa_backwards_dotted(int digits, long long i, char* last) {
    for (; digits>1; digits--) {
        *last-- = '0' + i % 10;
        i /= 10;
    }

    *last-- = '.';

    *last = '0' + i % 10;
}

template<typename T>
void dftostr_fast(
    T value,
    int digits,
    char *str,
    char** endptr
    ) {
    // special case (log undefined)
    if (value == 0.) {
        give0:
        *str++ = '+';
        *str++ = '0'; digits--;
        *str++ = '.';
        for (; digits; digits--) *str++ = '0';

        *str++ = 'e';
        *str++ = '+';
        *str++ = '0';
        *str++ = '0';
        *str++ = '0';

        // Point past end - don't forget!
        *endptr = str;
        return;
    }

    if (value < 0) {
        value = -value; *str++ = '-';
    }
    else *str++ = '+';

    
    int EXPONENT = (int)floor(log10(value));
    // TODO handle special cases where pow() cannot be computed because the exponent is too large
    if ((digits - 1) - EXPONENT > 308) goto give0;
    if ((digits - 1) - EXPONENT < -308) goto give0;

    // digits are obtained by making value an integer with digits many digits
    long long MANTISSA = value*pow(10., (digits-1)-EXPONENT); // is at 0 based position 'EXPONENT', want to get it to 0-based position digits-1

    str += digits;
    ulltoa_backwards_dotted(digits, MANTISSA, str);
    str++;

    // +-EXPONENT
    *str++ = 'e';
    str += 3;
    itoa_backwards_signed(3, EXPONENT, str);
    str++;

    *endptr = str;
}

void dtostr_fast(
    double value,
    int digits,
    char *str,
    char** endptr
    ) {
    dftostr_fast<double>(value, digits, str, endptr);
}

void ftostr_fast(
    float value,
    int digits,
    char *str,
    char** endptr
    ) {
    dftostr_fast<float>(value, digits, str, endptr);
}

#include <assert.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
void demo1() {
    char* e;
    double x = strtod_fast("3.141", &e);
    printf("%a", 0.0);

    char b[100];

    itoa_backwards_signed(3, -200, b + 3);
    itoa_backwards_signed(3, 400, b + 3);
    itoa_backwards_signed(3, 9, b + 3);


    ulltoa_backwards_dotted(3, 200, b + 3);
    ulltoa_backwards_dotted(3, 177, b + 3);
    ulltoa_backwards_dotted(3, 999, b + 3);


    e = 0; dtostr_fast(1.234e-250, 17, b, &e); assert(e == b + 24); // exactly 24 characters
    e = 0; dtostr_fast(0.0, 17, b, &e); assert(e == b + 24);
    e = 0; dtostr_fast(1.0, 17, b, &e); assert(e == b + 24);
    e = 0; dtostr_fast(M_E, 17, b, &e); assert(e == b + 24);
    e = 0; dtostr_fast(M_PI, 17, b, &e); assert(e == b + 24);
    e = 0; dtostr_fast(1.234e+250, 17, b, &e); assert(e == b + 24);
}