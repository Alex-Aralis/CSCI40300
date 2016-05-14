//PREFACE

//this project has gotten a little out of hand
//I had way too much time to work on it
//and kept adding... features?
//Hopefully whoever is grading this
//...likes it?

//I used malloc at one point to pass data into
//the thread instances of there argument structures at one point, 
//but it turns out that was not necessary.  I'm now using 
//a horrible abomination of coding spaghetti.

//execThreadedMatrixMult is esspecially grusome.  My hand was
//somewhat forced by the project constraints however
//in the form of having global varibles A B and C
//that were in the [][] array form, which does not
//mesh well with the ** format used by libmatrix.

//the max procs, matrix size, command mapping, and exec file can 
//be changed with the macros

//changing the matrix size really does work!!! neato right?

//also all the output is smush into a nearly unreadable
//mess because we couldnt lock the file, but that was inevitable.

//p.s.
//Blocks are not used with "for" or "if" statements when redundant 
//because I am a pedantic idiot
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

//the header file for the libmatrix library consisting of the
//non-main functions in matrix.c
#include "matrix.h"


//size of matrix to be created from stuff file
#define MATRIX_SIZE_I 4
#define MATRIX_SIZE_S "4"

//location and name of the matrix executable
//used for computation of add, subtract, and determinate
#define MATRIX_EXEC "./matrix.out"

//command mapping from infile to matrix operation
#define MATRIX_ADD '0'
#define MATRIX_SUB '1'
#define MATRIX_DET '2'
#define MATRIX_MUL '3'

//max procs that will be used concurrently
#define MAX_PROCS 20

//struct passed to the dot product wrapper function.
struct dot_info{
    long volatile * presult;
    long volatile * X;
    long volatile * Y;
    int len;
};

//the specified function that produces the dot product
void dotProduct(long volatile * presult, long volatile X[], long volatile Y[], int len);

//the entry point for the thread with a pthreads compatible function definitoin
void* dotProductThreadWrapper(void*);

//class to fork matrix commands
//
//returns 0 if an error occured
//returns 1 otherwise
long execMatrixCommand(char cmd, FILE * instream);

//class to create fork that creates and joins 
//threads then print the matrix multiplication 
//calculated.
//
//returns 0 if an error occures
//returns 1 otherwise
long execThreadedMatrixMult(FILE * instream, char cmd, pthread_attr_t attr, pthread_t ths[][MATRIX_SIZE_I], 
    struct dot_info pthsarg[][MATRIX_SIZE_I], volatile long sA[][MATRIX_SIZE_I], volatile long sTransB[][MATRIX_SIZE_I], 
    volatile long sC[][MATRIX_SIZE_I], long sB[][MATRIX_SIZE_I], long volatile ** tA, long volatile ** tB, long volatile ** tC);

//definitions of the shared matrices used by the dot product threads
//instead of sharing B the transposition is used instead
//this is simply for convenience
volatile long A[MATRIX_SIZE_I][MATRIX_SIZE_I];
long B[MATRIX_SIZE_I][MATRIX_SIZE_I];
volatile long C[MATRIX_SIZE_I][MATRIX_SIZE_I];
volatile long transB[MATRIX_SIZE_I][MATRIX_SIZE_I];



//declare the array that holds the references for thread i,j
struct dot_info thsarg[MATRIX_SIZE_I][MATRIX_SIZE_I];

