//This header is part of the clox runtime.
//It contains all function headers for the
//initialization of new objects types at runtime
//which include complex,matrices and list
//by native calls in clox.

#ifndef clox_init_object_h
#define clox_init_object_h

#include "object.h"
#include "value.h"



Value asNumNative(int,Value*);
Value asHexNative(int,Value*);

Value newComplexNative(int,Value*);
Value newListNative(int,Value*);
Value newMatrixNative(int argCount,Value* args);


#endif
