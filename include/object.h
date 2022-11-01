#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)					(AS_OBJ(value)->type)

#define IS_FUNCTION(value)			isObjType(value,OBJ_FUNCTION)
#define IS_NATIVE(value)				isObjType(value,OBJ_NATIVE)
#define IS_STRING(value)				isObjType(value,OBJ_STRING)
#define IS_COMPLEX(value)				isObjType(value,OBJ_COMPLEX)
#define IS_LIST(value)					isObjType(value,OBJ_LIST)
#define IS_REAL_MATRIX(value)		isObjType(value,OBJ_MATRIX)
#define IS_MAP(value)						isObjType(value,OBJ_MAP)
#define IS_CLOSURE(value)				isObjType(value,OBJ_CLOSURE)
#define IS_CLASS(value)					isObjType(value,OBJ_CLASS)
#define IS_INSTANCE(value)			isObjType(value,OBJ_INSTANCE)
#define IS_BOUND_METHOD(value)	isObjType(value,OBJ_BOUND_METHOD)

#define AS_STRING(value)		((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)		(((ObjString*)AS_OBJ(value))->chars)

#define AS_COMPLEX(value)		((ObjComplex*)AS_OBJ(value))
#define AS_CCOMPLEX(value)	\
	((ObjComplex*)AS_OBJ(value))->REAL,(((ObjComplex*)AS_OBJ(value))->IMAGINARY)>=0.0f?'+':'\0',((ObjComplex*)AS_OBJ(value))->IMAGINARY

#define AS_FUNCTION(value)	((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)\
	(((ObjNative*)AS_OBJ(value))->function)
#define AS_CLOSURE(value)				((ObjClosure*)AS_OBJ(value))
#define AS_CLASS(value)					((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value)			((ObjInstance*)AS_OBJ(value))
#define AS_BOUND_METHOD(value)	((ObjBoundMethod*)AS_OBJ(value))

#define AS_LIST(value)				((ObjList*)AS_OBJ(value))
#define AS_REAL_MATRIX(value) ((ObjRealMatrix*)AS_OBJ(value))
#define AS_MAP(value)					((ObjMap*)AS_OBJ(value))

typedef enum{
	OBJ_CLOSURE,//0
	OBJ_CLASS,
	OBJ_BOUND_METHOD,
	OBJ_INSTANCE,
	OBJ_MAP,//1
	OBJ_COMPLEX,//2
	OBJ_MATRIX,//3
	OBJ_LIST,//5
	OBJ_FUNCTION,//8
	OBJ_NATIVE,//9
	OBJ_STRING,//10
	OBJ_UPVALUE
}ObjType;

struct Obj{
	ObjType type;
	bool isMarked;
	struct Obj* next;
};

typedef struct{
	Obj obj;	//16B
	int arity;//4B
	int upvalueCount;//4B
	Chunk chunk;//12B
	ObjString* name; //40B
}ObjFunction;

typedef Value (*NativeFn)(int argCount,Value* args);

typedef struct{
	Obj obj;
	NativeFn function;
}ObjNative;

typedef struct{
	Obj obj;
	Value* location;
	Value closed;
	struct ObjUpvalue* next;
}ObjUpvalue;

typedef struct{
	Obj obj;
	ObjFunction* function;
	ObjUpvalue** upvalues;
	int upvalueCount;
}ObjClosure;

struct ObjString{
	Obj obj; //16B
	int length;//4B
	char* chars;//8B
	uint32_t hash;//4B
};

typedef struct ObjClass{
	Obj obj;
	ObjString* name;
	Table methods;
	Table op_overloads;
	struct ObjClass* parent;//to help during inheritance
}ObjClass;

typedef struct{
	Obj obj;
	ObjClass* klass;
	Table fields;
}ObjInstance;

typedef struct{
	Obj obj;
	Value receiver;
	ObjClosure* method;
}ObjBoundMethod;

typedef struct{
	Obj obj;
	Value receiver;
	NativeFn function;
}ObjNativeBoundMethod;

struct ObjComplex{
	Obj obj;
	double REAL;
	double IMAGINARY;
};

typedef struct ObjRealMatrix{
	Obj obj; //16B
	int rows;//4B
	int columns;//4B
	double** data;//8B
	struct ObjRealMatrix* factor1;//8B
	struct ObjRealMatrix* factor2;//8B
	int* perm_vector;//8B
	double determinant;//8B
}ObjRealMatrix;//64B

struct ObjRealVector{
	Obj obj;
	int length;
	double* values;
};

typedef struct{
	Obj obj;
	int count;
	int capacity;
	Value* items;
}ObjList;

typedef struct{
	Obj obj;
	Table values;
}ObjMap;

typedef struct{
	Obj obj;
	ObjString* name;
	Table globals;
	Table methods;
}ObjModule;

char* getType(Value);
ObjClass* newClass(ObjString* name);
ObjClosure* newClosure(ObjFunction* function);
ObjFunction* newFunction();
ObjInstance* newInstance(ObjClass* klass);
ObjNative* newNative(NativeFn function);
ObjUpvalue* newUpvalue(Value* slot);
ObjBoundMethod* newBoundMethod(Value receiver,ObjClosure* method);
ObjList* newList();
ObjString* takeString(char* chars,int length );
ObjString* copyString(const char* chars,int length );
ObjComplex* parseImaginary(const char* chars);
ObjComplex* allocateComplex(double,double);
ObjRealMatrix* initRealMatrix(int,int);
ObjMap* newMap();

void appendToList(ObjList*,Value);
void deleteFromList(ObjList*,int);
void storeToList(ObjList*,int,Value);
void storeToMatrix(ObjRealMatrix*,int,int,double);
Value indexFromList(ObjList*,int);
ObjList* sliceFromList(ObjList*,int,int,int);
bool isValidListIndex(ObjList*,int);
bool isValidStringIndex(ObjString*,int);
Value indexFromString(ObjString*,int);
Value indexFromMatrix(ObjRealMatrix*,int,int);
ObjRealMatrix* getRow(ObjRealMatrix*,int);
ObjRealMatrix* getColumn(ObjRealMatrix*,int);
void setRow(ObjRealMatrix*,double*,int);
void setCol(ObjRealMatrix*,double*,int);

Value lox_native_prod_list_scalar(int,Value*);
Value lox_native_convolute(int,Value*);
Value lox_native_mat_cols(int,Value*);
Value lox_native_mat_rows(int,Value*);
Value lox_native_is_complex(int,Value*);
Value lox_native_is_matrix(int,Value*);
Value lox_native_is_list(int,Value*);
Value lox_native_is_function(int,Value*);
Value lox_native_is_string(int,Value*);
Value lox_native_is_number(int,Value*);
Value lox_native_is_char(int,Value*);
Value lox_native_is_class_type(int,Value*);

void printObject(Value value,FILE*);

static inline bool isObjType(Value value,ObjType type){
	return IS_OBJ(value) && AS_OBJ(value)->type==type;
}

#endif
