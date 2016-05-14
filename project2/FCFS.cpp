#include "libprocsim.h"
#include "string.h"

using namespace ProcSim;
using namespace std;

int main(int argc, char** argv){
    EPQ* GEQ;
    long GT = 0;
    FILE* fd;
    bool v = false;
    const char* filepath = "infile";


    if(argc > 1){
        if(!strcmp(argv[1], "-v")){
            v = true;
            if(argc > 2){
                filepath = argv[argc-1];
            }
        }else{
            filepath = argv[argc-1];
        }
    }



    if(!(fd = fopen(filepath, "r"))){
        perror("infile failed to open\n");
        exit(1);
    }

    Alg* fcfs = new FCFS(&GT, &GEQ, 0, v);

    Host h = Host(&GT, &GEQ, &fcfs, 1, fd, v);

    fclose(fd);

    h.start();

    cout << "Average wait time for " << argv[0]  <<": " << fcfs->avgWait() << "\n";
    cout << "Average turnaround  time for " << argv[0] << ": " << fcfs->avgTurnaround() << "\n";
} 
