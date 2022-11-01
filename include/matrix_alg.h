#ifndef clox_native_matrix_alg
#define clox_native_matrix alg

#include "value.h"
#include "object.h"


Value lox_native_gauss_seidel(int,Value*);
Value lox_native_gauss_jacobi(int,Value*);

//The following functions return a list of two factors
Value lox_native_decompose_qr_house(int,Value*);
Value lox_native_decompose_qr_gram(int,Value*);
Value lox_native_decompose_qr_givens(int,Value*);

//Returns a list of eigen values
Value lox_native_jacobi_eig(int,Value*);
//Returns a list of matrices such that A=QDQt
Value lox_native_jacobi_eig_de(int,Value*);

Value lox_native_hessenberg(int,Value*);

//returns a list of eigen values of any matrix
Value lox_native_gen_qr_eig(int,Value*);
//returns the general eigen decomposition of a real matrix
Value lox_native_gen_qr_de(int,Value*);
//returns the singular value decomposition
Value lox_native_gen_svd(int,Value*);
//returns the cholesky decomposition of a matrix
Value lox_native_gen_cholesky(int,Value*);



#endif
