#include<stdarg.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include "../include/common.h"
#include "../include/compiler.h"
#include "../include/debug.h"
#include "../include/object.h"
#include "../include/memory.h"
#include "../include/vm.h"
#include "../include/complex.h"
#include "../include/native_io.h"
#include "../include/native_math.h"
#include "../include/collect_ops.h"
#include "../include/init_obj.h"
#include "../include/rmatrix.h"

#define RUNTIME_ERROR(msg)\
	do{\
		frame->ip=ip; \
		runtimeError(msg);\
		return INTERPRET_RUNTIME_ERROR;\
	}while(false)\
	
	
typedef enum{
	NO_ERROR,
	DIMENSION_ERROR,
	INCOMPATIBLE_OPERANDS,
}ERROR_TYPE;

VM vm;
extern int precision;
Value err_operand1;
Value err_operand2;

static void resetStack(){
	vm.stackTop=vm.stack;
	vm.frameCount=0;
	vm.openUpvalues=NULL;
}

static void runtimeError(const char* format, ...){
	va_list args;
	va_start(args,format);
	fprintf(stderr,"Runtime Error:");
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
	for(int i=vm.frameCount-1;i>=0;i--){
		CallFrame* frame=&vm.frames[i];
		ObjFunction* function=frame->closure->function;
		size_t instruction=frame->ip-function->chunk.code-1;
		fprintf(stderr,"[line %d] in",function->chunk.lines[instruction]);
		if(function->name==NULL){
			fprintf(stderr,"script\n");
		}else{
			fprintf(stderr,"%s()\n",function->name->chars);
		}
	}
	resetStack();
}
static void defineNative(const char* name,NativeFn function){
	push(OBJ_VAL(copyString(name,(int)strlen(name) )));
	push(OBJ_VAL(newNative(function)));
	tableSet(&vm.globals,AS_STRING(vm.stack[0]),vm.stack[1]);
	pop();
	pop();
}
static void sysFunctions(){
	defineNative("clock",clockNative);
	defineNative("input_number",scanNumberNative);
	defineNative("include",lox_native_import_module);
	defineNative("format",lox_native_format);
	defineNative("input",scanStringNative);
	defineNative("println",printNewNative);
	defineNative("printf",printNative);
}
static void objFunctions(){
	defineNative("complex",newComplexNative);
	defineNative("new_list",newListNative);
	defineNative("new_matrix",newMatrixNative);
}
static void typeFunctions(){
	defineNative("is_complex",lox_native_is_complex);
	defineNative("is_function",lox_native_is_function);
	defineNative("is_list",lox_native_is_list);
	defineNative("is_string",lox_native_is_string);
	defineNative("is_num",lox_native_is_number);
	defineNative("is_matrix",lox_native_is_matrix);
	defineNative("is_type",lox_native_is_class_type);
}
static void castFunctions(){
	defineNative("num",asNumNative);
	defineNative("hex",asHexNative);
}
static void mathFunctions(){
	defineNative("precision",lox_native_precision);
	defineNative("sci",lox_native_exp_not);
	defineNative("max",lox_native_max);
	defineNative("min",lox_native_min);
	defineNative("int_div",lox_native_int_div);
	defineNative("is_nan",lox_native_isnan);
	defineNative("is_inf",lox_native_isinf);
	defineNative("abs",lox_native_abs);
	defineNative("ceil",lox_native_ceil);
	defineNative("floor",lox_native_floor);
	defineNative("sqrt",lox_native_sqrt);
	defineNative("sin",lox_native_sin);
	defineNative("cos",lox_native_cos);
	defineNative("tan",lox_native_tan);
	defineNative("csc",lox_native_cosec);
	defineNative("sec",lox_native_sec);
	defineNative("cot",lox_native_cot);
	defineNative("asin",lox_native_arcsin);
	defineNative("acos",lox_native_arccos);
	defineNative("atan",lox_native_arctan);
	defineNative("acsc",lox_native_arccsc);
	defineNative("asec",lox_native_arcsec);
	defineNative("acot",lox_native_arccot);
	defineNative("sinh",lox_native_sinh);
	defineNative("cosh",lox_native_cosh);
	defineNative("tanh",lox_native_tanh);
	defineNative("csch",lox_native_cosech);
	defineNative("sech",lox_native_sech);
	defineNative("coth",lox_native_coth);
}
static void randomFunctions(){
	defineNative("rand",lox_native_rand);
	defineNative("rand_matrix",lox_native_randMatrix);
	defineNative("rand_upper",lox_native_randUpper);
	defineNative("rand_lower",lox_native_randLower);
	defineNative("rand_tri",lox_native_randTriDiag);
	defineNative("rand_diag",lox_native_randDiag);
	defineNative("rand_sym",lox_native_randSym);
	defineNative("rand_skew",lox_native_randSkew);
}
static void matrixFunctions(){
	defineNative("mat_id",lox_native_identity);
	defineNative("mat_trn",lox_native_getTranspose);
	defineNative("mat_lup",lox_native_decomposeLUP);
	defineNative("mat_det",lox_native_getDeterminant);
	defineNative("mat_tr",lox_native_getTrace);
	defineNative("mat_id",lox_native_identity);
	defineNative("mat_inv",lox_native_get_inverse);
	defineNative("rows",lox_native_mat_rows);
	defineNative("cols",lox_native_mat_cols);
	defineNative("solveWith",lox_native_solveWith);
}
static void listFunctions(){
	defineNative("append",appendNative);
	defineNative("delete",deleteNative);
	defineNative("scalar_mul",lox_native_prod_list_scalar);
	defineNative("convolute",lox_native_convolute);
	defineNative("len",getListLength);
}
void defineNativeFunctions(){
	sysFunctions();
	objFunctions();
	mathFunctions();
	castFunctions();
	listFunctions();
	matrixFunctions();
	typeFunctions();
	randomFunctions();
}

