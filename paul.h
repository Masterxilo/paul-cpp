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

void demo1();