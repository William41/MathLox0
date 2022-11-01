#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../include/memory.h"
#include "../include/value.h"
#include "../include/object.h"
#include "../include/table.h"
#include "../include/vm.h"
#include "../include/complex.h"

#define ALLOCATE_OBJ(type,objectType) \
	(type*)allocateObject(sizeof(type),objectType)

extern int precision;
static Obj* allocateObject(size_t size,ObjType type){
	Obj* object=(Obj*)reallocate(NULL,0,size);
	object->type=type;
	object->isMarked=false;
	//insertion at the head of the linked list...
	object->next=vm.objects;
	vm.objects=object;
	#ifdef DEBUG_LOG_GC
		printf("%p allocated %zu bytes for %d\n",(void*)object,size,type);
	#endif
	return object;
}

char* getType(Value value){
	switch(OBJ_TYPE(value)){
		case OBJ_COMPLEX:return "Complex";
		case OBJ_STRING:return "String";
		case OBJ_MATRIX:return "Matrix";
		case OBJ_LIST:return "List";
	}
}

ObjFunction* newFunction(){
	ObjFunction* function=ALLOCATE_OBJ(ObjFunction,OBJ_FUNCTION);
	function->arity=0;
	function->name=NULL;
	function->upvalueCount=0;
	initChunk(&function->chunk);
	return function;
}

ObjNative* newNative(NativeFn function){
	ObjNative* native=ALLOCATE_OBJ(ObjNative,OBJ_NATIVE);
	native->function=function;
	return native;
}

ObjClosure* newClosure(ObjFunction* function){
	ObjUpvalue** upvalues=ALLOCATE(ObjUpvalue*,function->upvalueCount);
	for(int i=0;i<function->upvalueCount;i++){
		upvalues[i]=NULL;
	}
	
	ObjClosure* closure=ALLOCATE_OBJ(ObjClosure,OBJ_CLOSURE);
	closure->function=function;
	closure->upvalues=upvalues;
	closure->upvalueCount=function->upvalueCount;
	return closure;
}

ObjClass* newClass(ObjString* name){
	ObjClass* klass=ALLOCATE_OBJ(ObjClass,OBJ_CLASS);
	klass->name=name;
	klass->parent=NULL;
	initTable(&klass->methods);
	initTable(&klass->op_overloads);
	return klass;
}

ObjInstance* newInstance(ObjClass* klass){
	ObjInstance* instance=ALLOCATE_OBJ(ObjInstance,OBJ_INSTANCE);
	instance->klass=klass;
	initTable(&instance->fields);
	return instance;
}

ObjBoundMethod* newBoundMethod(Value receiver,ObjClosure* method){
	ObjBoundMethod* bound=ALLOCATE_OBJ(ObjBoundMethod,OBJ_BOUND_METHOD);
	bound->receiver=receiver;
	bound->method=method;
	return bound;
}

ObjUpvalue* newUpvalue(Value* slot){
	ObjUpvalue* upvalue=ALLOCATE_OBJ(ObjUpvalue,OBJ_UPVALUE);
	upvalue->closed=NIL_VAL;
	upvalue->location=slot;
	upvalue->next=NULL;
	return upvalue;
}

static ObjString* allocateString(char* chars,int length,uint32_t hash){
	ObjString* string=ALLOCATE_OBJ(ObjString,OBJ_STRING);
	string->length=length;
	string->chars=chars;
	string->hash=hash;
	push(OBJ_VAL(string));
	tableSet(&vm.strings,string,NIL_VAL);
	pop();
	return string;
}

ObjComplex* allocateComplex(double real,double imag){
	ObjComplex* cmplx=ALLOCATE_OBJ(ObjComplex,OBJ_COMPLEX);
	cmplx->REAL=real;
	cmplx->IMAGINARY=imag;
	return cmplx;
}

static uint32_t hashString(const char* key,int length){
	//fowler-noll-vu (fnv-1a ) algorithm code
	uint32_t hash=2166136261u;//hash basis
	for(int i=0;i<length;i++){
		hash^=(uint8_t)key[i];
		hash*=16777619;
	}
	return hash;
}

