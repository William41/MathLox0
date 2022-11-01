#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../include/value.h"
#include "../include/object.h"
#include "../include/matrix_alg.h"

#define ERROR_ARG_COUNT(argc,msg) \
	if(argc!=argCount)	Error(msg);

#define ERROR_NOT_MATRIX(value,msg) \
	if(!IS_MATRIX(value)) Error(msg);

static void Error(const char* format, ...){
	va_list args;
	va_start(args,format);
	fprintf(stderr,"MathError:");
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
}

struct GivensState{
};
