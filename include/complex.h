#ifndef clox_datatypes_complex_h
#define clox_datatypes_complex_h

#include "object.h"
#include "value.h"

#define SET_IMAG_SIGN(value) value->IMAGINARY>=0.0f?'+':' '

double modulus(ObjComplex*);
double argument(ObjComplex*);
double modulus2(double,double);
double argument2(double,double);
double getRealPart(ObjComplex*);
double getImaginaryPart(ObjComplex*);
//output as string
ObjString* complexString(ObjComplex*);
//basic operations
ObjComplex* addComplexComplex(ObjComplex*,ObjComplex*);
ObjComplex* addComplexReal(ObjComplex*,double);
ObjComplex* addRealComplex(double,ObjComplex*);
ObjComplex* negateComplex(ObjComplex*);
ObjComplex* subtractComplexComplex(ObjComplex*,ObjComplex*);
ObjComplex* subtractComplexReal(ObjComplex*,double);
ObjComplex* subtractRealComplex(double,ObjComplex*);
ObjComplex* multiplyComplexComplex(ObjComplex*,ObjComplex*);
ObjComplex* multiplyComplexReal(ObjComplex*,double);
ObjComplex* multiplyRealComplex(double,ObjComplex*);
ObjComplex* getComplexConjugate(ObjComplex*);
ObjComplex* divideComplexComplex(ObjComplex*,ObjComplex*);
ObjComplex* divideRealComplex(double,ObjComplex*);
ObjComplex* divideComplexReal(ObjComplex*,double);
ObjComplex* complexReciprocal(ObjComplex*);
//functions of complex numbers...
ObjComplex* c_sin(ObjComplex*);
ObjComplex* c_cos(ObjComplex*);
ObjComplex* c_tan(ObjComplex*);
ObjComplex* c_cosec(ObjComplex*);
ObjComplex* c_sec(ObjComplex*);
ObjComplex* c_cot(ObjComplex*);
ObjComplex* c_arcsin(ObjComplex*);
ObjComplex* c_arccos(ObjComplex*);
ObjComplex* c_arctan(ObjComplex*);
ObjComplex* c_arccosec(ObjComplex*);
ObjComplex* c_arcsec(ObjComplex*);
ObjComplex* c_arccot(ObjComplex*);
ObjComplex* c_sinh(ObjComplex*);
ObjComplex* c_cosh(ObjComplex*);
ObjComplex* c_tanh(ObjComplex*);
ObjComplex* c_cosech(ObjComplex*);
ObjComplex* c_sech(ObjComplex*);
ObjComplex* c_coth(ObjComplex*);
ObjComplex* c_arcsinh(ObjComplex*);
ObjComplex* c_arccosh(ObjComplex*);
ObjComplex* c_arctanh(ObjComplex*);
ObjComplex* c_arccosech(ObjComplex*);
ObjComplex* c_arcsech(ObjComplex*);
ObjComplex* c_arccoth(ObjComplex*);
//powers and exponents...
ObjComplex* principlePower(ObjComplex*,double);
ObjComplex* generalPowers(ObjComplex*,double,int);





#endif