ObjString* takeString(char* chars,int length){
	uint32_t hash=hashString(chars,length);
	ObjString* interned=tableFindString(&vm.strings,chars,length,hash);
	if(interned!=NULL){
		FREE_ARRAY(char,chars,length+1);
		return interned;
	}
	return allocateString(chars,length,hash);
}

ObjString* copyString(const char* chars,int length){
	uint32_t hash=hashString(chars,length);
	ObjString* interned=tableFindString(&vm.strings,chars,length,hash);
	if(interned!=NULL) return interned;
	char* heapChars=ALLOCATE(char,length+1);
	memcpy(heapChars,chars,length);
	heapChars[length]='\0';
	return allocateString(heapChars,length,hash);
}

ObjComplex* parseImaginary(const char* chars){
	double im=0;
	int str_len=strlen(chars)-1;//exclude the i part
	char* imag_str=ALLOCATE(char,str_len);
	memcpy(imag_str,chars,str_len-1);
	im=strtod(imag_str,NULL);
	FREE(char,imag_str);
	return allocateComplex(0,im);
}

Value indexFromString(ObjString* string,int index){
	//#ifndef NAN_BOXING
		return CHAR_VAL(string->chars[index]);
	//#endif
}

static void printFunction(ObjFunction* function,FILE* file_ptr){
	if(function->name==NULL){
		fprintf(file_ptr,"<script>");
		return;
	}
	fprintf(file_ptr,"<fn %s>",function->name->chars);
}

ObjList* newList(){
	ObjList* list=ALLOCATE_OBJ(ObjList,OBJ_LIST);
	list->count=0;
	list->capacity=0;
	list->items=NULL;
	return list;
}

ObjRealMatrix* initRealMatrix(int rows,int columns){
	double** values=ALLOCATE(double*,rows);
	for(int i=0;i<rows;i++)
		values[i]=ALLOCATE(double,columns);
	ObjRealMatrix* matrix=ALLOCATE_OBJ(ObjRealMatrix,OBJ_MATRIX);
	matrix->rows=rows;
	matrix->columns=columns;
	matrix->data=values;
	matrix->factor1=NULL;
	matrix->factor2=NULL;
	matrix->perm_vector=NULL;
	matrix->determinant=1;
	return matrix;
}

Value indexFromMatrix(ObjRealMatrix* matrix,int row,int col){
	return NUMBER_VAL(matrix->data[row][col]);
}

void storeToMatrix(ObjRealMatrix* mat,int row,int col,double val){
	mat->data[row][col]=val;
}

Value lox_native_mat_rows(int argCount,Value* args){
	return NUMBER_VAL(AS_REAL_MATRIX(args[0])->rows);
}

Value lox_native_mat_cols(int argCount,Value* args){
	return NUMBER_VAL(AS_REAL_MATRIX(args[0])->columns);
}

ObjRealMatrix* getRow(ObjRealMatrix* matrix,int row){
	int columns=matrix->columns;
	ObjRealMatrix* row_vec=initRealMatrix(1,columns);
	/*for(int i=0;i<columns;i++)
		row_vec->data[0][i]=matrix->data[row][i];*/
	memcpy(row_vec->data[0],matrix->data[row],columns*sizeof(double));
	return row_vec;
}

ObjRealMatrix* getColumn(ObjRealMatrix* matrix,int column){
	int rows=matrix->rows;
	ObjRealMatrix* col_vec=initRealMatrix(rows,1);
	for(int i=0;i<rows;i++)
		col_vec->data[i][0]=matrix->data[i][column];
	return col_vec;
}

void setRow(ObjRealMatrix* matrix,double* row,int index){
	int cols=matrix->columns;
	/*for(int i=0;i<cols;i++)
		matrix->data[index][i]=row[i];*/
	memcpy(matrix->data[index],row,cols*sizeof(double));
}

void setCol(ObjRealMatrix* matrix,double* col,int index){
	int rows=matrix->rows;
	
}




