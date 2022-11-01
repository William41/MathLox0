#include <stdio.h>
#include <stdarg.h>
#include<stdlib.h>

#include "../include/init_obj.h"
#include "../include/vm.h"

#define SET_ERROR_ARGS(argc,msg)\
	if(argCount!=argc) Error(msg);

static void Error(const char* format, ...){
	va_list args;
	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
	return;
}
Value asNumNative(int argCount,Value* args){
	double val=0;
	//#ifndef NAN_BOXING
	if(IS_CHAR(args[0])){
		val=(double)(AS_CHAR(args[0])-48);
	}
	//#endif
	if(IS_NUMBER(args[0])){
		val=AS_NUMBER(args[0]);
	}else{
		if(IS_OBJ(args[0])){
		if(IS_STRING(args[0])){
			val=strtod(AS_CSTRING(args[0]),NULL);
			}else if(IS_NIL(args[0])){
			}else{
				printf("INVALID TYPE...");
			}
		}
		}
	return NUMBER_VAL(val);
}

Value asHexNative(int argCount,Value* args){
	printf("0x%lx\n",(uint64_t)AS_NUMBER(args[0]));
	return args[0];
}

Value newComplexNative(int argCount,Value* args){
	if(argCount!=2){
		Error("Required 2 arguments for complex()");
	}
	else{
		double real=AS_NUMBER(args[0]);
		double imag=AS_NUMBER(args[1]);
		return OBJ_VAL(allocateComplex(real,imag));
	}
}

Value newListNative(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to list().");
	ObjList* list=newList();
	push(OBJ_VAL(list));
	int capacity=AS_NUMBER(args[0]);
	for(int i=0;i<capacity;i++){
		appendToList(list,NUMBER_VAL(0));
	}
	pop();
	return OBJ_VAL(list);
}
Value newMatrixNative(int argCount,Value* args){
	SET_ERROR_ARGS(2,"Expected 2 arguments for matrix().");
	int rows=AS_NUMBER(args[0]);
	int cols=AS_NUMBER(args[1]);
	return OBJ_VAL(initRealMatrix(rows,cols));
}
