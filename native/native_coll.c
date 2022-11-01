#include <stdio.h>
#include <stdarg.h>
#include "../include/collect_ops.h"
#include "../include/value.h"
#include "../include/object.h"


static void Error(const char* format, ...){
	va_list args;
	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
	return;
}

Value appendNative(int argCount,Value* args){
	if(argCount!=2 || !IS_LIST(args[0])){
		Error("Invalid arguments to append()");
		return BOOL_VAL(false);
	}
	ObjList* list=AS_LIST(args[0]);
	Value item=args[1];
	appendToList(list,item);
	return NIL_VAL;
}

Value deleteNative(int argCount,Value* args){
	if(argCount!=2|| !IS_LIST(args[0])){
		Error("Invalid arguments to delete()");
	}
	ObjList* list = AS_LIST(args[0]);
  int index = AS_NUMBER(args[1]);
  if (!isValidListIndex(list, index)) {
     Error("Index out of range");
  }
  deleteFromList(list, index);
 	return NIL_VAL;
}


Value getListLength(int argCount,Value* args){
	if(argCount!=1){
		Error("Wrong number of arguments passed to len()");
	}
	if(IS_OBJ(args[0])){
		switch(OBJ_TYPE(args[0])){
			case OBJ_LIST:
				return NUMBER_VAL(AS_LIST(args[0])->count);
			case OBJ_STRING:
				return NUMBER_VAL(AS_STRING(args[0])->length);
		}
	}else{
		Error("Bad argument type to len()");
	}
}

