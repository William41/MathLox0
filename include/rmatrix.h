#ifndef clox_datatypes_rmatrix_h
#define clox_datatypes_rmatrix_h

#include "value.h"
#include "object.h"


//Matrix types
//Section contains operations
//Methods take the  name as follows
//operation-op1-op2
//operations:A=+,S=-,P=*
//R=real number ,M=matrix
ObjRealMatrix* AMM(ObjRealMatrix*,ObjRealMatrix*);
ObjRealMatrix* AMR(ObjRealMatrix*,double);
ObjRealMatrix* ARM(double,ObjRealMatrix*);
ObjRealMatrix* SMM(ObjRealMatrix*,ObjRealMatrix*);
ObjRealMatrix* SMR(ObjRealMatrix*,ObjRealMatrix*);
ObjRealMatrix* SRM(double,ObjRealMatrix*);
ObjRealMatrix* PMM(ObjRealMatrix*,ObjRealMatrix*);
ObjRealMatrix* PRM(double,ObjRealMatrix*);

Value lox_native_isSquare(int,Value*);
Value lox_native_isDiagonal(int,Value*);

Value lox_native_randMatrix(int,Value*);
Value lox_native_randUpper(int,Value*);
Value lox_native_randLower(int,Value*);
Value lox_native_randTriDiag(int,Value*);
Value lox_native_randDiag(int,Value*);
Value lox_native_randSym(int,Value*);
Value lox_native_randSkew(int,Value*);

Value lox_native_null(int,Value*);
Value lox_native_identity(int,Value*);

Value lox_native_getTranspose(int,Value*);
Value lox_native_getTrace(int,Value*);
Value lox_native_decomposeLUP(int,Value*);
Value lox_native_getDeterminant(int,Value*);
Value lox_native_get_inverse(int,Value*);
Value lox_native_solveWith(int,Value*);
Value lox_native_get_row(int,Value*);
Value lox_native_get_col(int,Value*);
Value lox_native_set_row(int,Value*);
Value lox_native_set_col(int,Value*);

#endif
