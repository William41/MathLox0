#ifndef clox_native_io_h
#define clox_native_io_h

#include "object.h"
#include "value.h"

Value clockNative(int,Value*);
Value scanNative(int,Value*);
Value scanNumberNative(int,Value*);
Value scanComplexNative(int,Value*);
Value scanStringNative(int,Value*);
Value lox_native_file_write(int,Value*);
Value lox_native_file_read(int,Value*);

Value lox_native_format(int,Value*);
Value lox_native_import_module(int,Value*);

Value printNative(int,Value*);
Value printNewNative(int,Value*);

#endif
