#include <stdio.h>
#include<time.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include<string.h>
//#ifdef OPENBLAS_CONFIG_H
	#include<cblas.h>
//#endif

#include "../include/memory.h"
#include "../include/object.h"
#include "../include/table.h"
#include "../include/vm.h"
#define EPSILON 1e-100

#define SET_ERROR_ARGS(argc,msg)\
	if(argCount!=argc) Error(msg);

#define SET_ERROR_COND(cond,message)\
	if(cond) Error(message);



static void Error(const char* format, ...){
	va_list args;
	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
	return;
}

static bool isSquare(ObjRealMatrix* A){
	return A->rows==A->columns;
}

static bool isSymmetric(ObjRealMatrix* mat){
	SET_ERROR_COND(!isSquare(mat),"Required square matrix");
	int order=mat->rows;
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
			if(mat->data[i][j]!=mat->data[j][i]) return false;
		}
	}
	return true;
}

static bool isSkewSymmetric(ObjRealMatrix* mat){
	SET_ERROR_COND(!isSquare(mat),"Required square matrix");
	int order=mat->rows;
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
			if(mat->data[i][j]!=-mat->data[j][i]) return false;
		}
	}
	return true;
}

static bool isRectangular(ObjRealMatrix* mat){
	return mat->rows!=mat->columns;
}

static bool isUpperTriangular(ObjRealMatrix* A){
}

static bool isLowerTriangular(ObjRealMatrix* A){
}

static bool isSingular(ObjRealMatrix* A){
}

static bool isIdempotent(ObjRealMatrix* A){
}

static bool isNilpotent(ObjRealMatrix* A){
}

static bool isInvolutary(ObjRealMatrix* A){
}

static bool isTridiagonal(ObjRealMatrix* A){
}

static bool isSparse(ObjRealMatrix* A){
}

static bool isIdentity(ObjRealMatrix* A){
}

static bool isOrthogonal(ObjRealMatrix* A){
}

static bool isToeplitz(ObjRealMatrix* A){
}

static bool isDiagonal(ObjRealMatrix* matrix){
	for(int i=0;i<matrix->rows;i++)
		for(int j=0;j<matrix->columns;j++)
			if(i!=j && matrix->data[i][j]!=0) return false;
	return true;
}

static bool isScalar(ObjRealMatrix* matrix){
	if(!isDiagonal(matrix)) return false;
	else{
		int order=matrix->rows;
		for(int i=0;i<order-1;i++)	
			if(matrix->data[i][i]!=matrix->data[i+1][i+1]) return false;
		return true;
	}
}

static ObjRealMatrix* identity_sq(int order){
	ObjRealMatrix* id=initRealMatrix(order,order);
	for(int i=0;i<order;i++)	id->data[i][i]=1.0;
	return id;
}

static ObjRealMatrix* null_sq(int order){
	ObjRealMatrix* id=initRealMatrix(order,order);
	return id;
}

static ObjRealMatrix* duplicate(ObjRealMatrix* matrix){
	ObjRealMatrix* dup=initRealMatrix(matrix->rows,matrix->columns);
	for(int i=0;i<matrix->rows;i++)
		for(int j=0;j<matrix->columns;j++)
			dup->data[i][j]=matrix->data[i][j];
	return dup;
}

ObjRealMatrix* AMM(ObjRealMatrix* A,ObjRealMatrix* B){
 int rows_a=A->rows;
		int cols_a=A->columns;
		int rows_b=B->rows;
		int cols_b=B->columns;
	ObjRealMatrix* result=initRealMatrix(rows_a,cols_a);
	for(int i=0;i<rows_a;i++){
		for(int j=0;j<cols_a;j++){
			result->data[i][j]=A->data[i][j]+B->data[i][j];
			}
		}
		return result;
}

void transpose2(double* A,double* B,int n){
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			B[j*n+i]=A[i*n+j];
		}
	}
}

