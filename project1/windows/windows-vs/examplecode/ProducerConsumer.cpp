#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//number of things to be produced before the program is terminated
#define DATASIZE 2000

//size of buf[]
#define BUFFER_SIZE 50

//Alphabet Cardinality, or the upper bound of the interval of integers 
//to be randomly selected from i.e. r member [0, ALPHA_CARD).
#define ALPHA_CARD 4

//shared memory
int in = 0;
int out = 0;
int buf[BUFFER_SIZE];
FILE* stuff;

//Entry point for the consumer thread.
DWORD WINAPI Consumer(LPVOID Param){
	//buffer for new char
	char newchar;

	//read from buf forever
	while (true) {
		//spin lock if buf empty
		while (in == out);

		//convert buf[out] to ascii
		_itoa_s(buf[out], &newchar, 2, 10);

		//increment out so that the main thread will know where to stop filling the buffer
		out = (out + 1) % BUFFER_SIZE;

		//output the new char into the stuff file
		fputc(newchar, stuff);
	}
}

int main(int argc, char *argv[])
{
DWORD Threadid;
HANDLE ThreadHandle;
int theClock = 0;

//open file so that both the main thread and the consumer thread can access it.
fopen_s(&stuff, "stuff", "w");

//spin off thread
ThreadHandle = CreateThread(
	NULL, /* default security attributes */
	0, /* default stack size */
	Consumer, /* thread function */
	NULL, /* parameter to thread function */
	0, /* default creation flags */
	&Threadid); /*returns the thread identifier*/

//seed the random number generator with current time
srand((unsigned)time(NULL));

//produce data for the consumer thread
while (theClock < DATASIZE) {
	while ((in + 1) % BUFFER_SIZE == out);
    
	buf[in] = (double)rand()/(RAND_MAX+1) * ALPHA_CARD;

	in = (in + 1) % BUFFER_SIZE;
	theClock++;
}

//close the thread
TerminateThread(ThreadHandle, 0);

//close the thread handle
CloseHandle(ThreadHandle);
 
//flush and close the file stream
fclose(stuff);
}