void initVM()
{
	resetStack();
	vm.objects=NULL;
	vm.grayCount=0;
	vm.grayCapacity=0;
	vm.grayStack=NULL;
	
	vm.bytesAllocated=0;
	vm.nextGC=1024*1024;
	
	initTable(&vm.globals);
	initTable(&vm.strings); 
	
	vm.initString=NULL;
	vm.initString=copyString("init",4);
	defineNativeFunctions();
	precision=5;
	srand(time(NULL));
}
void freeVM(){
	vm.initString=NULL;
	freeTable(&vm.globals);
	freeTable(&vm.strings);
	freeObjects();
}
void push(Value value){
	*vm.stackTop=value;
	vm.stackTop++;
}

Value pop(){
	vm.stackTop--;
	return *vm.stackTop;
}
static Value peek(int distance){
	return vm.stackTop[-1-distance];
}
static bool call(ObjClosure* closure,int argCount){
	if(argCount!=closure->function->arity){
		runtimeError("Expected %d arguments but got %d",closure->function->arity,argCount);
		return false;
	}
	if(vm.frameCount==FRAMES_MAX){
		runtimeError("Stack Overflow.Program terminated");
		return false;
	}
	CallFrame* frame=&vm.frames[vm.frameCount++];
	frame->closure=closure;
	frame->ip=closure->function->chunk.code;
	frame->slots=vm.stackTop-argCount-1;
	return true;
}

bool callValue(Value callee,int argCount){
	if(IS_OBJ(callee)){
		switch(OBJ_TYPE(callee)){
			case OBJ_BOUND_METHOD:{
				ObjBoundMethod* bound=AS_BOUND_METHOD(callee);
				vm.stackTop[-argCount-1]=bound->receiver;
				return call(bound->method,argCount);
			}
			case OBJ_CLASS:{
				ObjClass* klass=AS_CLASS(callee);
				vm.stackTop[-argCount-1]=OBJ_VAL(newInstance(klass));
				Value initializer;
				if(tableGet(&klass->methods,vm.initString,&initializer)){
					return call(AS_CLOSURE(initializer),argCount);
				}else if(argCount!=0){
					runtimeError("Expected 0 arguments but got '%d'.",argCount);
					return false;
				}
				return true;
			}
			case OBJ_CLOSURE:
				return call(AS_CLOSURE(callee),argCount);
			case OBJ_NATIVE:{
				NativeFn native=AS_NATIVE(callee);
				Value result=native(argCount,vm.stackTop-argCount);
				vm.stackTop-=argCount+1;
				push(result);
				return true;
			}
			default:
				break;
		}
	}
	runtimeError("Can only call functions and classes");
	return false;
}

static bool invokeFromClass(ObjClass* klass,ObjString* name,int argCount){
	Value method;
		
	if(!tableGet(&klass->methods,name,&method)){
		runtimeError("Undefined property '%s'.",name->chars);
		return false;
	}
	return call(AS_CLOSURE(method),argCount);
}

static bool invoke(ObjString* name,int argCount){
	Value receiver=peek(argCount);
	if(!IS_INSTANCE(receiver)){
			runtimeError("Only instances have methods");
			return false;
	}
	ObjInstance* instance=AS_INSTANCE(receiver);
	Value value;
	if(tableGet(&instance->fields,name,&value)){
		vm.stackTop[-argCount-1]=value;
		return callValue(value,argCount);
	}
	
	return invokeFromClass(instance->klass,name,argCount);
}

static bool bindMethod(ObjClass* klass,ObjString* name){
	Value method;
	if(!tableGet(&klass->methods,name,&method)){
		runtimeError("Undefined property or method '%s'.",name->chars);
		return false;
	}
	ObjBoundMethod* bound=newBoundMethod(peek(0),AS_CLOSURE(method));
	pop();
	push(OBJ_VAL(bound));
	return true;
}

static ObjUpvalue* captureUpvalue(Value* local){
	ObjUpvalue* prevUpvalue=NULL;
	ObjUpvalue* upvalue=vm.openUpvalues;
	while(upvalue!=NULL && upvalue->location >local){
		prevUpvalue=upvalue;
		upvalue=upvalue->next;
	}
	
	if(upvalue!=NULL && upvalue->location==local){
		return upvalue;
	}
	
	ObjUpvalue* createdUpvalue=newUpvalue(local);
	createdUpvalue->next=upvalue;
	
	if(prevUpvalue==NULL){
		vm.openUpvalues=createdUpvalue;
	}else{
		prevUpvalue->next=createdUpvalue;
	}
	return createdUpvalue;
}

