#include<math.h>
#include<stdio.h>

#include "../include/memory.h"
#include "../include/complex.h"
#include "../include/object.h"
#include "../include/value.h"

extern int precision;
double modulus(ObjComplex* c1){
	return sqrt(c1->REAL*c1->REAL+c1->IMAGINARY*c1->IMAGINARY);
}

double argument(ObjComplex* c1){
	return atan2(c1->IMAGINARY,c1->REAL);
}

double modulus2(double real,double imag){
	return sqrt(real*real+imag*imag);
}

double argument2(double real,double imag){
	return atan2(imag,real);
}

double getRealPart(ObjComplex* c1){
	return c1->REAL;
}

double getImaginaryPart(ObjComplex* c1){
	return c1->IMAGINARY;
}

ObjString* complexString(ObjComplex* c1){
	size_t len=0;
	len=snprintf(NULL,len,"%.*f%c%.*fi",precision,c1->REAL,SET_IMAG_SIGN(c1),precision,c1->IMAGINARY); 
	char* heapChars=ALLOCATE(char,len+1);
	snprintf(heapChars,len+1,"%.*f%c%.*fi",precision,c1->REAL,SET_IMAG_SIGN(c1),precision,c1->IMAGINARY);
	return takeString(heapChars,len+1);
}

ObjComplex* addComplexComplex(ObjComplex* a,ObjComplex* b){
	ObjComplex* result=allocateComplex(0,0);
	result->REAL=a->REAL+b->REAL;
	result->IMAGINARY=a->IMAGINARY+b->IMAGINARY;
	return result;
}

ObjComplex* addComplexReal(ObjComplex* a,double b){
	ObjComplex* result=allocateComplex(0,0);	
	result->REAL=a->REAL+b;
	result->IMAGINARY=a->IMAGINARY;
	return result;
}

ObjComplex* addRealComplex(double a,ObjComplex* b){
	ObjComplex* result=allocateComplex(0,0);
	result->REAL=a+b->REAL;
	result->IMAGINARY=b->IMAGINARY;
	return result;
}

ObjComplex* negateComplex(ObjComplex* a){
	return allocateComplex(-a->REAL,-a->IMAGINARY);
}

ObjComplex* subtractComplexComplex(ObjComplex* a,ObjComplex* b){
	ObjComplex* result=allocateComplex(0,0);
	result->REAL=a->REAL-b->REAL;
	result->IMAGINARY=a->IMAGINARY-b->IMAGINARY;
	return result;
}

ObjComplex* subtractComplexReal(ObjComplex* a,double b){
	ObjComplex* result=allocateComplex(0,a->IMAGINARY);
	result->REAL=a->REAL-b;
	return result;
}

ObjComplex* subtractRealComplex(double a,ObjComplex* b){
	ObjComplex* result=allocateComplex(0,-b->IMAGINARY);
	result->REAL=a-b->REAL;
	return result;
}

ObjComplex* multiplyComplexComplex(ObjComplex* c1,ObjComplex* c2){
	double a=c1->REAL;
	double b=c1->IMAGINARY;
	double c=c2->REAL;
	double d=c2->IMAGINARY;
	double m1=(a+b)*(c-d);
	double m2=b*c;
	double m3=a*d;
	return allocateComplex(m1-m2+m3,m2+m3);
}

ObjComplex* multiplyRealComplex(double a,ObjComplex* c1){
	double u=a*(c1->REAL);
	double v=a*(c1->IMAGINARY);
	return allocateComplex(u,v);
}

ObjComplex* multiplyComplexReal(ObjComplex* c1,double b){
	double u=b*(c1->REAL);
	double v=b*(c1->IMAGINARY);
	return allocateComplex(u,v);
}

ObjComplex* getComplexConjugate(ObjComplex* c1){
	return allocateComplex(c1->REAL,-c1->IMAGINARY);
}

ObjComplex* divideComplexComplex(ObjComplex* dividend,ObjComplex* divisor){
	ObjComplex* mult=getComplexConjugate(divisor);
	double den=divisor->REAL*divisor->REAL+divisor->IMAGINARY*divisor->IMAGINARY;
	ObjComplex* num=multiplyComplexComplex(dividend,mult);
	return allocateComplex(num->REAL/den,num->IMAGINARY/den);
}

ObjComplex* divideRealComplex(double x,ObjComplex* c1){
	double den=c1->REAL*c1->REAL+c1->IMAGINARY*c1->IMAGINARY;
	ObjComplex* num=multiplyRealComplex(x,getComplexConjugate(c1));
	return allocateComplex(num->REAL/den,num->IMAGINARY/den);
}

ObjComplex* divideComplexReal(ObjComplex* c1,double x){
	return allocateComplex(c1->REAL/x,c1->IMAGINARY/x);
}

ObjComplex* complexReciprocal(ObjComplex* c1){
	return divideRealComplex(1.0,c1);
}
//---------------------------------------------------------------------------------------------------------------------
ObjComplex* c_sin(ObjComplex* c1){
	double x=c1->REAL;
	double y=c1->IMAGINARY;
	return allocateComplex(sin(x)*cosh(y),cos(x)*sinh(y));
}

ObjComplex* c_cos(ObjComplex* c1){
	double x=c1->REAL;
	double y=c1->IMAGINARY;
	return allocateComplex(cos(x)*cosh(y),-sin(x)*sinh(y));
}

ObjComplex* c_tan(ObjComplex* c1){
	return divideComplexComplex(c_sin(c1),c_cos(c1));
}
