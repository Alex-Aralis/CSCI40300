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
    long slice = 4;
    string input = "";




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


    cout << "slice size: ";

    getline(cin, input);

    stringstream snum(input);

    snum >> slice;

    Alg* rr = new RR(&GT, &GEQ, 0, slice, v);

    Host h = Host(&GT, &GEQ, &rr, 1, fd, v);

    fclose(fd);

    h.start();

    cout << "Average wait time for " << argv[0]  <<": " << rr->avgWait() << "\n";
    cout << "Average turnaround  time for " << argv[0] << ": " << rr->avgTurnaround() << "\n";
} 
