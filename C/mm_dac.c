/**
 * Copyright (c) 2018 I-Ting Angelina Lee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 **/

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#ifdef INSTRUMENT_RTS
#include <cilk/common.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "getoptions.h"
#include "ktiming.h"
#include "papi.h"

#ifndef RAND_MAX
#define RAND_MAX 32767
#endif

#define REAL double 

#define EPSILON (1.0E-6)

#define GRAINSIZE 32

	unsigned long rand_nxt = 0;

int cilk_rand(void) {
	int result;
	rand_nxt = rand_nxt * 1103515245 + 12345;
	result = (rand_nxt >> 16) % ((unsigned int)RAND_MAX + 1);
	return result;
}

/*
 * Naive sequential algorithm, for comparison purposes
 */
void matrixmul(REAL* C, REAL* A, REAL* B, int n) {

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			REAL s = (REAL)0;
			for (int k = 0; k < n; ++k) {
				s += A[i * n + k] * B[k * n + j];
			}
			C[i * n + j] = s;
		}
	}
}

void matrixmul_sq(REAL* C, REAL* A, REAL* B, int n, int orig_n) {
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			for (int k = 0; k < n; ++k) {
				C[i * orig_n + j] += A[i * n + k] * B[j * n + k];
			}
		}
	}
}

/*
 * Compare two matrices.  Print an error message if they differ by
 * more than EPSILON.
 * return 0 if no error, and return non-zero if error.
 */
static int compare_matrix(REAL* A, REAL* B, int n) {

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			// compute the relative error c 
			REAL c = A[n * i + j] - B[n * i + j];
			if (c < 0.0) c = -c;

			c = c / A[n * i + j];
			if (c > EPSILON) {
				return -1;
			}
		}
	}
	return 0;
}

//init the matrix to all zeros 
void zero(REAL* M, int n) {
	int i;
	for (i = 0; i < n * n; i++) {
		M[i] = 0.0;
	}
}

//init the matrix to random numbers
void init(REAL* M, int n) {
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			M[i * n + j] = (REAL)cilk_rand();
		}
	}
}

//real transpose also
void shuffle_B(REAL* z_B, int n, REAL* B, int orig_n) {
	int index = 0;
	for (int i = 0; i < n; ++i) {
		//ith col
		for (int j = 0; j < n; ++j) {
			//jth row
			z_B[index] = B[j * orig_n + i];
			index++;
		}
	}
}

void shuffle_A(REAL* z_A, int n, REAL* A, int orig_n) {
	int index = 0;
	for (int i = 0; i < n; ++i) {
		//ith row
		for (int j = 0; j < n; ++j) {
			//jth col
			z_A[index] = A[i * orig_n + j];
			index++;
		}
	}
}


//real transpose also
//void transpose_B(REAL* B, int n, int orig_n) {
//
//	for (int i = 0; i < n; ++i) {
//		for (int j = i + 1; j < n; ++j) {
//			int tmp = B[i * orig_n + j];
//			B[i * orig_n + j] = B[j * orig_n + i];
//			B[j * orig_n + i] = tmp;
//		}
//		for (int j = 0; j < i; ++j) {
//			int tmp = B[i * orig_n + j];
//			B[i * orig_n + j] = B[j * orig_n + i];
//			B[j * orig_n + i] = tmp;
//		}
//	}
//	return;
//}

void morton_col(REAL* z_B, int base_n, REAL* B, int n, int orgi_n) { //int level
	if (n <= base_n) {
		shuffle_B(z_B, n, B, orgi_n);
	}
	else {
		//each sub-matrix points to the start of the z pattern
		int size = (n * n) >> 2;
		REAL* B1 = &B[0];
		REAL* B2 = &B[(n * orgi_n) >> 1];
		REAL* B3 = &B[n >> 1];
		REAL* B4 = &B[((n * orgi_n) + n) >> 1];
		REAL* z_B1 = &z_B[0];
		REAL* z_B2 = &z_B[size];
		REAL* z_B3 = &z_B[size << 1];
		REAL* z_B4 = &z_B[size + (size << 1)];
		//recrusively call the sub-matrices for evaluation in parallel
		cilk_spawn morton_col(z_B1, base_n, B1, n / 2, orgi_n);
		cilk_spawn morton_col(z_B2, base_n, B2, n / 2, orgi_n);
		cilk_spawn morton_col(z_B3, base_n, B3, n / 2, orgi_n);
		morton_col(z_B4, base_n, B4, n / 2, orgi_n);
		cilk_sync;
	}
}