int main(int argc, char* argv[]){

    FILE* stuff;
    stuff = fopen("stuff", "r");
    char cmd;

    //char array used to store the required input for the next operation
    char str[MATRIX_SIZE_I*MATRIX_SIZE_I*2+1];

    //used for iteration 
    long i,j;

    //used for array indexing optimizations 
    long pos;

    //the pthread attributes variable used for all created pthreads
    //attr is allocated only once and reused for all the threads
    //then deallocated once prior to termination of the main thread
    pthread_attr_t attr;

    //the array of thread handles so they can be kept track of
    //computation for array position i, j is done by the thread ths[i][j]
    pthread_t ths[MATRIX_SIZE_I][MATRIX_SIZE_I];

    //used to convert between [][], used in this project
    //to ** used in libmatrix.
    long volatile * tA[MATRIX_SIZE_I];
    long volatile * tB[MATRIX_SIZE_I];
    long volatile * tC[MATRIX_SIZE_I];
   
    //keeping track of the running forks
    long procs = 0;

    //pid
    long pid;

    //allocation of the pthread attributes
    pthread_attr_init(&attr);

    //set tA, tB, and tC such that they convert corretely
    //only needs to be done once because the memory locations of
    //the data in A, B, and C never change. 
    for(i = 0; i < MATRIX_SIZE_I; i++){
        tA[i] = A[i];
        tB[i] = B[i];
        tC[i] = C[i];
    }


    //generate arg for thread ij
    //this can be done outside of the loop because it is input independent.
    for(i=0; i < MATRIX_SIZE_I; i++) for(j=0; j < MATRIX_SIZE_I; j++){
        thsarg[i][j].presult = C[i]+j;
        thsarg[i][j].X = A[i];
        thsarg[i][j].Y = transB[j];
        thsarg[i][j].len = MATRIX_SIZE_I;
    } 
  

    //loop while fgetc has not reached the end of file or encountered an error
    while((cmd = fgetc(stuff)) != EOF){
        pid = -1;
        //switch on char retreived by fgetc
        switch(cmd){

            //matrix addition with matrix.out
            case MATRIX_ADD: 

            //matrix subtraction with matrix.out
            case MATRIX_SUB: 

            //determinate with matrix.out
            case MATRIX_DET: 
                procs += execMatrixCommand(cmd, stuff);
                break;

            //matrix multiplication using pthreads
            case MATRIX_MUL: 
                procs += execThreadedMatrixMult(stuff, cmd, attr, ths, thsarg, A, transB, C, B, tA, tB, tC);
                break;
        }
        
        //clean up a finished child if any (unblocking)
        // s.t. number of zombie procs member O(contant)
        //in theory (not so much in practice)
        if(waitpid(-1, NULL, WNOHANG)) procs--;

        //wait for 1/4 of the procs to finish if max procs is reached
        if(procs >= MAX_PROCS){
            while (procs >= 3*MAX_PROCS/4){
                wait(NULL);
                procs--;
                if(errno == ECHILD){
                    fprintf(stderr, "procs errno missmach!!! \n resetting...\n");
                    procs = 0;
                    break;
                }
            }
        }

    }

    //deallocate attr
    pthread_attr_destroy(&attr);
    
    //wait for all children to finish 
    while(wait(NULL)) if(errno == ECHILD) break;
}

//class to fork matrix commands
long execMatrixCommand(char cmd, FILE * instream){
    pid_t pid = -1;
    long bufsize = -1;

    //decide size of buffer
    switch(cmd){
        case MATRIX_ADD:
        case MATRIX_SUB:
            bufsize = MATRIX_SIZE_I*MATRIX_SIZE_I*2+1;
            break;
        case MATRIX_DET:
            bufsize = MATRIX_SIZE_I*MATRIX_SIZE_I+1;
            break;
        default:
            fprintf(stderr, "-->matrix exec command not found!!! command was %c\n", cmd);
            return 0;
    }
    char str[bufsize];

    if(fgets(str, bufsize, instream) && strlen(str) == bufsize-1 && !(pid = fork())){
        char scmd[2];

        //create string from char cmd
        scmd[0] = cmd;
        scmd[1] = '\0';

        //close input file before fork so child does not have fd associated with it
        fclose(instream);

        execlp(MATRIX_EXEC, MATRIX_EXEC, scmd, MATRIX_SIZE_S, MATRIX_SIZE_S, str, NULL);

        fprintf(stderr, "-->execlp failed!!! command was %c\n", cmd);
        exit(-1);
    }else if(pid > 0){
        return 1;
    }else if(feof(instream)){
        fprintf(stderr, "-->end of instream reached!!! command was %c\n", cmd);
    }else{
        fprintf(stderr, "-->matrix exec failed to fork!!! command was %c\n", cmd);
    }
    return 0;
}


