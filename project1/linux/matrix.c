
#include <string.h>
#include <math.h>

#include <stdlib.h>
#include <stdio.h>

#include "matrix.h"



int main(int argc, char*argv[]){

	long**A;

	long**ans;
	long**B;
	int cmd;
	int row;
	int col;
	int len;
	long ansval;
	char ch;
	
	if(argc<5)
	{
		printf("you had %d arguments this prorgam needs 4 integer arguments cmd row col  and string\n", argc);
		return 1;
	}
	
	row=atoi(argv[2]);
	col=atoi(argv[3]);
	cmd=atoi(argv[1]);

	printf("%d\n",cmd);

	if(cmd <=1)
	{
		len=strlen(argv[4]);
	    B=parseStringToMatrix(argv[4]+(len/2),row,  col);
		argv[4][len/2]=0;
		//printf("%s\n", argv[4]);
		 A=parseStringToMatrix(argv[4],row,  col);
	}
	else
		A=parseStringToMatrix(argv[4],row,  col);
	if(A==NULL)
	{
		printf("bad string\n");
		return 1;
	}

	switch(cmd)
	{
		case 0:
			  ans=matrixAdd(A, B, row,  col);
			  break;
		case 1:
			ans=matrixSub(A, B, row,  col);
			break;
		case 2:
			ansval=matrixDet(A,row);
	}

	
	if(cmd<=1)
	{
		printf(" The inputs were \n");
		matrixPrint(A,row,col);
		matrixPrint(B,row,col);
		printf(" The output is \n");
		matrixPrint(ans,row,col);
		printf("....\n");
		matrixFree(A,row);
		matrixFree(B,row);
		matrixFree(ans,row);
	}
	else
	{
		printf(" The input is \n");
		matrixPrint(A,row,col);
		printf(" The determinant is %ld \n\n", ansval);
		matrixFree(A,row);
	}




    return 0;

}
