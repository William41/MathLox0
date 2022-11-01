#ifndef clox_native_complex_math
#define clox_native_complex_math

#include<math.h>
#include<stdarg.h>
#include<stdio.h>
#include "value.h"
#include "object.h"

#define PI 3.14158265358979323846
#define E  2.718

/*#define ERROR_ARG_COUNT(argc,msg) \
	if(argc!=argCount)	Error(msg);

#define ERROR_NOT_COMPLEX(value,msg) \
	if(!IS_COMPLEX(value)) Error(msg);

static void Error(const char* format, ...){
	va_list args;
	va_start(args,format);
	fprintf(stderr,"MathError:");
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
}*/

Value lox_native_cabs(int,Value*);
Value lox_native_carg(int,Value*);
Value lox_native_crand(int,Value*);
Value lox_native_csin(int,Value*);
Value lox_native_ccos(int,Value*);
Value lox_native_ctan(int,Value*);
Value lox_native_ccsc(int,Value*);
Value lox_native_csec(int,Value*);
Value lox_native_ccot(int,Value*);
Value lox_native_casin(int,Value*);
Value lox_native_cacos(int,Value*);
Value lox_native_catan(int,Value*);
Value lox_native_cacsc(int,Value*);
Value lox_native_casec(int,Value*);
Value lox_native_cacot(int,Value*);
Value lox_native_csinh(int,Value*);
Value lox_native_ccosh(int,Value*);
Value lox_native_ctanh(int,Value*);
Value lox_native_ccsch(int,Value*);
Value lox_native_csech(int,Value*);
Value lox_native_ccoth(int,Value*);
Value lox_native_casinh(int,Value*);
Value lox_native_cacosh(int,Value*);
Value lox_native_catanh(int,Value*);
Value lox_native_cacsch(int,Value*);
Value lox_native_casech(int,Value*);
Value lox_native_cacoth(int,Value*);







#endif

