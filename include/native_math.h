#ifndef clox_native_math
#define clox_native_math

#include<stdio.h>

#include "constants.h"
#include "object.h"
#include "value.h"
#include "math.h"

Value lox_native_rand(int,Value*);
Value lox_native_precision(int,Value*);
Value lox_native_exp_not(int,Value*);
Value lox_native_isnan(int,Value*);
Value lox_native_isinf(int,Value*);
Value lox_native_abs(int,Value*);
Value lox_native_min(int,Value*);
Value lox_native_max(int,Value*);
Value lox_native_ceil(int,Value*);
Value lox_native_floor(int,Value*);
Value lox_native_int_div(int,Value*);

Value lox_native_sin(int,Value*);
Value lox_native_cos(int,Value*);
Value lox_native_tan(int,Value*);
Value lox_native_cosec(int,Value*);
Value lox_native_sec(int,Value*);
Value lox_native_cot(int,Value*);

Value lox_native_arcsin(int,Value*);
Value lox_native_arccos(int,Value*);
Value lox_native_arctan(int,Value*);
Value lox_native_arccsc(int,Value*);
Value lox_native_arcsec(int,Value*);
Value lox_native_arccot(int,Value*);

Value lox_native_sinh(int,Value*);
Value lox_native_cosh(int,Value*);
Value lox_native_tanh(int,Value*);
Value lox_native_cosech(int,Value*);
Value lox_native_sech(int,Value*);
Value lox_native_coth(int,Value*);

Value lox_native_asinh(int,Value*);
Value lox_native_acosh(int,Value*);
Value lox_native_atanh(int,Value*);
Value lox_native_acosech(int,Value*);
Value lox_native_asech(int,Value*);
Value lox_native_acoth(int,Value*);

Value lox_native_pow(int,Value*,double);
Value lox_native_exp(int,Value*);
Value logNative(int,Value*,double base);
Value lox_native_ln(int,Value*);
Value lox_native_sqrt(int,Value*);
Value cubeNative(int,Value*);
Value cbrtNative(int,Value*);


#endif