static void closeUpvalues(Value* last){
	while(vm.openUpvalues!=NULL && vm.openUpvalues->location>=last){
		ObjUpvalue* upvalue=vm.openUpvalues;
		upvalue->closed=*upvalue->location;
		upvalue->location=&upvalue->closed;
		vm.openUpvalues=upvalue->next;
	}
}

static void defineMethod(ObjString* name){
	Value method=peek(0);
	ObjClass* klass=AS_CLASS(peek(1));
	tableSet(&klass->methods,name,method);
	pop();
}

static bool isFalsey(Value value){
	return IS_NIL(value)||(IS_BOOL(value) && !AS_BOOL(value));
}
static void concatenate(){
	ObjString* b=AS_STRING(peek(0));
	ObjString* a=AS_STRING(peek(1));
	int length=a->length+b->length;
	char* chars=ALLOCATE(char,length+1);
	memcpy(chars,a->chars,a->length);
	memcpy(chars+a->length,b->chars,b->length);
	chars[length]='\0';
	
	ObjString* result=takeString(chars,length);
	pop();
	pop();
	push(OBJ_VAL(result));
}
static void concatenateStringNumber(double x,ObjString* a,bool STRING_FIRST){
	size_t len=0;
	len=snprintf(NULL,len,"%.*f",precision,x);
	char* dble_string=ALLOCATE(char,len+1);
	snprintf(dble_string,len+1,"%.*f",precision,x);
	int length=a->length+len;
	char* chars=ALLOCATE(char,length+1);
	if(STRING_FIRST){
		memcpy(chars,a->chars,a->length);
		memcpy(chars+a->length,dble_string,len);
	}else{
		memcpy(chars,dble_string,len);
		memcpy(chars+len,a->chars,a->length);
	}
	chars[length]='\0';
	
	free(dble_string);
	
	ObjString* result=takeString(chars,length);
	pop();
	pop();
	push(OBJ_VAL(result));
}

