#include<stdio.h>
#include<string.h>
#include<math.h>
#include "../include/memory.h"
#include "../include/value.h"
#include "../include/object.h"


extern int precision;
extern bool expo_notation;
void initValueArray(ValueArray* array){
  array->values=NULL;
  array->capacity=0;
  array->count=0;
 }
void writeValueArray(ValueArray* array,Value value){
  if(array->capacity < array->count+1){
    int oldCapacity=array->capacity;
    array->capacity=GROW_CAPACITY(oldCapacity);
    array->values=GROW_ARRAY(Value,array->values,oldCapacity,array->capacity);
  }
  array->values[array->count]=value;
  array->count++;
 }
void freeValueArray(ValueArray* array){
  FREE_ARRAY(Value,array->values,array->capacity);
  initValueArray(array);
 }

char* getValueType(Value value){
	/*switch(value.type){
		case VAL_BOOL :return "Boolean";
		case VAL_NUMBER:return "Number";
		case VAL_CHAR: return "Character";
		case VAL_NIL:return "nil";
		case VAL_OBJ:return getType(value);
	}*/
	return "";
}
 
void printValue(Value value,FILE* file_ptr){
#ifdef NAN_BOXING
	if(IS_BOOL(value)) fprintf(file_ptr,AS_BOOL(value)?"true":"false");
	else if(IS_NIL(value)) fprintf(file_ptr,"nil");
	else if(IS_NUMBER(value)) {
		if(expo_notation){
			fprintf(file_ptr,"%.*e",precision,AS_NUMBER(value));
		}else{
			fprintf(file_ptr,"%.*f",precision,AS_NUMBER(value));
		}
	}else if(IS_OBJ(value)) printObject(value,file_ptr);
	else if(IS_CHAR(value)) fprintf(file_ptr,"%c",AS_CHAR(value));
#else		
	switch(value.type){
		case VAL_BOOL:
			fprintf(file_ptr,AS_BOOL(value)?"true":"false");
			break;	
		case VAL_NIL:
			fprintf(file_ptr,"nil");break;
		case VAL_NUMBER:
			if(expo_notation){
				fprintf(file_ptr,"%.*e",precision,AS_NUMBER(value));
			}else{
 				fprintf(file_ptr,"%.*f",precision,AS_NUMBER(value));
 			}
 			break;
 		case VAL_CHAR:
 			//#ifndef NAN_BOXING
 				fprintf(file_ptr,"%c",AS_CHAR(value));
 			//#endif
 			break;
 		case VAL_OBJ:{
 			printObject(value,file_ptr);
 			break;
 			}
 		}
#endif
}

bool valuesEqual(Value a,Value b){
#ifdef NAN_BOXING
	if(IS_NUMBER(a) && IS_NUMBER(b)){
		return AS_NUMBER(a)==AS_NUMBER(b);
	}
	return a==b;
#else
	if(a.type != b.type) return false;
	switch(a.type){
		case VAL_BOOL:		return AS_BOOL(a)==AS_BOOL(b);
		case VAL_NIL:			return true;
		case VAL_NUMBER:	return AS_NUMBER(a)==AS_NUMBER(b);
		case VAL_CHAR:	
			//#ifndef NAN_BOXING
				return AS_CHAR(a)==AS_CHAR(b);
			//#endif
		case VAL_OBJ:			return AS_OBJ(a)==AS_OBJ(b);
		default:					return false;
	}
#endif
}