//all this stuff is pass so it can be generic shm not just gross global variables
long execThreadedMatrixMult(FILE * instream, char cmd, pthread_attr_t attr, pthread_t ths[][MATRIX_SIZE_I], 
    struct dot_info pthsarg[][MATRIX_SIZE_I], volatile long sA[][MATRIX_SIZE_I], volatile long sTransB[][MATRIX_SIZE_I], 
    volatile long sC[][MATRIX_SIZE_I], long sB[][MATRIX_SIZE_I], long volatile ** tA, long volatile ** tB, long volatile ** tC){
    
    pid_t pid;
    char str[MATRIX_SIZE_I*MATRIX_SIZE_I*2+1];

    //if the file does not end before requred string length is retrieved then fork to prevent blocking
    if(fgets(str, MATRIX_SIZE_I*MATRIX_SIZE_I*2+1, instream) && strlen(str) == MATRIX_SIZE_I*MATRIX_SIZE_I*2 && !(pid = fork())) {
        long i, j;

        fclose(instream);

        //output formated to imitate matrix.out
        printf("%c\n", cmd);

        //put read str into A and B and transB
        long pos=0;
        for(i = 0; i < MATRIX_SIZE_I; i++) for(j = 0; j < MATRIX_SIZE_I; j++){
            sA[i][j] = str[pos]-48;
            sB[i][j] = str[pos + 16]-48;
            sTransB[j][i] = str[pos + 16]-48;
            pos++;
        }
        

        //spin the thread beginning execution at the wrapper function
        //passing precomputed thsarg
        for(i = 0; i  < MATRIX_SIZE_I; i++) for(j = 0; j < MATRIX_SIZE_I; j++)
            pthread_create(ths[i]+j, &attr, dotProductThreadWrapper, pthsarg[i]+j);

        //print A and B using translations created previously 
        //while waiting for threads to finish
        printf(" The inputs were\n");
        matrixPrint((long **)tA, MATRIX_SIZE_I, MATRIX_SIZE_I);
        matrixPrint((long **)tB, MATRIX_SIZE_I, MATRIX_SIZE_I);

        //let all threads join, assuring the 
        //matrix multiplication is complete
        for(i = 0; i  < MATRIX_SIZE_I; i++) for(j = 0; j < MATRIX_SIZE_I; j++)
            pthread_join(ths[i][j], NULL);
        
        //output product
        printf(" The output is\n") ;
        matrixPrint((long **) tC, MATRIX_SIZE_I, MATRIX_SIZE_I);
        printf("....\n");


        //deallocate attr in main
        pthread_attr_destroy(&attr);
        
        exit(0);
    }else if(pid > 0){
        return 1;
    }else{
        fprintf(stderr, "-->matrix failed to fork!!! command was %c\n", cmd);
        return 0;
    }

}

//the prescribed method in the assignment write-up.
void dotProduct(long volatile * presult, long volatile X[], long volatile Y[], int len){
    *presult = 0;

    //compute dot product for vectors
    for(long i = 0; i < len; i++) *presult += X[i] * Y[i];
}

//entry point for dot product thread execution
void* dotProductThreadWrapper(void* args){
    //call dot product with args, must be cast so compiler knows how to read fields
    dotProduct(((struct dot_info*)args)->presult, ((struct dot_info*)args)->X, ((struct dot_info*)args)->Y, ((struct dot_info*)args)->len);
    
    //terminate thread so that the cleanup stack executes
    pthread_exit(NULL);
}