void morton_row(REAL* z_A, int base_n, REAL* A, int n, int orgi_n) { //int level
	if (n <= base_n) {
		shuffle_A(z_A, n, A, orgi_n);
	}
	else {
		//each sub-matrix points to the start of the z pattern
		int size = (n * n) >> 2;
		REAL* A1 = &A[0];
		REAL* A2 = &A[n >> 1];
		REAL* A3 = &A[(n * orgi_n) >> 1];
		REAL* A4 = &A[((n * orgi_n) + n) >> 1];
		REAL* z_A1 = &z_A[0];
		REAL* z_A2 = &z_A[size];
		REAL* z_A3 = &z_A[size << 1];
		REAL* z_A4 = &z_A[size + (size << 1)];
		//recrusively call the sub-matrices for evaluation in parallel
		cilk_spawn morton_row(z_A1, base_n, A1, n / 2, orgi_n);
		cilk_spawn morton_row(z_A2, base_n, A2, n / 2, orgi_n);
		cilk_spawn morton_row(z_A3, base_n, A3, n / 2, orgi_n);
		morton_row(z_A4, base_n, A4, n / 2, orgi_n);
		//cilk_sync;
	}
}


//void mat_mul_par(REAL* A, REAL* B, REAL* C, int n, int orig_n) {
//	
//
//	if (n <= GRAINSIZE) {
//		//*C += *A * *B;
//		//transpose(B, n, orig_n);
//		matrixmul_sq(C, A, B, n, orig_n);
//		return;
//	}
//	//partition each matrix into 4 sub matrices
//	//each sub-matrix points to the start of the z pattern
//	int size = n * n;
//	REAL* A1 = &A[0];
//	REAL* A2 = &A[size >> 2];
//	REAL* A3 = &A[size >> 1];
//	REAL* A4 = &A[(size >> 1) + (size >> 2)];
//
//	REAL* B1 = &B[0];
//	REAL* B3 = &B[size >> 2];
//	REAL* B2 = &B[size >> 1];
//	REAL* B4 = &B[(size >> 1) + (size >> 2)];
//
//	REAL* C1 = &C[0];
//	REAL* C2 = &C[n >> 1];
//	REAL* C3 = &C[(n * orig_n) >> 1];
//	REAL* C4 = &C[((n * orig_n) + n) >> 1];
//	//recrusively call the sub-matrices for evaluation in parallel
//	cilk_spawn mat_mul_par(A1, B1, C1, n >> 1, orig_n);
//	cilk_spawn mat_mul_par(A1, B2, C2, n >> 1, orig_n);
//	cilk_spawn mat_mul_par(A3, B1, C3, n >> 1, orig_n);
//	mat_mul_par(A3, B2, C4, n >> 1, orig_n);
//	cilk_sync; //wait here for first round to finish
//	cilk_spawn mat_mul_par(A2, B3, C1, n >> 1, orig_n);
//	cilk_spawn mat_mul_par(A2, B4, C2, n >> 1, orig_n);
//	cilk_spawn mat_mul_par(A4, B3, C3, n >> 1, orig_n);
//	mat_mul_par(A4, B4, C4, n >> 1, orig_n);
//	cilk_sync; //wait here for all second round to finish
//}

