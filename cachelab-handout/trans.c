/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void deal16(int M, int N, int* pA, int* pB)
{
    int i;
    int t0, t1, t2, t3, t4, t5, t6, t7;
    for (i = 0; i < 16; ++i)
    {
        t0 = pA[i * M], t1 = pA[i * M + 1], t2 = pA[i * M + 2], t3 = pA[i * M + 3],
        t4 = pA[i * M + 4], t5 = pA[i * M + 5], t6 = pA[i * M + 6], t7 = pA[i * M + 7];
        pB[i] = t0, pB[N + i] = t1, pB[2 * N + i] = t2, pB[3 * N + i] = t3,
        pB[4 * N + i] = t4, pB[5 * N + i] = t5, pB[6 * N + i] = t6, pB[7 * N + i] = t7;
    }
    for (i = 0; i < 16; ++i)
    {
        t0 = pA[i * M + 8], t1 = pA[i * M + 9], t2 = pA[i * M + 10], t3 = pA[i * M + 11],
        t4 = pA[i * M + 12], t5 = pA[i * M + 13], t6 = pA[i * M + 14], t7 = pA[i * M + 15];
        pB[8 * N + i] = t0, pB[9 * N + i] = t1, pB[10 * N + i] = t2, pB[11 * N + i] = t3,
        pB[12 * N + i] = t4, pB[13 * N + i] = t5, pB[14 * N + i] = t6, pB[15 * N + i] = t7;
    }
}

void deal8_64(int M, int N, int* pA, int* pB)
{
    int i;
    int t0, t1, t2, t3, t4, t5, t6, t7;
    for (i = 0; i < 4; ++i)
    {
        t0 = pA[i * M], t1 = pA[i * M + 1], t2 = pA[i * M + 2], t3 = pA[i * M + 3],
        t4 = pA[i * M + 4], t5 = pA[i * M + 5], t6 = pA[i * M + 6], t7 = pA[i * M + 7];
        pB[i] = t0, pB[i + 4] = t4, pB[N + i] = t1, pB[N + i + 4] = t5,
        pB[2 * N + i] = t2, pB[2 * N + i + 4] = t6, pB[3 * N + i] = t3, pB[3 * N + i + 4] = t7;
    }
    for (i = 0; i < 4; ++i)
    {
        t0 = pA[4 * M + i], t1 = pA[5 * M + i], t2 = pA[6 * M + i], t3 = pA[7 * M + i],
        t4 = pB[i * N + 4], t5 = pB[i * N + 5], t6 = pB[i * N + 6], t7 = pB[i * N + 7];
        pB[i * N + 4] = t0, pB[i * N + 5] = t1, pB[i * N + 6] = t2, pB[i * N + 7] = t3;
        pB[(i + 4) * N] = t4, pB[(i + 4) * N + 1] = t5, pB[(i + 4) * N + 2] = t6, pB[(i + 4) * N + 3] = t7;
    }
    for (i = 4; i < 8; ++i)
    {
        t4 = pA[i * M + 4], t5 = pA[i * M + 5], t6 = pA[i * M + 6], t7 = pA[i * M + 7];
        pB[4 * N + i] = t4, pB[5 * N + i] = t5, pB[6 * N + i] = t6, pB[7 * N + i] = t7; 
    }
}

void deal16_64(int M, int N, int* pA, int* pB)
{
    deal8_64(M, N, pA, pB);
    deal8_64(M, N, pA + 8, pB + 8 * N);
    deal8_64(M, N, pA + 8 * M, pB + 8);
    deal8_64(M, N, pA + 8 * M + 8, pB + 8 * N + 8);
}

void deal3x61(int M, int N, int* pA, int* pB)
{
    int i;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8;
    for (i = 0; i < 60; i += 3)
    {
        t0 = pA[i], t1 = pA[i + 1], t2 = pA[i + 2],
        t3 = pA[M + i], t4 = pA[M + i + 1], t5 = pA[M + i + 2],
        t6 = pA[2 * M + i], t7 = pA[2 * M + i + 1], t8 = pA[2 * M + i + 2];
        pB[i * N] = t0, pB[i * N + 1] = t3, pB[i * N + 2] = t6,
        pB[(i + 1) * N] = t1, pB[(i + 1) * N + 1] = t4, pB[(i + 1) * N + 2] = t7,
        pB[(i + 2) * N] = t2, pB[(i + 2) * N + 1] = t5, pB[(i + 2) * N + 2] = t8;
    }
    pB[60 * N] = pA[60], pB[60 * N + 1] = pA[M + 60], pB[60 * N + 2] = pA[2 * M + 60];
}