void appendToList(ObjList* list,Value value){
	//Grow the array if necessary
	if(list->capacity<list->count+1){
		int oldCapacity=list->capacity;
		list->capacity=GROW_CAPACITY(oldCapacity);
		list->items=GROW_ARRAY(Value,list->items,oldCapacity,list->capacity);
	}
	list->items[list->count]=value;
	list->count++;
	return;
}

void storeToList(ObjList* list,int index,Value value){
	list->items[index]=value;
}

Value indexFromList(ObjList* list,int index){
	return list->items[index];
}

ObjList* sliceFromList(ObjList* list,int start,int end,int step){
	ObjList* slice=newList();
	for(int i=start;i<end;i+=step){
		appendToList(slice,indexFromList(list,i));
	}
	return slice;
}

void deleteFromList(ObjList* list,int index){
	for(int i=index;i<list->count-1;i++){
		//move all items forward
		list->items[i]=list->items[i+1];
	}
	//set the last item to nil
	list->items[list->count-1]=NIL_VAL;
	list->count--;
	int count=list->count;
	int curr_cap=list->capacity;
	if(count==curr_cap>>1){
		list->capacity=count;
	}
}

bool isValidListIndex(ObjList* list,int index){
	if(index<0 || index>list->count-1) return false;
	else
		return true;
}

bool isValidStringIndex(ObjString* string,int index){
	return !(index<0 || index>string->length-1);
}

ObjMap* newMap(){
	ObjMap* map=ALLOCATE_OBJ(ObjMap,OBJ_MAP);
	initTable(&map->values);
	return map;
}

static void printMatrix(ObjRealMatrix* matrix,FILE* file_ptr){
	#ifdef DEBUG_LOG_GC
		printf("[matrix obj]");
			for(int i=0;i<matrix->rows;i++){
				for(int j=0;j<matrix->columns;j++){
					printf("%f,",matrix->data[i*matrix->columns+j]);
			}
			printf(";");
		}
		printf("]");
	#endif
	int rows=matrix->rows;
	int cols=matrix->columns;
	int max_lens[cols];
	size_t len=0;
	int padding=0;
	for(int i=0;i<cols;i++) max_lens[i]=0;
	for(int i=0;i<rows;i++){
		for(int j=0;j<cols;j++){
			len=snprintf(NULL,0,"%.*f",precision,matrix->data[i][j]);
			max_lens[j]=(max_lens[j]>len)?max_lens[j]:len;
		}
	}
	const char* space=" ";
	for(int i=0;i<rows;i++){
		for(int j=0;j<cols;j++){
			padding=0;
			len=snprintf(NULL,0,"%.*f",precision,matrix->data[i][j]);
			padding=max_lens[j]-len;
		if(padding>0) padding+=1;
			fprintf(file_ptr,"%*c%.*f%4s",padding,' ',precision,matrix->data[i][j],space);
		}
	printf("\n");
	}
}

static void printList(ObjList* list,FILE* file_ptr){
	int len=list->count;
	fprintf(file_ptr,"{");
	for(int i=0;i<len;i++){
		printValue(list->items[i],file_ptr);
		if(i==len-1) continue;
		else{
			fprintf(file_ptr,",");
		}
	}
	fprintf(file_ptr,"}");
}

static void printComplex(ObjComplex* cvalue,FILE* file_ptr){
	fprintf(file_ptr,"%.*f%c%.*fi",precision,cvalue->REAL,SET_IMAG_SIGN(cvalue),precision,cvalue->IMAGINARY);
}

void printObject(Value value,FILE* file_ptr){
	switch(OBJ_TYPE(value))
	{
		case OBJ_STRING:
			fprintf(file_ptr,"%s",AS_CSTRING(value));
			break;
		case OBJ_COMPLEX:
			printComplex(AS_COMPLEX(value),file_ptr);
			break;
		case OBJ_FUNCTION:
			printFunction(AS_FUNCTION(value),file_ptr);
			break;
		case OBJ_NATIVE:
			fprintf(file_ptr,"<native fn>");
			break;
		case OBJ_CLOSURE:
			printFunction(AS_CLOSURE(value)->function,file_ptr);
			break;
		case OBJ_LIST:
			printList(AS_LIST(value),file_ptr);
			break;
	  case OBJ_MATRIX:
			printMatrix(AS_REAL_MATRIX(value),file_ptr);
			break;
		case OBJ_UPVALUE:
			fprintf(file_ptr,"upvalue");
			break;
		case OBJ_CLASS:
			fprintf(file_ptr,"%s",AS_CLASS(value)->name->chars);
			break;
		case OBJ_INSTANCE:
			fprintf(file_ptr,"%s instance",AS_INSTANCE(value)->klass->name->chars);
			break;
		case OBJ_BOUND_METHOD:
			printFunction(AS_BOUND_METHOD(value)->method->function,file_ptr);
			break;
	}
}