static void catStringChar(){
 ObjString* a=AS_STRING(peek(0));
	char app=AS_CHAR(peek(1));
	char str[1];
	str[0]=app;
	int len=a->length+2;
	char* chars=ALLOCATE(char,len);
	
	memcpy(chars,str,1);
	memcpy(chars+1,a->chars,a->length);
	
	ObjString* result=takeString(chars,len);
	pop();
	pop();
	push(OBJ_VAL(result));
}
static void catCharString(){
	char app=AS_CHAR(peek(0));
	char str[2];
	str[0]=app;
	ObjString* a=AS_STRING(peek(1));
	int len=a->length+1;
	char* chars=ALLOCATE(char,len+1);
	
	memcpy(chars,a->chars,a->length);
	memcpy(chars+a->length,str,1);
	chars[len]='\0';
	
	ObjString* result=takeString(chars,len);
	pop();
	pop();
	push(OBJ_VAL(result));
}
static void _addRealComplex(){
	double a=AS_NUMBER(peek(0));
	ObjComplex* b=AS_COMPLEX(peek(1));
	ObjComplex* c=addRealComplex(a,b);
	pop();
	pop();
	push(OBJ_VAL(c));
}
static void _addComplexReal(){
	ObjComplex* a=AS_COMPLEX(pop());
	double b=AS_NUMBER(pop());
	ObjComplex* c=addComplexReal(a,b);
	push(OBJ_VAL(c));
}
static void _minusComplexReal(){
	double b=AS_NUMBER(pop());
	ObjComplex* a=AS_COMPLEX(pop());
	ObjComplex* c=subtractComplexReal(a,b);
	push(OBJ_VAL(c));
}
static void _minusRealComplex(){
	ObjComplex* a=AS_COMPLEX(pop());
	double b=AS_NUMBER(pop());
	ObjComplex* c=subtractRealComplex(b,a);
	push(OBJ_VAL(c));
}
static void _negateComplex(){
	ObjComplex* t=AS_COMPLEX(pop());
	push(OBJ_VAL(negateComplex(t)));
}
static ERROR_TYPE handlePlus(CallFrame* frame,uint8_t* ip){
	 if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
		double b=AS_NUMBER(pop());
		double a=AS_NUMBER(pop());
		push(NUMBER_VAL(a+b));
		return NO_ERROR;
	}else if(IS_NUMBER(peek(0)) && IS_STRING(peek(1))){
		 concatenateStringNumber(AS_NUMBER(peek(0)),AS_STRING(peek(1)),true);return NO_ERROR;
	}else if(IS_STRING(peek(0)) && IS_NUMBER(peek(1))){
		 concatenateStringNumber(AS_NUMBER(peek(1)),AS_STRING(peek(0)),false);return NO_ERROR;
	}else if(IS_STRING(peek(0)) && IS_STRING(peek(1))){
		 concatenate();
	}else if(IS_COMPLEX(peek(0)) && IS_COMPLEX(peek(1))){
		ObjComplex* a=AS_COMPLEX(peek(0));
		ObjComplex* b=AS_COMPLEX(peek(1));
		ObjComplex* c=addComplexComplex(a,b);
		pop();
		pop();
		push(OBJ_VAL(c));return NO_ERROR;
	}else if(IS_NUMBER(peek(0)) && IS_COMPLEX(peek(1))){
		 _addRealComplex();return NO_ERROR;
	}else if(IS_STRING(peek(0)) && IS_CHAR(peek(1))){
		 catStringChar();return NO_ERROR;
	}else if(IS_CHAR(peek(0)) && IS_STRING(peek(1))){
		 catCharString();return NO_ERROR;
	}else if(IS_COMPLEX(peek(0)) && IS_NUMBER(peek(1))){
		 _addComplexReal();return NO_ERROR;
	}else if(IS_COMPLEX(peek(0)) && IS_STRING(peek(1))){
		 catCharString();return NO_ERROR;
	}else if(IS_COMPLEX(peek(0)) && IS_STRING(peek(1))){
		 _addComplexReal();return NO_ERROR;
	}else if(IS_REAL_MATRIX(peek(0)) && IS_REAL_MATRIX(peek(1))){
		 ObjRealMatrix* b=AS_REAL_MATRIX(peek(0));
		 ObjRealMatrix* a=AS_REAL_MATRIX(peek(1));
		 	int rows_a=a->rows;
			int cols_a=a->columns;
			int rows_b=b->rows;
			int cols_b=b->columns;
		if(rows_a!=rows_b && cols_a!=cols_b) { 
			err_operand1=OBJ_VAL(b);
			err_operand2=OBJ_VAL(a);
			return DIMENSION_ERROR;
		}else{
		 ObjRealMatrix* res=AMM(a,b);
		 pop();
		 pop();
		 push(OBJ_VAL(res));return NO_ERROR;
		 }
	}else{
		err_operand1=peek(0);
		err_operand2=peek(1);
		return INCOMPATIBLE_OPERANDS;
	}
}
static InterpretResult handleMinus(){
	if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
		double b=AS_NUMBER(pop());
		double a=AS_NUMBER(pop());
		push(NUMBER_VAL(a-b));
	}else if(IS_COMPLEX(peek(0)) && IS_COMPLEX(peek(1))){
		ObjComplex* a=AS_COMPLEX(pop());
		ObjComplex* b=AS_COMPLEX(pop());
		ObjComplex* c=subtractComplexComplex(b,a);
		push(OBJ_VAL(c));
	}else if(IS_NUMBER(peek(0)) && IS_COMPLEX(peek(1))){
		 _minusComplexReal();
	}else if(IS_COMPLEX(peek(0)) && IS_NUMBER(peek(1))){
		_minusRealComplex();
	}else{
		runtimeError("Unexpected operands '%s' and '%s' for '-'",getValueType(peek(0)),getValueType(peek(1)));
		return INTERPRET_RUNTIME_ERROR;
	}
}
static InterpretResult handleProduct(){
	if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
		double b=AS_NUMBER(pop());
		double a=AS_NUMBER(pop());
		push(NUMBER_VAL(a*b));
	}else if(IS_COMPLEX(peek(0)) && IS_COMPLEX(peek(1))){
		ObjComplex* a=AS_COMPLEX(peek(0));
		ObjComplex* b=AS_COMPLEX(peek(1));
		ObjComplex* c=multiplyComplexComplex(b,a);
		pop();
		pop();
		push(OBJ_VAL(c));
	}else if(IS_NUMBER(peek(0)) && IS_COMPLEX(peek(1))){
		double x=AS_NUMBER(peek(0));
		ObjComplex* c1=AS_COMPLEX(peek(1));
		ObjComplex* res=multiplyRealComplex(x,c1);   
		pop();
		pop();
		push(OBJ_VAL(res));
	}else if(IS_COMPLEX(peek(0)) && IS_NUMBER(peek(1))){
		ObjComplex* c1=AS_COMPLEX(peek(0));
		double x=AS_NUMBER(peek(1));
		ObjComplex* res=multiplyRealComplex(x,c1);
		pop();
		pop();
		push(OBJ_VAL(res));
	}else if(IS_REAL_MATRIX(peek(0)) && IS_REAL_MATRIX(peek(1))){
		ObjRealMatrix* b=AS_REAL_MATRIX(pop());
		ObjRealMatrix* a=AS_REAL_MATRIX(pop());
		push(OBJ_VAL(PMM(a,b)));
	}else{
		runtimeError("Unexpected operand types %s and %s for '*'",getValueType(peek(0)),getValueType(peek(1)));
		return INTERPRET_RUNTIME_ERROR;
	}
}
static InterpretResult handleDivision(){
	if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
		double b=AS_NUMBER(pop());
		double a=AS_NUMBER(pop());
		push(NUMBER_VAL(a/b));
	}else if(IS_COMPLEX(peek(0)) && IS_COMPLEX(peek(1))){
		ObjComplex* a=AS_COMPLEX(pop());
		ObjComplex* b=AS_COMPLEX(pop());
		ObjComplex* c=divideComplexComplex(b,a);
		push(OBJ_VAL(c));
	}else if(IS_NUMBER(peek(0)) && IS_COMPLEX(peek(1))){
		double x=AS_NUMBER(pop());
		ObjComplex* c1=AS_COMPLEX(pop());
		ObjComplex* res=divideComplexReal(c1,x);
		push(OBJ_VAL(res));
	}else if(IS_COMPLEX(peek(0)) && IS_NUMBER(peek(1))){
		ObjComplex* c1=AS_COMPLEX(pop());
		double x=AS_NUMBER(pop());
		ObjComplex* res=divideRealComplex(x,c1);
		push(OBJ_VAL(res));
	}else{
		runtimeError("Unexpected operand types %s and %s for '/'",getValueType(peek(0)),getValueType(peek(1)));
		return INTERPRET_RUNTIME_ERROR;
	}
}
static InterpretResult run(){
	ERROR_TYPE error_type;
	CallFrame* frame=&vm.frames[vm.frameCount-1];
	register uint8_t* ip=frame->ip;
	#define READ_BYTE() (*ip++)
	#define READ_SHORT() \
		(ip+=2,(uint16_t)((ip[-2] <<8) | ip[-1]))
	#define BINARY_OP(valueType,op) \
    do { \
    	if(!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
    		frame->ip=ip;\
    		runtimeError("Operands must be numbers."); \
    		return INTERPRET_RUNTIME_ERROR; \
    		} \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)
	#define READ_CONSTANT() (frame->closure->function->chunk.literals.values[READ_BYTE()])
	#define READ_STRING()	AS_STRING(READ_CONSTANT())
	for(;;){
		#ifdef DEBUG_TRACE_EXECUTION
			printf("       ");
			for(Value* slot=vm.stack;slot<vm.stackTop;slot++){
				printf("[");
				printValue(*slot,stdout);
				printf("]");
			}
			printf("\n");
			disassembleInstruction(&frame->closure->function->chunk,(int)(frame->ip-frame->closure->function->chunk.code),stdout);
		#endif
		uint8_t instruction;
		switch(instruction=READ_BYTE()){
			case OP_CONSTANT:{
				Value constant=READ_CONSTANT();
				push(constant);
				break;
				}
			case OP_NIL:
				push(NIL_VAL);
				break;
			case OP_TRUE:
				push(BOOL_VAL(true));
				break;
			case OP_FALSE:
				push(BOOL_VAL(false));
				break;
			case OP_GET_UPVALUE:{
				uint8_t slot=READ_BYTE();
				push(*frame->closure->upvalues[slot]->location);
				break;
			}
			case OP_SET_UPVALUE:{
				uint8_t slot=READ_BYTE();
				*frame->closure->upvalues[slot]->location=peek(0);
				break;
			}
			case OP_EQUAL:{
					Value b=pop();
					Value a=pop();
					push(BOOL_VAL(valuesEqual(a,b)));
					break;
				}
			case OP_NOT:
      	push(BOOL_VAL(isFalsey(pop())));
      	break;
			case OP_GREATER:	BINARY_OP(BOOL_VAL,>);break;
			case OP_LESS:			BINARY_OP(BOOL_VAL,<);break;
		  case OP_ADD:{
		  	error_type=handlePlus(frame,ip);
		  	if(error_type==DIMENSION_ERROR){
		  		frame->ip=ip;
					runtimeError("Matrix dimensions should match");
					return INTERPRET_RUNTIME_ERROR;
		  	}else if(error_type==INCOMPATIBLE_OPERANDS){
		  		frame->ip=ip;
		  		runtimeError("Unexpected operand types '%s' and '%s' for '+'",getValueType(err_operand1),getValueType(err_operand2));
					return INTERPRET_RUNTIME_ERROR;
		  	}
		  	break;
		  }
      case OP_SUBTRACT:{
     // 	frame->ip=ip;
      	handleMinus();
      	break;
      }
      case OP_MULTIPLY:{
      //	frame->ip=ip;
      	handleProduct();
      	break;
      }
      case OP_DIVIDE: {
      	//frame->ip=ip;
      	handleDivision();
      	break;
      }
      case OP_POWER:{
      	//	frame->ip=ip;
      		double exp =AS_NUMBER(pop());
      		double base=AS_NUMBER(pop());
      		push(NUMBER_VAL(pow(base,exp)));
      		break;
      	}
      case OP_MODULUS:{
      	//	frame->ip=ip;
      		int op1=(int)AS_NUMBER(pop());
      		int op2=(int)AS_NUMBER(pop());
      		push(NUMBER_VAL(op2 %op1 ));//Time taken:25.61482 seconds for 1000000 primes
      		break;
      	}
			case OP_NEGATE:
				/*if(!IS_NUMBER(peek(0))){
					runtimeError("Operand must be a number");
					return INTERPRET_RUNTIME_ERROR;
				}*/
				//push(-pop());
				if(IS_NUMBER(peek(0)))
					*(vm.stackTop-1)=NUMBER_VAL(-AS_NUMBER(vm.stackTop[-1]));
				else if(IS_COMPLEX(peek(0)))
					_negateComplex();
				else{
					frame->ip=ip;
					runtimeError("Operand must be a number");
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			case OP_POP:	pop();	break;
			case OP_GET_LOCAL:{
				uint8_t slot=READ_BYTE();
				push(frame->slots[slot]);
				break;
			}
			case OP_SET_LOCAL:{
				uint8_t slot=READ_BYTE();
				frame->slots[slot]=peek(0);
				break;
			}
			case OP_GET_GLOBAL:{
				ObjString* name=READ_STRING();
				Value value;
				if(!tableGet(&vm.globals,name,&value)){
					frame->ip=ip;
					runtimeError("Undefined variable '%s'.",name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}
				push(value);
				break;
			}
			case OP_DEFINE_GLOBAL:{
				ObjString* name=READ_STRING();
				tableSet(&vm.globals,name,peek(0));
				pop();
				break;
			}
			case OP_SET_GLOBAL:{
				ObjString* name=READ_STRING();
				if(tableSet(&vm.globals,name,peek(0))){
					tableDelete(&vm.globals,name);
					frame->ip=ip;
					runtimeError("Undefined variable '%s'.",name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			}
			case OP_PRINT:
				printValue(pop(),stdout);
				printf("\n");
				break;
			case OP_INCREMENT:
				if(!IS_NUMBER(peek(0))) RUNTIME_ERROR("Expected real operand");
				double c=AS_NUMBER(vm.stackTop[-1]);
				*(vm.stackTop-1)=NUMBER_VAL(++c);
				break;
			case OP_DECREMENT:
				if(!IS_NUMBER(peek(0))) RUNTIME_ERROR("Expected real operand");
				double c1=AS_NUMBER(vm.stackTop[-1]);
				*(vm.stackTop-1)=NUMBER_VAL(--c1);
				break;
			case OP_JUMP:{
				uint16_t offset=READ_SHORT();
				ip+=offset;
				break;
			}
			case OP_JUMP_IF_FALSE:{
				uint16_t offset=READ_SHORT();
				if(isFalsey(peek(0)))	ip+=offset;
				break;
			}
			case OP_LOOP:{
				uint16_t offset=READ_SHORT();
				ip-=offset;
				break;
			}
			case OP_CALL:{
				int argCount=READ_BYTE();
				frame->ip=ip;
				if(!callValue(peek(argCount),argCount)){
					return INTERPRET_RUNTIME_ERROR;
				}
				frame=&vm.frames[vm.frameCount-1];
				ip=frame->ip;
				break;
			}
			case OP_CLOSURE:{
				ObjFunction* function=AS_FUNCTION(READ_CONSTANT());
				ObjClosure* closure=newClosure(function);
				push(OBJ_VAL(closure));
				for(int i=0;i<closure->upvalueCount;i++){
					uint8_t isLocal=READ_BYTE();
					uint8_t index=READ_BYTE();
					if(isLocal){
						closure->upvalues[i]=captureUpvalue(frame->slots+index);
					}else{
						closure->upvalues[i]=frame->closure->upvalues[index];
					}
				}
				break;
			}
			case OP_CLASS:{
				push(OBJ_VAL(newClass(READ_STRING())));
				break;
			}
			case OP_INSTANCE_OF:{
				if(!IS_INSTANCE(peek(1)) || AS_INSTANCE(peek(1))->klass==NULL){
					pop();
					pop();
					push(BOOL_VAL(false));
				}else{
					ObjInstance* instance=AS_INSTANCE(peek(1));
					ObjClass* op_klass=AS_CLASS(peek(0));
					ObjClass* inst_klass=instance->klass;
					bool is_inst_of=false;
					while(inst_klass!=NULL){
						if(inst_klass==op_klass){
							is_inst_of=true;
							break;
						}else{
							inst_klass=inst_klass->parent;
						}
					}
					pop();
					pop();
					push(BOOL_VAL(is_inst_of));
				}
				break;
			}
			case OP_CLOSE_UPVALUE:{
				closeUpvalues(vm.stackTop-1);
				pop();
				break;
			}
			case OP_RETURN:{
				Value result=pop();
				closeUpvalues(frame->slots);
				vm.frameCount--;
				if(vm.frameCount==0){
					pop();
					return INTERPRET_OK;
				}
				vm.stackTop=frame->slots;
				push(result);
				frame=&vm.frames[vm.frameCount-1];
				ip=frame->ip;
				break;
				}
			case OP_BUILD_LIST:{
				//stack before [item1,item2,item3.....] after [list]
				ObjList* list=newList();
				uint8_t itemCount=READ_BYTE();
				push(OBJ_VAL(list));
				for(int i=itemCount;i>0;i--){
					appendToList(list,peek(i));
				}
				pop();
				//pop the items from the stack
				while(itemCount-- >0)
					pop();
				push(OBJ_VAL(list));
				break;
			}
			case OP_BUILD_MATRIX:{
				uint8_t row_count=READ_BYTE();
				uint8_t col_count=READ_BYTE();
				int x=row_count*col_count-1;
				ObjRealMatrix* matrix=initRealMatrix(row_count,col_count);
				push(OBJ_VAL(matrix));
				int k=0;
				for(int i=row_count;i>0;i--){
					for(int j=col_count;j>0;j--){
						matrix->data[row_count-i][col_count-j]=AS_NUMBER(peek(x-k+1));
						k++;
					}
				}
				pop();
				while(x-- >0)
					pop();
				push(OBJ_VAL(matrix));
				break;
			}
			case OP_INDEX_SUBSCR:{
				// Stack before: [list, index] and after: [index(list, index)]
				Value result;
				if(IS_LIST(peek(1))){
					Value vindex=pop();
					Value vlist=pop();
					ObjList* list=AS_LIST(vlist);
					if(!IS_NUMBER(vindex))	RUNTIME_ERROR("Index should be a number");
					int index=AS_NUMBER(vindex);
					if(!isValidListIndex(list,index)) RUNTIME_ERROR("List index out of range");
					result=indexFromList(list,AS_NUMBER(vindex));
					push(result);
				}else if(IS_MAP(peek(1))){
					Value vindex=pop();
					Value vlist=pop();
					ObjMap* map=AS_MAP(vlist);
					tableGet(&map->values,AS_STRING(vindex),&result);
					push(result);
				}else if(IS_STRING(peek(1))){
					Value vindex=pop();
					Value vlist=pop();
					ObjString* string=AS_STRING(vlist);
					if(!IS_NUMBER(vindex))	RUNTIME_ERROR("Index should be a number");
					int index=AS_NUMBER(vindex);
					if(!isValidStringIndex(string,index))RUNTIME_ERROR("List index out of range");
					result=indexFromString(string,index);
					push(result);
				}else if(IS_REAL_MATRIX(peek(2))){
					Value col=pop();
					Value row=pop();
					if(!IS_NUMBER(col)) RUNTIME_ERROR("Column index should be a number");
					else if(!IS_NUMBER(row)) RUNTIME_ERROR("Row index should be a number");
					int i_col=AS_NUMBER(col);
					int i_row=AS_NUMBER(row);
					ObjRealMatrix* mat=AS_REAL_MATRIX(pop());
					if(i_row>=mat->rows ){RUNTIME_ERROR("Row index out of range");}
					else if(i_col>=mat->columns) RUNTIME_ERROR("Column index out of range"); 
					push(indexFromMatrix(mat,i_row,i_col));
				}else RUNTIME_ERROR("Invalid type to index into");
				break;
			}
			case OP_INDEX_SUBSCR_2:{
				Value vindex=peek(0);
				Value vlist=peek(1);
				Value result;
				if(IS_LIST(vlist)){
					ObjList* list=AS_LIST(vlist);
					if(!IS_NUMBER(vindex)) RUNTIME_ERROR("Index should be a number");
					int index=AS_NUMBER(vindex);
					if(!isValidListIndex(list,index)) RUNTIME_ERROR("Index out of range");
					result=indexFromList(list,AS_NUMBER(vindex));
					push(result);
				}else RUNTIME_ERROR("Invalid type to index into");
				break;
			}
			case OP_STORE_SUBSCR:{
				 // Stack before: [list, index, item] and after: [item]
				 Value item=pop();
				 if(IS_LIST(peek(1))){
				 	Value vindex=pop();
				 	Value vlist=pop();
				 	if(!IS_LIST(vlist))RUNTIME_ERROR("Cannot store a value in non list item");
					ObjList* list=AS_LIST(vlist);
					if(!IS_NUMBER(vindex))RUNTIME_ERROR("Index should be a number");
				 	int index=AS_NUMBER(vindex);
				 	if(!isValidListIndex(list,index)) RUNTIME_ERROR("Invalid list index");
					storeToList(list,index,item);
				 	push(item);
				 }else if(IS_REAL_MATRIX(peek(2))){
				 		int i_col=AS_NUMBER(pop());
				 		int i_row=AS_NUMBER(pop());
				 		ObjRealMatrix* mat=AS_REAL_MATRIX(pop());
				 		storeToMatrix(mat,i_row,i_col,AS_NUMBER(item));
				 		push(item);
				 }else RUNTIME_ERROR("Invalid type for indexing and storing");
				break;
			}
			case OP_BUILD_SLICE:{
				if(IS_LIST(peek(3))){
					int step=AS_NUMBER(pop());
					int end=AS_NUMBER(pop());
					int start=AS_NUMBER(pop());
					ObjList* list=AS_LIST(pop());
					if(end>list->count) RUNTIME_ERROR("End index out of range error.");
					else if(start<0 || start>list->count) RUNTIME_ERROR("Start index out of range error");
					ObjList* slice=sliceFromList(list,start,end,step);
					push(OBJ_VAL(slice));
				}else if(IS_LIST(peek(2))){
					int end=AS_NUMBER(pop());
					int start=AS_NUMBER(pop());
					ObjList* list=AS_LIST(pop());
					if(end>list->count) RUNTIME_ERROR("End index out of range error.");
					else if(start<0 || start>list->count) RUNTIME_ERROR("Start index out of range error");
					ObjList* slice=sliceFromList(list,start,end,1);
					push(OBJ_VAL(slice));
				}
				break;
			}
			case OP_BUILD_MAP:{
				ObjMap* map = newMap();
        uint8_t itemCount= READ_BYTE();
        push(OBJ_VAL(map)); // So that the map isn't sweeped by GC when table allocates
       	int i = 2 * itemCount;
        while (i > 0) {
          Value key = peek(i--);
         	Value value = peek(i--);
         //if (!isHashable(key)) {
           //  runtimeError("Map key is not hashable.");
           //  return INTERPRET_RUNTIME_ERROR;
          // }
          tableSet(&map->values, AS_STRING(key), value);
          }
          pop();
         while (itemCount-- > 0) {
          pop();
          pop();
         }
         push(OBJ_VAL(map));
         break;
			}
			case OP_GET_PROPERTY:{
				if(!IS_INSTANCE(peek(0))){
					frame->ip=ip;
					runtimeError("Only instances have properties");
					return INTERPRET_RUNTIME_ERROR;
				}
				
				ObjInstance* instance=AS_INSTANCE(peek(0));
				ObjString* name=READ_STRING();
				
				Value value;
				if(tableGet(&instance->fields,name,&value)){
					pop();
					push(value);
					break;
				}
				
				if(!bindMethod(instance->klass,name)){
					frame->ip=ip;
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			}
			case OP_SET_PROPERTY:{
				if(!IS_INSTANCE(peek(1))){
					frame->ip=ip;
					runtimeError("Only fields have instances.");
					return INTERPRET_RUNTIME_ERROR;
				}
				ObjInstance* instance=AS_INSTANCE(peek(1));
				tableSet(&instance->fields,READ_STRING(),peek(0));
				Value value=pop();
				pop();
				push(value);
				break;
			}
			case OP_METHOD:
				defineMethod(READ_STRING());
				break;
			case OP_INVOKE:{
				ObjString* method=READ_STRING();
				int argCount=READ_BYTE();
				frame->ip=ip;
				if(!invoke(method,argCount)){
					return INTERPRET_RUNTIME_ERROR;
				}
				frame=&vm.frames[vm.frameCount-1];
				ip=frame->ip;
				break;
			}
			case OP_INHERIT:{
				Value superclass=peek(1);
				if(!IS_CLASS(superclass)){
					frame->ip=ip;
					runtimeError("Superclass must be a class");
					return INTERPRET_RUNTIME_ERROR;
				}
				ObjClass* subclass=AS_CLASS(peek(0));
				subclass->parent=AS_CLASS(superclass);
				tableAddAll(&AS_CLASS(superclass)->methods,&subclass->methods);
				pop();
				break;
			}
			case OP_GET_SUPER:{
				ObjString* name=READ_STRING();
				ObjClass* superclass=AS_CLASS(pop());
				
				if(!bindMethod(superclass,name)){
					frame->ip=ip;
					return INTERPRET_RUNTIME_ERROR;
				}
			}
			case OP_SUPER_INVOKE:{
				ObjString* method=READ_STRING();
				int argCount =READ_BYTE();
				ObjClass* superClass=AS_CLASS(pop());
				frame->ip=ip;
				if(!invokeFromClass(superClass,method,argCount)){
					return INTERPRET_RUNTIME_ERROR;
				}
				frame=&vm.frames[vm.frameCount-1];
				ip=frame->ip;
				break;
			}
			case OP_GET_COL:{
				int col_index=AS_NUMBER(pop());
				ObjRealMatrix* mat=AS_REAL_MATRIX(pop());
				if(col_index>mat->columns){
					frame->ip=ip;
					runtimeError("Column index out of range");
					return INTERPRET_RUNTIME_ERROR;
				}
				ObjRealMatrix* col=getColumn(mat,col_index);
				push(OBJ_VAL(col));
				break;
			}
			case OP_GET_ROW:{
				int row_index=AS_NUMBER(pop());
				ObjRealMatrix* mat=AS_REAL_MATRIX(pop());
				if(row_index>mat->rows){
					frame->ip=ip;
					runtimeError("Row index out of range");
					return INTERPRET_RUNTIME_ERROR;
				}
				ObjRealMatrix* row=getRow(mat,row_index);
				push(OBJ_VAL(row));
				break;
			}
		}
	}
	#undef READ_BYTE
	#undef READ_SHORT
	#undef READ_CONSTANT
	#undef READ_STRING
}
InterpretResult interpret(const char* source){
	ObjFunction* function=compile(source);
	if(function==NULL)	return INTERPRET_COMPILE_ERROR;
	
	push(OBJ_VAL(function));
	ObjClosure* closure=newClosure(function);
	pop();
	push(OBJ_VAL(closure));
	call(closure,0);
	
	return run();
}

