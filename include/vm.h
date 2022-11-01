#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 256
#define STACK_MAX (FRAMES_MAX*UINT8_COUNT)


//this call frame represents a single ongoing function call
typedef struct{
	ObjClosure* closure;		//to look up constants within the function
	uint8_t* ip;							//the ip to return to
	Value* slots;							//pointer to slots on the value stack
}CallFrame;

typedef struct{
	CallFrame frames[FRAMES_MAX];
	int frameCount;
	Value stack[STACK_MAX];
	Value* stackTop;
	Table strings;
	Table globals;
	Table natives;
	Table modules;
	ObjString* initString;
	ObjUpvalue* openUpvalues;
	size_t bytesAllocated;
	size_t nextGC;
	Obj* objects;
	
	int grayCount;
	int grayCapacity;
	Obj** grayStack;
}VM;

typedef enum{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
}InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
bool callValue(Value callee,int argCount);
void push(Value value);
Value pop();
//void defineNativeMethod(ObjClass* klass,const char* name,NativeFn method);

//ObjClass* object;

#endif          