Value lox_native_prod_list_scalar(int argCount,Value* args){
	ObjList* list=AS_LIST(args[1]);
	double n=AS_NUMBER(args[0]);
	ObjList* mul=newList();
	push(OBJ_VAL(mul));
	for(int i=0;i<list->count;i++)
		appendToList(mul,NUMBER_VAL(n*AS_NUMBER(list->items[i])));
	pop();
	return OBJ_VAL(mul);
}

Value lox_native_convolute(int argCount,Value* args){
	ObjList* list1=AS_LIST(args[0]);
	ObjList* list2=AS_LIST(args[1]);
	int n=list1->count;
	int m=list2->count;
	int con_size=n+m-1;
	ObjList* conv=newList();
	push(OBJ_VAL(conv));
	for(int i=0;i<con_size;i++)
		appendToList(conv,NUMBER_VAL(0));
	for(int i=0;i<n;i++){
		double g=AS_NUMBER(list1->items[i]);
		for(int j=0;j<m;j++){
			conv->items[i+j]=NUMBER_VAL(AS_NUMBER(conv->items[i+j])+g*AS_NUMBER(list2->items[j]));
		}
	}
	pop();
	return OBJ_VAL(conv);
}

#define ERROR_ARGS(value,message) \
	if(argCount!=value) Error(message);

static void Error(const char* format, ...){
	va_list args;
	fprintf(stderr,"Argument Error:");
	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
	return;
}

Value lox_native_is_complex(int argCount,Value* args){
	ERROR_ARGS(1,"Wrong number of arguments to is_complex()");
	return BOOL_VAL(IS_OBJ(args[0]) && AS_OBJ(args[0])->type==OBJ_COMPLEX);
}

Value lox_native_is_function(int argCount,Value* args){
	ERROR_ARGS(1,"Wrong number of arguments to is_function()");
	return BOOL_VAL(IS_OBJ(args[0]) && (AS_OBJ(args[0])->type==OBJ_CLOSURE||AS_OBJ(args[0])->type==OBJ_FUNCTION||AS_OBJ(args[0])->type==OBJ_NATIVE));
}

Value lox_native_is_list(int argCount,Value* args){
	ERROR_ARGS(1,"Wrong number of arguments to is_list()");
	return BOOL_VAL(IS_OBJ(args[0]) && AS_OBJ(args[0])->type==OBJ_LIST);
}

Value lox_native_is_string(int argCount,Value* args){
	ERROR_ARGS(1,"Wrong number of arguments to is_string()");
	return BOOL_VAL(IS_OBJ(args[0]) && AS_OBJ(args[0])->type==OBJ_STRING);
}

Value lox_native_is_matrix(int argCount,Value* args){
	ERROR_ARGS(1,"Wrong number of arguments to is_matrix()");
	return BOOL_VAL(IS_OBJ(args[0]) && AS_OBJ(args[0])->type==OBJ_MATRIX);
}	

Value lox_native_is_number(int argCount,Value* args){
	ERROR_ARGS(1,"Wrong number of arguments to is_matrix()");
	return BOOL_VAL(!IS_OBJ(args[0]) && IS_NUMBER(args[0]));
}

Value lox_native_is_class_type(int argCount,Value* args){
	ERROR_ARGS(2,"Wrong number of arguments to is_type()");
	return BOOL_VAL(IS_OBJ(args[0]) && AS_INSTANCE(args[0])->klass==AS_CLASS(args[1]));
}
