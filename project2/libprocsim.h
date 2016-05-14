#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <stack>

#ifndef PROCSIM_INCLUDE
#define PROCSIM_INCLUDE

#define IO_INT_LEN 5;

using namespace std;

namespace ProcSim{
    enum pState {unfinished, finished};

    class Event;
    class Proc;
    class EventComparator;
    class ProcComparator;
    class Alg;
    class FCFS;
    class SJFS;
    class RR;
    class Host;

    typedef priority_queue<Event*, vector<Event*>, EventComparator> EPQ;
    typedef priority_queue<Event*, vector<Event*>, EventComparator> IOEQ;
    typedef priority_queue<Proc*, vector<Proc*>, ProcComparator> SJFPQ;
    typedef queue<Proc*> PRQ;


    class Event{
        public: Proc* const proc;
        public: long T;
        public: Event(Proc* proc, long T):proc(proc), T(T) {};
    };

    class EventComparator{
        public: bool operator()(Event* &lhs, Event* &rhs);
    };

    class Proc {
        public:long const PID;
        public:long const T; //run time
        public:long const priority; 
        public:long const aT; //time of first arrival
        public:long eT; //time of last event
        public:long wT; //total time waited
        public:long tT; //time from arrival to completion
        public:long rT; //remaining cpu time needed
        public:long gT; //time proc has been given
        public:long totIO; //total number of IO events
        public:IOEQ PIOQ; //Proc IO Queue

        public:Proc(long PID = -1, long aT = -1, long T = -1, long priority = -1)
            :PID(PID), aT(aT), eT(aT), T(T), priority(priority), rT(T), gT(0), wT(0), tT(-1), totIO(0){}
        
        public: pState runfor(long, long*);
        public: long setIOs();
        public: long TTNE();
        public: void print(bool v = true);
    };
    
    class ProcComparator{
        public: bool operator()(Proc* &lhs, Proc* &rhs);
    };

    class Alg {
        protected: long *GT;
        protected: bool verbose;
        public:long const priority;
        public:EPQ** GEQ;
        public:stack<Proc*>* FPS;
        protected:Proc* running;

        public: Alg(long *GT, EPQ** GEQ, long priority, bool v = false)
            :GT(GT), GEQ(GEQ), priority(priority), verbose(v), running(nullptr), FPS(new stack<Proc*>){}
    
        public: double avgTurnaround();
        public: double avgWait();
        public: bool interupted(Proc* proc);
        public: void schedNextIO(Proc* proc);

        public: virtual bool step() = 0;
        public: virtual void push(Proc*) = 0; 
        public: virtual void log(string) = 0;
        public: virtual bool empty() = 0;

        public: ~Alg();
    };

    class SJFS: public Alg{
        public: SJFPQ RQ;

        public:SJFS(long* GT, EPQ** GEQ, long priority, bool v):Alg(GT, GEQ, priority, v){}

        public: void log(string msg);
        public: void push(Proc* proc);
        public: bool empty();
        public: bool step();
    };

    class RR: public Alg{
        public: PRQ RQ;
        public: const long slice;
        public: long rS;

        public:RR(long* GT, EPQ** GEQ, long priority, long s = 4, bool v = false):Alg(GT, GEQ, priority, v), slice(s){}

        public: void log(string msg);
        public: void push(Proc* proc);
        public: bool empty();
        public: bool interupted(Proc* proc);
        public: bool step();
    };

    class FCFS: public Alg{
        public: PRQ RQ;

        public:FCFS(long* GT, EPQ** GEQ, long priority, bool v = false):Alg(GT, GEQ, priority, v){}

        public: void log(string msg);
        public: void push(Proc* proc);
        public: bool empty();
        public: bool step();
    };

    class Host {
        public:long* GT;
        long priorities;
        Alg** algList;
        public:EPQ** GEQ;
        public:bool verbose;

        public:Host(long *GT, EPQ** GEQ, Alg** algList = nullptr, long priorities = 0, FILE* infile = stdin, bool v = false);

        public: void log(string msg);
        public: bool algsEmpty();
        public: bool empty();
        public: void start();

        public:~Host();
    };

}

#endif
