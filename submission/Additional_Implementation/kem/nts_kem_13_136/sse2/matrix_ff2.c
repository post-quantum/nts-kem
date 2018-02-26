/**
 *  matrix_ff2.c
 *  NTS-KEM
 *
 *  Parameter: NTS-KEM(13, 136)
 *  Platform: SSE2/SSE4.1
 *
 *  This file is part of the additional implemention of NTS-KEM
 *  submitted as part of NIST Post-Quantum Cryptography
 *  Standardization Process.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix_ff2.h"
#include "m4r.h"
#include "random.h"
#include "keccak.h"
#include "nts_kem_errors.h"

#define M4R_ROW(M, r)                   ((uint8_t *)(M)->v + ((r) * (M)->stride))
#define PQ_EXTRA_ROWS_FOR_FULL_RANK     20
#define RANDOM_MATRIX_SEED_SIZE         512

matrix_ff2* alloc_matrix_ff2(int nrows, int ncols)
{
    matrix_ff2 *M = NULL;
    
    if (nrows > 0 && ncols > 0) {
        M = (matrix_ff2 *)malloc(sizeof(matrix_ff2));
        if (!M)
            return NULL;
        M->nrows = nrows;
        M->ncols = ncols;
        M->nblocks = (ncols + MOD) >> LOG2;
        M->stride = ALIGNMENT * (((M->nblocks * sizeof(packed_t)) + ALIGNMENT - 1) / ALIGNMENT);
#if defined(_WIN32)
        if (!(M->v = _aligned_malloc(M->stride * M->rows, ALIGNMENT)))
#else
        if (0 != posix_memalign((void **)&M->v, ALIGNMENT, M->stride * M->nrows))
#endif
        {
            free(M);
            return NULL;
        }
    }

    return M;
}

matrix_ff2* calloc_matrix_ff2(int nrows, int ncols)
{
    matrix_ff2 *M = alloc_matrix_ff2(nrows, ncols);
    if (!M)
        return NULL;
    zero_matrix_ff2(M);
    
    return M;
}

void free_matrix_ff2(matrix_ff2 *M)
{
    if (M) {
        if (M->v) {
#if defined(_WIN32)
            _aligned_free(M->v);
#else
            free(M->v);
#endif
            M->v = NULL;
        }
        M->nrows = M->ncols = M->nblocks = M->stride = 0;
        free(M);
    }
}

void zero_matrix_ff2(matrix_ff2* M)
{
    memset(M->v, 0, M->stride * M->nrows);
}

matrix_ff2* clone_matrix_ff2(const matrix_ff2* M)
{
    matrix_ff2* R = alloc_matrix_ff2(M->nrows, M->ncols);
    if (!R)
        return NULL;
    
    memcpy(R->v, M->v, M->stride * M->nrows);
    
    return R;
}

int is_equal_matrix_ff2(const matrix_ff2 *A, const matrix_ff2 *B)
{
    if (!A || !B || A->nrows != B->nrows || A->ncols != B->ncols)
        return 0;
    
    return (0 == memcmp(A->v, B->v, A->stride * A->nrows));
}

void column_swap_matrix_ff2(matrix_ff2* M, int32_t a, int32_t b)
{
    int32_t r;
    packed_t va, vb;
    packed_t *row_ptr = (packed_t *)row_ptr_matrix_ff2(M, 0);

    /**
     * Column swapping of a matrix is an expensive operation, 
     * if a and b point to the same column, skip it
     **/
    if (a == b)
        return;
    
    for (r=0; r<M->nrows; r++) {
        va = bit_value(row_ptr, a);
        vb = bit_value(row_ptr, b);
        bit_clear(row_ptr, a);
        bit_set_value(row_ptr, a, vb);
        bit_clear(row_ptr, b);
        bit_set_value(row_ptr, b, va);
        row_ptr += (M->stride/sizeof(packed_t));
    }
}

uint32_t reduce_row_echelon_matrix_ff2(matrix_ff2 *M)
{
    return m4r_rref(M);
}
