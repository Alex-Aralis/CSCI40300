#ifndef MATRIX_HEADER
#define MATRIX_HEADER

long **matrixMalloc(int row, int col);
void matrixInit(long **M, FILE *fr, int row, int col);
void matrixFree(long **, int row);
long **matrixAdd(long**, long**, int row, int col);
void matrixPrint(long **M, int row, int col);
long matrixDet(long **A, int row);
long **matrixSub(long **A, long **B, int row, int col);
long **matrixMax(long **A, long **B, int row, int col);
long **parseStringToMatrix(char* str, int row, int col);

#endif
