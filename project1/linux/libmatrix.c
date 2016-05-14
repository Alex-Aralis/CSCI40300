
#include <string.h>
#include <math.h>

#include <stdlib.h>
#include <stdio.h>

#include "matrix.h"

long **matrixMalloc(int row, int col)
{
	long **M;
	int i,j;

	M=(long **)malloc(row*sizeof(long*));

	for(i=0;i<row; i++)
		M[i]=(long *)malloc(col*sizeof(long));

	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
			M[i][j]=0;


	return M;

}
void matrixInit(long **M, FILE *fr, int row, int col)
{
	int ch;
	int i,j;
	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
					M[i][j]=fgetc(fr)-'0';
			


	


}
void matrixFree(long **M, int row)
{
	int i;
	for(i=0;i<row;i++)
		free(M[i]);
	free(M);



}
long **matrixAdd(long**A, long**B, int row, int col)
{
	long **C;
	int i,j;
	
	C=matrixMalloc(row, col);
	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
			C[i][j]=A[i][j]+B[i][j];
	return C;
}

void matrixPrint(long **M, int row, int col)
{
	int i,j;
	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
		{
			if(j==0)
				printf("\n[");
			printf("%-5ld",M[i][j]);
			if(j==col-1)
				printf("]");

		}
		printf("\n");
}
//must be square
long matrixDet(long **A, int row)
{
	long ** temp;
	long detval=0;
	int i;
	long sign=1;
	int j,k;
	

	if(row==2)
		return A[0][0]*A[1][1]-A[1][0]*A[0][1];

	temp=matrixMalloc(row-1, row-1);
	for(i=0;i<row;i++)
	{ 
		for(j=0;j<row-1;j++)
			for(k=0;k<row-1;k++)
			{
				
				temp[j][k]= j<i? A[j][k+1]: A[j+1][k+1];

			}
			detval+=sign*A[i][0]*matrixDet(temp, row-1);
			sign =sign*(-1);



	}
	matrixFree(temp, row-1);
	return detval;




}

long **matrixSub(long**A, long**B, int row, int col)
{
	long **C;
	int i,j;
	
	C=matrixMalloc(row, col);
	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
			C[i][j]=A[i][j]-B[i][j];
	return C;
} 

long **matrixMax(long**A, long**B, int row, int col)
{
	long **C;
	int i,j;
	
	C=matrixMalloc(row, col);
	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
			C[i][j]=A[i][j]>=B[i][j]? A[i][j]:B[i][j];
	return C;
}

//does not validate memory 
long **parseStringToMatrix(char * str,int row, int col)
{
	char*ptr;
	int i,j;
	int len=strlen(str);
	long **A;
	
		
	if( row*col !=len)
			return NULL;

	A=matrixMalloc( row, col);

	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
		{
			if(*str=='\0')
				return NULL;

			A[i][j]=*str-'0';
			str++;
		//	printf("%s   %d\n", str, A[i][j]);
			
		}
		return A;

}