void deal64x5(int M, int N, int* pA, int* pB)
{
    int i;
    int t0, t1, t2, t3, t4;
    for (i = 0; i < 64; ++i)
    {
        t0 = pA[i * M], t1 = pA[i * M + 1], t2 = pA[i * M + 2], t3 = pA[i * M + 3], t4 = pA[i * M + 4];
        pB[i] = t0, pB[N + i] = t1, pB[2 * N + i] = t2, pB[3 * N + i] = t3, pB[4 * N + i] = t4;
    }
}

void deal64x13(int M, int N, int* pA, int* pB)
{
    deal8_64(M, N, pA, pB);
    deal8_64(M, N, pA + 8 * M, pB + 8);
    deal8_64(M, N, pA + 16 * M, pB + 16);
    deal8_64(M, N, pA + 24 * M, pB + 24);
    deal8_64(M, N, pA + 32 * M, pB + 32);
    deal8_64(M, N, pA + 40 * M, pB + 40);
    deal8_64(M, N, pA + 48 * M, pB + 48);
    deal8_64(M, N, pA + 56 * M, pB + 56);
    deal64x5(M, N, pA + 8, pB + 8 * N);
}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 16 && N == 16)
    {
        deal16_64(M, N, *A, *B);
    }
    if (M == 32 && N == 32)
    {
        deal16(M, N, *A, *B);
        deal16(M, N, *A + 16, *B + 16 * N);
        deal16(M, N, *A + 16 * M, *B + 16);
        deal16(M, N, *A + 16 * M + 16, *B + 16 * N + 16);
    }
    else if (M == 64 && N == 64)
    {
        deal16_64(M, N, *A, *B);
        deal16_64(M, N, *A + 16, *B + 16 * N);
        deal16_64(M, N, *A + 32, *B + 32 * N);
        deal16_64(M, N, *A + 48, *B + 48 * N);

        deal16_64(M, N, *A + 16 * M, *B + 16);
        deal16_64(M, N, *A + 16 * M + 16, *B + 16 * N + 16);
        deal16_64(M, N, *A + 16 * M + 32, *B + 32 * N + 16);
        deal16_64(M, N, *A + 16 * M + 48, *B + 48 * N + 16);

        deal16_64(M, N, *A + 32 * M, *B + 32);
        deal16_64(M, N, *A + 32 * M + 16, *B + 16 * N + 32);
        deal16_64(M, N, *A + 32 * M + 32, *B + 32 * N + 32);
        deal16_64(M, N, *A + 32 * M + 48, *B + 48 * N + 32);

        deal16_64(M, N, *A + 48 * M, *B + 48);
        deal16_64(M, N, *A + 48 * M + 16, *B + 16 * N + 48);
        deal16_64(M, N, *A + 48 * M + 32, *B + 32 * N + 48);
        deal16_64(M, N, *A + 48 * M + 48, *B + 48 * N + 48);
    }
    else if (M == 61 && N == 67)    // 9.6/10, not good
    {
        deal16_64(M, N, *A, *B);
        deal16_64(M, N, *A + 16, *B + 16 * N);
        deal16_64(M, N, *A + 32, *B + 32 * N);

        deal16_64(M, N, *A + 16 * M, *B + 16);
        deal16_64(M, N, *A + 16 * M + 16, *B + 16 * N + 16);
        deal16_64(M, N, *A + 16 * M + 32, *B + 32 * N + 16);

        deal16_64(M, N, *A + 32 * M, *B + 32);
        deal16_64(M, N, *A + 32 * M + 16, *B + 16 * N + 32);
        deal16_64(M, N, *A + 32 * M + 32, *B + 32 * N + 32);

        deal16_64(M, N, *A + 48 * M, *B + 48);
        deal16_64(M, N, *A + 48 * M + 16, *B + 16 * N + 48);
        deal16_64(M, N, *A + 48 * M + 32, *B + 32 * N + 48);

        deal3x61(M, N, *A + 64 * M, *B + 64);
        deal64x13(M, N, *A + 48, *B + 48 * N);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

