#ifndef clox_value_h
#define clox_value_h

#include "common.h"
#include <string.h>
#include <stdio.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjComplex ObjComplex;

#ifdef NAN_BOXING

typedef uint64_t Value;

#define SIGN_BIT 				((uint64_t)0x8000000000000000)
#define QNAN 						((uint64_t)0x7ffc000000000000)

#define TAG_NIL 	1
#define TAG_FALSE 2
#define TAG_TRUE	3	
#define TAG_CHAR	0

#define IS_CHAR(value)		((value) & 255)
#define IS_BOOL(value)		(((value)|1)==TRUE_VAL)
#define IS_NIL(value)			((value)==NIL_VAL)
#define IS_NUMBER(value)	(((value)& QNAN)!=QNAN)
#define IS_OBJ(value) \
	(((value) & (QNAN | SIGN_BIT))==(QNAN | SIGN_BIT))

#define AS_CHAR(value)		((char)(value))
#define AS_NUMBER(value)	valueToNum(value)
#define AS_BOOL(value)		((value)==TRUE_VAL)
#define AS_OBJ(value) \
	((Obj*)(uintptr_t)((value) & ~(SIGN_BIT |QNAN)))

#define BOOL_VAL(b)				((b)? TRUE_VAL:FALSE_VAL)
#define FALSE_VAL					((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL					((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL						((Value)(uint64_t)(QNAN | TAG_NIL))
#define CHAR_VAL(value)		((Value)(uint64_t)((value & 255) | QNAN))
#define NUMBER_VAL(num) 	numToValue(num)
#define OBJ_VAL(obj)	\
	(Value)(SIGN_BIT | QNAN |(uint64_t)(uintptr_t)(obj))

static inline double valueToNum(Value value){
	double num;
	memcpy(&num,&value,sizeof(value));
	return num;
}

static inline Value numToValue(double num){
	Value value;
	memcpy(&value,&num,sizeof(double));
	return value;
}

#else

typedef enum{
	VAL_BOOL,
	VAL_NIL,
	VAL_NUMBER,
	VAL_CHAR,
	VAL_OBJ
}ValueType;

typedef struct{
	ValueType type;
	union{
		bool boolean;
		char character;
		double number;
		Obj* obj;
	}as;
}Value;

#define IS_BOOL(value)		((value).type==VAL_BOOL)
#define IS_NIL(value)			((value).type==VAL_NIL)
#define IS_NUMBER(value)	((value).type==VAL_NUMBER)
#define IS_CHAR(value)		((value).type==VAL_CHAR)
#define IS_OBJ(value)			((value).type==VAL_OBJ)

#define AS_OBJ(value)			((value).as.obj)
#define AS_BOOL(value)		((value).as.boolean)
#define AS_NUMBER(value)	((value).as.number)
#define AS_CHAR(value)		((value).as.character)

#define BOOL_VAL(value) 	((Value){VAL_BOOL,{.boolean=value}})
#define NIL_VAL						((Value){VAL_NIL,{.number=0}})
#define NUMBER_VAL(value)	((Value){VAL_NUMBER,{.number=value}})
#define CHAR_VAL(value)		((Value){VAL_CHAR,{.character=value}})
#define OBJ_VAL(object)		((Value){VAL_OBJ,{.obj=(Obj*)object}})

#endif

typedef struct{
	int capacity;
	int count;
	Value* values;
}ValueArray;

char* getValueType(Value);
bool valuesEqual(Value a,Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array,Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value,FILE*);

#endif