void mat_mul_par(REAL* A, REAL* B, REAL* C, int n, int orig_n) {


	if (n <= GRAINSIZE) {
		//*C += *A * *B;
		//transpose(B, n, orig_n);
		matrixmul_sq(C, A, B, n, orig_n);
		return;
	}
	//partition each matrix into 4 sub matrices
	//each sub-matrix points to the start of the z pattern
	int size = n * n;
	REAL* A1 = &A[0];
	REAL* A2 = &A[size >> 2];
	REAL* A3 = &A[size >> 1];
	REAL* A4 = &A[(size >> 1) + (size >> 2)];

	REAL* B1 = &B[0];
	REAL* B3 = &B[size >> 2];
	REAL* B2 = &B[size >> 1];
	REAL* B4 = &B[(size >> 1) + (size >> 2)];
	// C1 = (A1 + A4)(B1 + B4) + A4(B2 - B1) - (A1 + A2)B4 + (A2 - A4)(B2 + B4)
	// C2 = A1(B3 - B4) + (A1 + A2)B4
	// C3 = (A3 + A4)B1 + A4(B2 - B1)
	// C4 = (A1 + A4)(B1 + B4) - (A3 + A4)B1 + A1(B3 - B4) + (A3 - A1)(B2 + B4)
	REAL* C1 = &C[0];
	REAL* C2 = &C[n >> 1];
	REAL* C3 = &C[(n * orig_n) >> 1];
	REAL* C4 = &C[((n * orig_n) + n) >> 1];
	//recrusively call the sub-matrices for evaluation in parallel
	cilk_spawn mat_mul_par(A1, B1, C1, n >> 1, orig_n);
	cilk_spawn mat_mul_par(A1, B2, C2, n >> 1, orig_n);
	cilk_spawn mat_mul_par(A3, B1, C3, n >> 1, orig_n);
	mat_mul_par(A3, B2, C4, n >> 1, orig_n);
	cilk_sync; //wait here for first round to finish
	cilk_spawn mat_mul_par(A2, B3, C1, n >> 1, orig_n);
	cilk_spawn mat_mul_par(A2, B4, C2, n >> 1, orig_n);
	cilk_spawn mat_mul_par(A4, B3, C3, n >> 1, orig_n);
	mat_mul_par(A4, B4, C4, n >> 1, orig_n);
	cilk_sync; //wait here for all second round to finish
}

const char* specifiers[] = { "-n", "-c", "-h", 0 };
int opt_types[] = { INTARG, BOOLARG, BOOLARG, 0 };
int usage(void) {
	fprintf(stderr,
		"\nUsage: mm_dac [-n #] [-c]\n\n"
		"Multiplies two randomly generated n x n matrices. To check for\n"
		"correctness use -c\n");
	return 1;
}
int main(int argc, char* argv[]) {
	int n = 2048;
	int verify = 0;
	int help = 0;
	get_options(argc, argv, specifiers, opt_types, &n, &verify, &help);
	if (help || argc == 1) return usage();
	REAL* A, * B, * C;
	A = (REAL*)malloc(n * n * sizeof(REAL)); //source matrix
	B = (REAL*)malloc(n * n * sizeof(REAL)); //source matrix
	C = (REAL*)malloc(n * n * sizeof(REAL)); //result matrix
	init(A, n);
	init(B, n);
	zero(C, n);
	//z_layout(A, B);
	REAL* z_A = (REAL*)malloc(n * n * sizeof(REAL));
	REAL* z_B = (REAL*)malloc(n * n * sizeof(REAL));
	zero(z_A, n);
	zero(z_B, n);
	//morton_row(z_A, GRAINSIZE, A, n, n);
	//morton_col(z_B, GRAINSIZE, B, n, n);
#ifdef INSTRUMENT_RTS
	__cilkrts_init();
	__cilkrts_reset_timing();
#endif
	clockmark_t begin_rm = ktiming_getmark();
	morton_row(z_A, GRAINSIZE, A, n, n);
	morton_col(z_B, GRAINSIZE, B, n, n);
	mat_mul_par(z_A, z_B, C, n, n);
	clockmark_t end_rm = ktiming_getmark();
	//mat_mul_par(z_A, z_B, C, n, n);
#ifdef INSTRUMENT_RTS
	__cilkrts_accum_timing();
#endif
	printf("Elapsed time in seconds: %f\n", ktiming_diff_sec(&begin_rm, &end_rm));
	if (verify) {
		printf("Checking results ... \n");
		REAL* C2 = (REAL*)malloc(n * n * sizeof(REAL));
		matrixmul(C2, A, B, n);
		verify = compare_matrix(C, C2, n);
		free(C2);
	}
	if (verify) {
		printf("WRONG RESULT!\n");
	}
	else {
		printf("\nCilk Example: matrix multiplication\n");
		printf("Options: n = %d\n\n", n);
	}
	//clean up memory
	delete[] A;
	delete[] B;
	delete[] C;
	return 0;
}

