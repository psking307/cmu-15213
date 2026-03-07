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

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

void transpose32_32(int A[32][32], int B[32][32]);
void transpose64_64(int A[64][64], int B[64][64]);
void transpose67_61(int A[67][61], int B[61][67]);
 
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (N == 32 && M == 32) transpose32_32(A, B);
    if (N == 64 && M == 64) transpose64_64(A, B);
    if (M == 61 && N == 67) transpose67_61(A, B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

void transpose32_32(int A[32][32], int B[32][32]){
    int i, j, k;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    for(i = 0; i < 32; i += 8){
        for(j = 0; j < 32; j += 8){
            for(k = i; k < i + 8; k++){
                tmp1 = A[k][j];
                tmp2 = A[k][j + 1];
                tmp3 = A[k][j + 2];
                tmp4 = A[k][j + 3];
                tmp5 = A[k][j + 4];
                tmp6 = A[k][j + 5];
                tmp7 = A[k][j + 6];
                tmp8 = A[k][j + 7];
                B[j][k] = tmp1;
                B[j + 1][k] = tmp2;
                B[j + 2][k] = tmp3;
                B[j + 3][k] = tmp4;
                B[j + 4][k] = tmp5;
                B[j + 5][k] = tmp6;
                B[j + 6][k] = tmp7;
                B[j + 7][k] = tmp8;
            }
        }
    }
}

void transpose64_64(int A[64][64], int B[64][64]){
    int i, j, k;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    for(i = 0; i < 64; i += 8){
        for(j = 0; j < 64; j += 8){
            for(k = 0; k < 4; k++, i++){
                tmp1 = A[i][j];
                tmp2 = A[i][j + 1];
                tmp3 = A[i][j + 2];
                tmp4 = A[i][j + 3];
                tmp5 = A[i][j + 4];
                tmp6 = A[i][j + 5];
                tmp7 = A[i][j + 6];
                tmp8 = A[i][j + 7];
                B[j][i] = tmp1;
                B[j + 1][i] = tmp2;
                B[j + 2][i] = tmp3;
                B[j + 3][i] = tmp4;
                B[j][i + 4] = tmp5;
                B[j + 1][i + 4] = tmp6;
                B[j + 2][i + 4] = tmp7;
                B[j + 3][i + 4] = tmp8;
            }
            i -= 4;
            for(k = 0; k < 4; k++, j++){
                tmp1 = A[i + 4][j];
                tmp2 = A[i + 5][j];
                tmp3 = A[i + 6][j];
                tmp4 = A[i + 7][j];
                tmp5 = B[j][i + 4];
                tmp6 = B[j][i + 5];
                tmp7 = B[j][i + 6];
                tmp8 = B[j][i + 7];
                B[j][i + 4] = tmp1;
                B[j][i + 5] = tmp2;
                B[j][i + 6] = tmp3;
                B[j][i + 7] = tmp4;
                B[j + 4][i] = tmp5;
                B[j + 4][i + 1] = tmp6;
                B[j + 4][i + 2] = tmp7;
                B[j + 4][i + 3] = tmp8;
            }
            i += 4;
            for(k = 0; k < 4; k++, i++){
                tmp1 = A[i][j];
                tmp2 = A[i][j + 1];
                tmp3 = A[i][j + 2];
                tmp4 = A[i][j + 3];
                B[j][i] = tmp1;
                B[j + 1][i] = tmp2;
                B[j + 2][i] = tmp3;
                B[j + 3][i] = tmp4;
            }
            i -= 8;
            j -= 4;
        }
    }
}

void transpose67_61(int A[67][61], int B[61][67]){
    int i, j, k;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    for(i = 0; i < 67; i += 8){
        for(j = 0; j < 61; j += 8){
            for(k = i; k < i + 8 && k < 67; k++){
                if (j + 0 < 61) tmp1 = A[k][j];
                if (j + 1 < 61) tmp2 = A[k][j + 1];
                if (j + 2 < 61) tmp3 = A[k][j + 2];
                if (j + 3 < 61) tmp4 = A[k][j + 3];
                if (j + 4 < 61) tmp5 = A[k][j + 4];
                if (j + 5 < 61) tmp6 = A[k][j + 5];
                if (j + 6 < 61) tmp7 = A[k][j + 6];
                if (j + 7 < 61) tmp8 = A[k][j + 7];
                if (j + 0 < 61) B[j][k] = tmp1;
                if (j + 1 < 61) B[j + 1][k] = tmp2;
                if (j + 2 < 61) B[j + 2][k] = tmp3;
                if (j + 3 < 61) B[j + 3][k] = tmp4;
                if (j + 4 < 61) B[j + 4][k] = tmp5;
                if (j + 5 < 61) B[j + 5][k] = tmp6;
                if (j + 6 < 61) B[j + 6][k] = tmp7;
                if (j + 7 < 61) B[j + 7][k] = tmp8;
            }
        }
    }    
}
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