ObjRealMatrix* PMM(ObjRealMatrix* A,ObjRealMatrix* B)
{
	int rows_a=A->rows;
	int cols_a=A->columns;
	int rows_b=B->rows;
	int cols_b=B->columns;
	double* a=NULL;
	double* b=NULL;
	double* c=NULL;
	if(cols_a!=rows_b ) {
		Error("Number of rows of first matrix should match the columns of second matrix");
		return AS_REAL_MATRIX(NIL_VAL);
	}else
	{
	double r=0;
	ObjRealMatrix* product=initRealMatrix(rows_a,cols_b);
	if(isSquare(A) && isSquare(B)){
		if(rows_a<1024){
			for(int i=0;i<rows_a;i++){
				for(int k=0;k<rows_a;k++){
					r=A->data[i][k];
					for(int j=0;j<rows_a;j++){
						product->data[i][j]+=r*B->data[k][j];
						}
					}
				}
		}else{
		//#ifdef OPENBLAS_CONFIG_H
			a=(double*)malloc(rows_a*cols_a*sizeof(double));
			b=(double*)malloc(rows_a*cols_a*sizeof(double));
			c=(double*)malloc(rows_a*cols_a*sizeof(double));
			int count=0;
			memcpy(a,A->data,rows_a*cols_a*sizeof(double));
			memcpy(b,B->data,rows_a*cols_a*sizeof(double));
			cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,rows_a,rows_a,rows_a,1.0,a,rows_a,b,rows_a,0.0,c,rows_a);
			for(int i=0;i<rows_a;i++){
				for(int j=0;j<cols_a;j++){
					product->data[i][j]=c[count];
					count++;
				}
			}
			//memcpy(product->data,c,rows_a*cols_a*sizeof(double));
				free(a);
				free(b);
				free(c);
		//#endif
			}
		}else{
		for(int i=0;i<rows_a;i++){
			for(int k=0;k<rows_b;k++){
				r=A->data[i][k];
				for(int j=0;j<cols_b;j++)
					product->data[i][j]+=r*B->data[k][j];
					}
				}
			}
		return product;
	}
}
ObjRealMatrix* PRM(double scalar,ObjRealMatrix* A){
	int rows=A->rows;
	int cols=A->columns;;
	ObjRealMatrix* result=initRealMatrix(rows,cols);
	for(int i=0;i<rows;i++){
		for(int j=0;j<cols;j++){
			result->data[i][j]=A->data[i][j]*scalar;
		}
	}
}


static void decomposeLU(ObjRealMatrix* matrix){
	int rows=matrix->rows;
	int cols=matrix->columns;
	matrix->factor1=identity_sq(rows);//L
	matrix->factor2=duplicate(matrix);//U
	matrix->perm_vector=ALLOCATE(int,rows);
	for(int i=0;i<rows;i++) matrix->perm_vector[i]=i;
	double max_element=0.0;
	double t1=0;
	double t2=0;
	int pivot_element_index=0;
	for(int k=0;k<cols-1;k++){
		max_element=0.0;
		for(int i=k;i<rows;i++){
			if(fabs(matrix->factor2->data[i][k])>max_element){
				max_element=fabs(matrix->factor2->data[i][k]);
				pivot_element_index=i;
			}
		}
		//if(fabs(max_element)<1e-10){
		//	fprintf(stderr,"The matrix is singular\n");
			//return;
		//}
		t1=matrix->perm_vector[k];
		matrix->perm_vector[k]=matrix->perm_vector[pivot_element_index];
		matrix->perm_vector[pivot_element_index]=t1;
		for(int i=0;i<k;i++){
			t1=matrix->factor1->data[k][i];
			matrix->factor1->data[k][i]=matrix->factor1->data[pivot_element_index][i];
			matrix->factor1->data[pivot_element_index][i]=t1;
		}
		if(k != pivot_element_index) 
			matrix->determinant *= -1;
		for(int i=0;i<cols;i++){
			t1=matrix->factor2->data[k][i];
			matrix->factor2->data[k][i]=matrix->factor2->data[pivot_element_index][i];
			matrix->factor2->data[pivot_element_index][i]=t1;
		}
		for(int i=k+1;i<rows;i++){
			matrix->factor1->data[i][k]=(matrix->factor2->data[i][k]/matrix->factor2->data[k][k]);
			for(int j=k;j<cols;j++)	 
				matrix->factor2->data[i][j] -=(matrix->factor1->data[i][k] * matrix->factor2->data[k][j]);
			}
		}
}

static double Trace(ObjRealMatrix* matrix){
	double trace=0;
	for(int i=0;i<matrix->rows;i++)	trace+=matrix->data[i][i];
	return trace;
}

