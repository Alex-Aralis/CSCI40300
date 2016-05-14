#include <iostream>
#include <vector>
#include <queue>
#include <cassert>

#define MAX_FAILURES 2
#define BUF_SIZE 1024

using namespace std;

/*
this class will be used to store vectors of resources

the addition, minus, and less than or equal operation
are define for uses in the algorithm
*/
class resources:vector<long>{
public:
    resources(){
        
    }

    resources(char* input){
        while(*input != '\n' && *input != '\0'){
            push_back(atol(input));

            while(*input != ',' && *input != '\n' && *input != '\0'){
                input++;
            }

            if(*input == ','){
                input++;
            }
        }
    }


    resources operator- (const resources &rhs){
        assert(size() == rhs.size());

        resources ret;
        for(long i = 0; i < size(); i++){
            ret.push_back((*this)[i] - rhs[i]);
        }

        return ret;
    };

    resources operator+ (const resources &rhs){
        assert(size() == rhs.size());

        resources ret;
        for(long i = 0; i < size(); i++){
            ret.push_back((*this)[i] + rhs[i]);
        }

        return ret;
    };

    bool is_non_negative(resources res){
        for(long i = 0; i < res.size(); i++){
            if (res[i] < 0){
                return false;
            }
        }

        return true;
    }

    bool operator<= (resources rhs){
        assert(size() == rhs.size());

        if(is_non_negative(rhs - *this)){
            return true;
        }else{
            return false;
        }
    };

    void print(bool verbose = true){
        if (verbose){
            cout << "[";
            
            for(long i = 0; i < size() - 1; i++){
                cout << (*this)[i] << ",";
            }

            cout << (*this)[size()-1] << "]\n";
        }
    }
};

/*
this struct is used to track which procs
remain unsatisfied, and if a proc has been
found to be unsatisfiable for a given iteration
of the bankers algorithm.

i is the index of the proc

mark is the index of the iteration that
the proc was last found to be unsatisfied

ex.
if available is [0,0,1]

and reqs is:
[1,0,0]
[0,0,1]
[0,1,0]

if the indexes are being scanned in order
starting with index 0

0 rejected mark set to 0
1 accepted, remove from the queue of indexes
    availavle is now [0,0,2]
    bankers algorithm iteration incremented
2 rejected mark set to 1
0 rejected mark set to 1
2 is found to have mark == iteration
    all the indexes left in the queue are 
    unsatisfiable
terminate bankers algorithm
*/
struct index{
    long i;
    long mark;

    index(long i): i(i), mark(-1){};
};


/*
used to print out queues of longs
*/
void print_seq(queue<long> q, bool verbose = true){
    if(verbose){
        cout << "<" << q.front();
        q.push(q.front());
        q.pop();

        for(long i = 0; i < q.size() - 1; i++){
            cout << "," << q.front();
            q.push(q.front());
            q.pop();
        }
        
        cout << ">\n";
    }
}

void print_seq(queue<index> q, bool verbose = true){
    if(verbose){
        cout << "<" << q.front().i;
        q.push(q.front().i);
        q.pop();

        for(long i = 0; i < q.size() - 1; i++){
            cout << "," << q.front().i;
            q.push(q.front().i);
            q.pop();
        }
        
        cout << ">\n";
    }
}

bool get_reqs(FILE* fd, long N, vector<resources> &ret){
    char buf[BUF_SIZE];

    for(long i = 0; i < N; i++){
        if(!fgets(buf, BUF_SIZE, fd)) return false;
        ret.emplace_back(buf);
    }

    return true;
}


int main(){
    
    //provide debugging output if true
    bool v = false;

    //open file
    FILE* fd = fopen("data.txt", "r");

    //get first line that is prepended by the number of procs
    char buf[BUF_SIZE];
    fgets(buf, BUF_SIZE, fd);

    //read grab the number and store it in N
    char* p = buf;
    const long N = atol(p);

    //advance p until p is after the ; in the first line
    while(*(p++) != ';');


    //get total system recources on the rest of the first line
    resources system = resources(p);
    system.print(v);
    
    
    //get initial resources allocated
    vector<resources> allocated;
    get_reqs(fd, N, allocated);

    //deduct allocated resources from system
    for(long i = 0; i < N; i++){
        system = system - allocated[i];
        system.print(v);
    }


    //the number of the request occuring
    long request = 0;
    //grab next set of requests
    vector<resources> reqs;
    long failures = 0;
    //the first proc to have its resources harvested
    long marked_proc = 0;
    while(get_reqs(fd, N, reqs)){
        cout << "---------- REQUEST " << request << " START __________\n";

        cout << "Starting available resources: ";
        system.print();

        //place all possible indexes into a queue
        queue<index> indexes;
        for(long i = 0; i < N; i++){
            indexes.push(index(i));
        }

        //loop until the index queue goes full circle
        //without finding a satisfied process
        queue<long> seq;
        long iteration = 0;
        resources available = system;
        available.print(v);
        while(indexes.front().mark < iteration){
            //cycle an index
            indexes.front().mark = iteration;
            indexes.push(indexes.front());
            indexes.pop();

            //if index is satisfied
            if(reqs[indexes.front().i] <= available){
                //expand available resources
                available = available + reqs[indexes.front().i];
                //add index to safe sequence
                seq.push(indexes.front().i);
                //remove index from index queue
                indexes.pop();

                if(indexes.empty()){
                    break;
                }else{
                    //the iteration has advanced,
                    //now all marks will be less than
                    //iteration
                    iteration++;
                }
            }
        }

        for(long i = 0; i < N; i++){
            reqs[i].print(v);
        }

        
        //check if there is a safe sequence
        if(indexes.empty()){
            failures = 0;
            cout << "There is a safe sequence: ";
            print_seq(seq);
        }else{
            failures++;
            cout << "No safe seequence!!!\n";
            cout << "Unsafe indexes: ";
            print_seq(indexes);
        }

        //give some debugging output
        if(v){
            cout << "failures: " << failures << "\n";
            cout << "marked_proc: " << marked_proc << "\n";
        }

        cout << "---------- REQUEST " << request << " END ------------\n";

        //check if failures has exeeded the max allowed
        if(failures > MAX_FAILURES){
            //if some allocations remain unharvested
            if (marked_proc < N){
                system = system + allocated[marked_proc];            
                cout << "Resources harvested: ";
                allocated[marked_proc].print();
                marked_proc++;
            //if there is nothing left to havest from allocated
            //system is already has all resources available
            }else{
                cout << "Nothing left to harvest!!!\n";
                cout << "Halting...\n";
                exit(1);
            }
        }


        //reset reqs
        reqs = vector<resources>();
        request++;
    }

    cout << "EOF reached.\n";
    cout << "Halting...\n";
}
