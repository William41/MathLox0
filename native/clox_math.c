#include<stdarg.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>

#include "../include/native_math.h"
#include "../include/object.h"
#include "../include/value.h"
#include "../include/complex.h"

extern int precision;
extern bool expo_notation;

#define PI_180 0.01745329251994329576924
#define r180_PI 57.2957795130823208768
#define PI_200 0.01570796326794896619231
#define r200_PI 63.66197723675813430755

#define SET_ERROR_ARGS(argc ,message)\
	if(argCount!=argc) Error(message);
	
#define NOT_NUMBER_TYPE(value,message)\
	if(!IS_NUMBER(value)) Error(message);


static void Error(const char* format, ...){
	va_list args;
	va_start(args,format);
	fprintf(stderr,"MathError:");
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
}



Value lox_native_max(int argCount,Value* args){
	SET_ERROR_ARGS(2,"Wrong number of arguments to max()");
	double x=AS_NUMBER(args[0]);
	double y=AS_NUMBER(args[1]);
	return NUMBER_VAL(x>y?x:y);
}

Value lox_native_min(int argCount,Value* args){
	SET_ERROR_ARGS(2,"Wrong number of arguments to min()");
	double x=AS_NUMBER(args[0]);
	double y=AS_NUMBER(args[1]);
	return NUMBER_VAL(x<y?x:y);
}

Value lox_native_rand(int argCount,Value* args){
	SET_ERROR_ARGS(0,"Wrong number of arguments to rand()");
	double x= (double)rand()/2147483647.0;
	return NUMBER_VAL(x);
}

Value lox_native_precision(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments for precision()");
	precision=(int)AS_NUMBER(args[0]);
	return NUMBER_VAL(precision) ;
}

Value lox_native_exp_not(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments for sci()");
	expo_notation=AS_BOOL(args[0]);
	return BOOL_VAL(expo_notation);
}

Value lox_native_int_div(int argCount,Value* args){
	SET_ERROR_ARGS(2,"Wrong number of arguments to int_div()");
	int x=(int)AS_NUMBER(args[0]);
	int y=(int)AS_NUMBER(args[1]);
	return NUMBER_VAL(x/y);
}

Value lox_native_isnan(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments for is_nan()");
	return BOOL_VAL(isnan(AS_NUMBER(args[0])));
}

Value lox_native_isinf(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments for is_inf()");
	return BOOL_VAL(AS_NUMBER(args[0])==INFINITY);
}

Value lox_native_abs(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Expected single argument to abs()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to abs()");
	return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

Value lox_native_ceil(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Expected single argument to ceil()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to ceil()");
	return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

Value lox_native_floor(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Expected single argument to floor()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to floor()");
	return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

Value lox_native_sin(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to sin()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to sin()");
	return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

Value lox_native_cos(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to cos()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to cos()");
	return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

Value lox_native_tan(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to tan()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to tan()");
	return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

Value lox_native_cosec(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to cosec()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to cosec()");
	return NUMBER_VAL(1.0/sin(AS_NUMBER(args[0])));
}

Value lox_native_sec(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to sec()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to sec()");
	return NUMBER_VAL(1.0/cos(AS_NUMBER(args[0])));
}

Value lox_native_cot(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to cot()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to cot()");
	return NUMBER_VAL(1.0/tan(AS_NUMBER(args[0])));
}

Value lox_native_arcsin(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to asin()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to asin()");
	return NUMBER_VAL(asin(AS_NUMBER(args[0])));
}

Value lox_native_arccos(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to acos()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to acos()");
	return NUMBER_VAL(acos(AS_NUMBER(args[0])));
}

Value lox_native_arctan(int argCount,Value* args){
	if(argCount==1){
		NOT_NUMBER_TYPE(args[0],"Invalid argument type to atan()");
		return NUMBER_VAL(atan(AS_NUMBER(args[0])));
	}else if(argCount==2){
		return NUMBER_VAL(atan2(AS_NUMBER(args[0]),AS_NUMBER(args[1])));
	}else{
		Error("Wrong number of arguments to atan()");
	}
}

Value lox_native_arccsc(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to acsc()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to acsc()");
	return NUMBER_VAL(asin(1.0/AS_NUMBER(args[0])));
}

Value lox_native_arcsec(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to asec()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to asec()");
	return NUMBER_VAL(acos(1.0/AS_NUMBER(args[0])));
}

Value lox_native_arccot(int argCount,Value* args){
	if(argCount==1){
		NOT_NUMBER_TYPE(args[0],"Invalid argument type to acot()");
		return NUMBER_VAL(atan(1.0/AS_NUMBER(args[0])));
	}else if(argCount==2){
		return NUMBER_VAL(atan2(AS_NUMBER(args[1]),AS_NUMBER(args[0])));
	}else{
		Error("Wrong number of arguments to acot()");
	}
}

Value lox_native_sinh(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to sinh()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to sinh()");
	return NUMBER_VAL(sinh(AS_NUMBER(args[0])));
}

Value lox_native_cosh(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to cosh()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to cosh()");
	return NUMBER_VAL(cosh(AS_NUMBER(args[0])));
}

Value lox_native_tanh(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to tanh()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to tanh()");
	return NUMBER_VAL(tanh(AS_NUMBER(args[0])));
}

Value lox_native_cosech(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to cosech()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to cosech()");
	return NUMBER_VAL(1.0/sinh(AS_NUMBER(args[0])));
}

Value lox_native_sech(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to sech()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to sech()");
	return NUMBER_VAL(1.0/cosh(AS_NUMBER(args[0])));
}

Value lox_native_coth(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to coth()");
	NOT_NUMBER_TYPE(args[0],"Invalid argument type to coth()");
	return NUMBER_VAL(1.0/tanh(AS_NUMBER(args[0])));
}

Value lox_native_sqrt(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to sqrt()");
	double arg=AS_NUMBER(args[0]);
	if(arg>0) return NUMBER_VAL(sqrt(arg));
}