static double Determinant(ObjRealMatrix* matrix){
	if(matrix->factor1==NULL)	decomposeLU(matrix);
	double determinant=matrix->determinant;
	for(int i=0;i<matrix->rows;i++)	determinant=determinant*(matrix->factor2->data[i][i]);
	return determinant;
}

static ObjRealMatrix* forwardSubstitution(ObjRealMatrix* A,ObjRealMatrix* b){
	/*if(A->factor1==NULL)
		decomposeLU(A);*/
	int row_count=A->rows;
	int cols=A->columns;
	ObjRealMatrix* x=NULL;
	push(OBJ_VAL(A));
	push(OBJ_VAL(x));
	x=initRealMatrix(row_count,1);
	for(int i=0;i<row_count;i++){
		x->data[i][0]=b->data[i][0];
		for(int j=0;j<i;j++)
			x->data[i][0]-=(A->data[i][j]*x->data[j][0]);
		x->data[i][0] /= A->data[i][i];
	}
	pop();
	pop();
	return x;
}

static ObjRealMatrix* backwardSubstitution(ObjRealMatrix* A,ObjRealMatrix* b){
	/*if(A->factor1==NULL)
		decomposeLU(A);*/
	int rows=A->rows;
	int cols=A->columns;
	ObjRealMatrix* x=initRealMatrix(rows,1);
	for(int i=rows-1;i>-1;i--){
		x->data[i][0]=b->data[i][0];
		for(int j=rows-1;j>i;j--)
			x->data[i][0]-=(A->data[i][j]*x->data[j][0]);
		x->data[i][0] /=A->data[i][i];
	}
	return x;
}

static ObjRealMatrix* solveWith(ObjRealMatrix* A,ObjRealMatrix* b){
	if(isUpperTriangular(A)) return backwardSubstitution(A,b);
	else if(isLowerTriangular(A)) return forwardSubstitution(A,b);
	else{
		if(A->factor1==NULL) decomposeLU(A);
		int rows=A->rows;
		ObjRealMatrix* v=initRealMatrix(rows,1); 
		push(OBJ_VAL(v));
		for(int i=0;i<rows;i++)
			v->data[i][0]=b->data[A->perm_vector[i]][0];
		pop();
		ObjRealMatrix* y=forwardSubstitution(A->factor1,v);
		ObjRealMatrix* x=backwardSubstitution(A->factor2,y);
		return x;
	}
}

static ObjRealMatrix* inverse(ObjRealMatrix* A){
   int order = A->rows;
   ObjRealMatrix* inv = initRealMatrix(order,order);
   ObjRealMatrix* Ei=NULL;
   ObjRealMatrix* col =	NULL;
   push(OBJ_VAL(inv));
   push(OBJ_VAL(Ei));
   push(OBJ_VAL(col));
  for(int i = 0;i<order; i++)
   {
     Ei= initRealMatrix(order, 1);
     Ei->data[i][0] = 1.0;
    	col=solveWith(A,Ei);
     for(int j=0;j<order;j++)
    		inv->data[j][i]=col->data[j][0];
   }
   pop();
   pop();
   pop();
   return inv;
}

Value lox_native_identity(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to mat_id()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* id=initRealMatrix(order,order);
	for(int i=0;i<order;i++){
		id->data[i][i]=1.0;
	}
	return OBJ_VAL(id);
}

Value lox_native_null(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to mat_id()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* id=initRealMatrix(order,order);
	return OBJ_VAL(id);
}

Value lox_native_randMatrix(int argCount,Value* args){
	if(argCount<1 && argCount>2)
		Error("Wrong number of arguments to rand_matrix()");
	ObjRealMatrix* randm=NULL;
	if(argCount==1){
		int order=AS_NUMBER(args[0]);
		randm=initRealMatrix(order,order);
		for(int i=0;i<order;i++){
			for(int j=0;j<order;j++)
				randm->data[i][j]=(double)rand()/RAND_MAX;
	}
	}else if(argCount==2){
		int rows=AS_NUMBER(args[0]);
		int cols=AS_NUMBER(args[1]);
		randm=initRealMatrix(rows,cols);
		for(int i=0;i<rows;i++){
			for(int j=0;j<cols;j++)
				randm->data[i][j]=(double)rand()/((double)RAND_MAX);
		}
	}
	return OBJ_VAL(randm);
}

