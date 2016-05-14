#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//number of things to be produced before the process is terminated
#define DATASIZE 2000 

//size of buffer used to pass information between the producer and consumer
#define BUFFER_SIZE 50

//Alphabet Cardinality; if r is the random number to be produced then
//r member [0, ALPHA_CARD) and r member the set of integers.
#define ALPHA_CARD 4

//allocate memory to be shared
//int's volatile to allow for O2 optimization
volatile int in = 0;
volatile int out = 0;
volatile int buf[BUFFER_SIZE];

FILE* stuff;

void* Consumer(void *param); 
void ConsumerCleanupHandler(void *param);

int main(int argc, char *argv[]){
    pthread_t th;
    pthread_attr_t attr; 

    int theClock = 0;
    
    stuff = fopen("stuff", "w");
    
    //initialize attr struct
    pthread_attr_init(&attr);

    //dispatch thread
    pthread_create(&th, &attr, Consumer, NULL);

    //destroy attr struct
    pthread_attr_destroy(&attr);

    //seed random()
    srandom(time(NULL));

    //produce random numbers until there are 
    //DATASIZE numbers produced.
    while(theClock < DATASIZE){
        //spinlock while buffer is full
        while((in + 1) % BUFFER_SIZE == out);
       
        buf[in] = (double)random()/((double)RAND_MAX + 1) * ALPHA_CARD;
        in = (in + 1) % BUFFER_SIZE;
        theClock++;
    }

    //tell th to terminate at its soonest convenience.
    pthread_cancel(th);

    //tell kernel PCB can be dealocated
    pthread_join(th,NULL);

}

//entry point of the consumer thread 
void* Consumer(void *val){    
    //buffer for new char
    char newchar;

    //set a cleanup handler to close stuff after thread is canceled.
    pthread_cleanup_push(ConsumerCleanupHandler, NULL);

    //set pthread to cancel as soon as it recieves a cancel signal.
    int trash;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &trash);
 
         
 
    //read from buf forever
    while(1){
        //spinlock while buf empty
        while(in == out);

        //convert buf[out] from an in to ascii code via offset
        //then write to stuff
        if(fputc(buf[out]+48, stuff) == EOF){
            printf("fputc failed");
        }

        //update out to next int to read
        out = (out + 1) % BUFFER_SIZE;
    }

    //cleanup pop for posix compliance
    pthread_cleanup_pop(1);
}

void ConsumerCleanupHandler(void *param){
    //close resources in order to flush
    fclose(stuff);
}