Value lox_native_randUpper(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to rand_upper()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* randm=initRealMatrix(order,order);
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
			if(i<=j)
				randm->data[i][j]=(double)rand()/((double)RAND_MAX);
		}
	}
	return OBJ_VAL(randm);
}

Value lox_native_randLower(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to rand_lower()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* randm=initRealMatrix(order,order);
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
			if(i>=j)
				randm->data[i][j]=(double)rand()/((double)RAND_MAX);
		}
	}
	return OBJ_VAL(randm);
}

Value lox_native_randTriDiag(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to rand_tri()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* randm=initRealMatrix(order,order);
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
			if(i==j+1) randm->data[i][j]=(double)rand()/((double)RAND_MAX);
			else if(i==j) randm->data[i][j]=(double)rand()/((double)RAND_MAX);
			else if(j==i+1) randm->data[i][j]=(double)rand()/((double)RAND_MAX);
		}
	}
	return OBJ_VAL(randm);
}

Value lox_native_randDiag(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to rand_diag()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* randm=initRealMatrix(order,order);
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
			if(i==j)
				randm->data[i][j]=(double)rand()/((double)RAND_MAX);
		}
	}
	return OBJ_VAL(randm);
}

Value lox_native_randSym(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to rand_sym()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* randm=initRealMatrix(order,order);
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
				randm->data[i][j]=randm->data[j][i]=(double)rand()/((double)RAND_MAX);
		}
	}
	return OBJ_VAL(randm);
}

Value lox_native_randSkew(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to rand_skew()");
	int order=AS_NUMBER(args[0]);
	ObjRealMatrix* randm=initRealMatrix(order,order);
	for(int i=0;i<order;i++){
		for(int j=0;j<order;j++){
				randm->data[i][j]=(double)rand()/((double)RAND_MAX);
				randm->data[j][i]=-randm->data[i][j];
		}
	}
	return OBJ_VAL(randm);
}

Value lox_native_getTranspose(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to trn()");
	ObjRealMatrix* op=AS_REAL_MATRIX(args[0]);
	int rows=op->rows;
	int cols=op->columns;
	ObjRealMatrix* trn=initRealMatrix(cols,rows);
	for(int i=0;i<rows;i++){
		for(int j=0;j<cols;j++){
			trn->data[j][i]=op->data[i][j];
			}
	}
	return OBJ_VAL(trn);
}

Value lox_native_getTrace(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to trace()");
	return NUMBER_VAL(Trace(AS_REAL_MATRIX(args[0])));
}

Value lox_native_getDeterminant(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to determinant()");
	return NUMBER_VAL(Determinant(AS_REAL_MATRIX(args[0])));
}

Value lox_native_decomposeLUP(int argCount,Value* args){
	ObjRealMatrix* mat=AS_REAL_MATRIX(args[0]);
	ObjRealMatrix* perm=initRealMatrix(mat->rows,mat->columns);
	decomposeLU(mat);
	int rows=mat->rows;
	int cols=mat->columns;
	for(int i=0;i<rows;i++){
		for(int j=0;j<cols;j++){
			perm->data[i][mat->perm_vector[i]]=1;
		}
	}
	ObjList* values=newList();
	appendToList(values,OBJ_VAL(mat->factor1));
	appendToList(values,OBJ_VAL(mat->factor2));
	appendToList(values,OBJ_VAL(perm));
	return OBJ_VAL(values);
}

Value lox_native_solveWith(int argCount,Value* args){
	SET_ERROR_ARGS(2,"Wrong number of arguments to solve_with()");
	ObjRealMatrix* A=AS_REAL_MATRIX(args[0]);
	ObjRealMatrix* b=AS_REAL_MATRIX(args[1]);
	SET_ERROR_COND(!isSquare(A),"Required square matrix for solve_with()");
	SET_ERROR_COND(A->rows!=b->rows,"Number of rows of both matrices should match");
	return OBJ_VAL(solveWith(A,b));
}

Value lox_native_get_inverse(int argCount,Value* args){
	SET_ERROR_ARGS(1,"Wrong number of arguments to mat_inv()");
	ObjRealMatrix* A=AS_REAL_MATRIX(args[0]);
	if(!isSquare(A)){
		Error("Required square matrix for mat_inv()");
		return NIL_VAL;
	}else{
		return OBJ_VAL(inverse(A));
	}
}
