#include "libprocsim.h"

#define IO_INT_LEN 5;

using namespace std;

namespace ProcSim{

    long Proc::setIOs(){
        totIO = PIOQ.size();
    }

    long Proc::TTNE(){
        if(!PIOQ.empty()){
            return PIOQ.top()->T - gT; 
        }else{
            return rT;
        }
    }

    pState Proc::runfor(long slice, long* GT){
        pState ret;

        wT += *GT - eT;

        *GT += slice;
        rT -= slice;
        gT += slice;

        eT = *GT;

        if(rT == 0 && PIOQ.empty()){
            tT = *GT - aT;
            ret = finished;
        }else{
            ret = unfinished;
        }

        return ret;
    }

    void Proc::print(bool v){
        if(!v){
            return;
        }

        printf("PID:%ld, T:%ld, Prior:%ld, aT:%ld, eT:%ld; wT:%ld, tT:%ld, gT:%ld, rT:%ld, totIO:%ld, IO:", PID, T, priority, aT, eT, wT, tT, gT, rT, totIO);
        IOEQ tmp;
        while(!PIOQ.empty()){
            tmp.push(PIOQ.top());
            printf("%ld", PIOQ.top()->T);
            PIOQ.pop();
            if(!PIOQ.empty()){
                printf(",");
            }
        }
        PIOQ = tmp;
        printf(";\n");
    }

    bool EventComparator::operator()(Event* &lhs, Event* &rhs) {
        return lhs->T>rhs->T || (lhs->T==rhs->T && lhs->proc->PID>rhs->proc->PID);
    }

    bool ProcComparator::operator()(Proc* &lhs, Proc* &rhs) {
        //alternate inperpretation of shortest
        //return lhs->rT>rhs->rT;

        return lhs->TTNE()>rhs->TTNE();

    }

    double Alg::avgTurnaround(){
        Proc* proc;
        stack<Proc*>* tmp = new stack<Proc*>;
        long sum = 0;
        long count = 0;
        while(!FPS->empty()){
            proc = FPS->top();
            FPS->pop();
            count++;
            sum += proc->tT;
            tmp->push(proc);
        }

        delete FPS;
        FPS = tmp;

        return (double) sum / count;
    }

    double Alg::avgWait(){
        Proc* proc;
        stack<Proc*>* tmp = new stack<Proc*>;
        long sum = 0;
        long count = 0;
        while(!FPS->empty()){
            if(verbose){
                printf("PID: %ld, wT: %ld, tT: %ld\n", FPS->top()->PID, FPS->top()->wT, FPS->top()->tT);
            }
            proc = FPS->top();
            FPS->pop();
            count++;
            sum += proc->wT;
            tmp->push(proc);
        }

        delete FPS;
        FPS = tmp;

        return (double) sum / count;
    }

    bool Alg::interupted(Proc* proc){
        return !((*GEQ)->empty() || proc->TTNE() + *GT < (*GEQ)->top()->T);
    }

    void Alg::schedNextIO(Proc* proc){
        proc->PIOQ.top()->T = *GT + IO_INT_LEN;
        proc->eT = *GT + IO_INT_LEN;
        (*GEQ)->push(proc->PIOQ.top());
        proc->PIOQ.pop();
    }

    Alg::~Alg(){
        Proc* proc;

        while(!FPS->empty()){
            proc = FPS->top();
            FPS->pop();
            delete proc;
        }
        
        delete FPS;
    }


    void SJFS::log(string msg){
        if(verbose){
            cout << "[" << priority << " SJFS GT: " << *GT << "] " << msg << "\n";
        }
    }

    void SJFS::push(Proc* proc){
        RQ.push(proc);
    }

    bool SJFS::empty(){
        return RQ.empty() && running == nullptr;
    }

    bool SJFS::step(){
        log("beginning step");
        bool ret;

        //if nothing is running load next proc
        if(!running){
            log("popping from RQ");

            running = RQ.top();
            RQ.pop();
        }
        
        running->print(verbose);
        
        //if remainder of job will not be interupted
        if(!interupted(running)){
            log("no interupt from gloabal");

            //if running is finished
            if(running->runfor(running->TTNE(), GT) == finished){
                log("---------------------->running has finished");
                FPS->push(running);
            //if running hit IO
            }else{
                log("scheduling next IO");
                schedNextIO(running);
            }

            running->print(verbose);
            running = nullptr;
            ret = false;

        //if proc will be interupted by global q
        }else{
            log("proc interupted by global q");
            running->runfor((*GEQ)->top()->T - *GT, GT);

            running->print(verbose);
            ret = true;
        }

        return ret;
    }

    void RR::log(string msg){
        if(verbose){
            cout << "[" << priority << " RR GT: " << *GT << "] " << msg << "\n";
        }
    }

    void RR::push(Proc* proc){
        RQ.push(proc);
    }

    bool RR::empty(){
        return RQ.empty() && running == nullptr;
    }

    bool RR::interupted(Proc* proc){
        return !((*GEQ)->empty() || proc->TTNE() + *GT <= (*GEQ)->top()->T || rS + *GT < (*GEQ)->top()->T);
    }

    bool RR::step(){
        log("beginning step");
        bool ret;
        long ttne;
        //if nothing is running load next proc
        if(!running){
            log("popping from RQ");
            rS = slice;
            running = RQ.front();
            RQ.pop();
        }

        running->print(verbose);

        //if remainder of job will not be interupted
        if(!interupted(running)){
            log("no interupt from gloabal");

            //if running is finished

            ttne = running->TTNE();
            
            if(running->rT == ttne && running->PIOQ.empty() && ttne <= rS){
                running->runfor(ttne, GT);
                log("---------------------------->proc finished");
                running->print(verbose);

                FPS->push(running);
            //if time slice is up
            }else if(ttne > rS){
                log("time slice up");
                running->runfor(rS, GT);
                RQ.push(running);

                running->print(verbose);
            //if running hit IO
            }else{
                log("running IO");
                running->runfor(ttne, GT);
                schedNextIO(running);
                running->print(verbose);
            }

            running = nullptr;
            ret = false;

        //if proc will be interupted by global q
        }else{
            log("proc being interupted by global");
            rS -= (*GEQ)->top()->T - *GT;
            running->runfor((*GEQ)->top()->T - *GT, GT);

            running->print(verbose);
            ret = true;
        }

        log("ending step");
        return ret;
    }

    void FCFS::log(string msg){
        if(verbose){
            cout << "[" << priority << " FCFS GT: " << *GT << "] " << msg << "\n";
        }
    }

    void FCFS::push(Proc* proc){
        RQ.push(proc);
    }

    bool FCFS::empty(){
        return RQ.empty() && running == nullptr;
    }

    bool FCFS::step(){
        log("beginning step");
        bool ret;

        //if nothing is running load next proc
        if(!running){
            log("popping from RQ");
            running = RQ.front();
            RQ.pop();
        }

        running->print(verbose);

        //if remainder of job will not be interupted
        if(!interupted(running)){
            log("no interupt from gloabal");

            //if running is finished
            if(running->runfor(running->TTNE(), GT) == finished){
                log("------------------------>proc finished");
                running->print(verbose);

                FPS->push(running);
            //if running hit IO
            }else{
                log("running IO");
                schedNextIO(running);
                running->print(verbose);
            }

            running = nullptr;
            ret = false;

        //if proc will be interupted by global q
        }else{
            log("proc being interupted by global");
            running->runfor((*GEQ)->top()->T - *GT, GT);

            running->print(verbose);
            ret = true;
        }

        log("ending step");
        return ret;
    }


    //it is worth noting that even though FCFS, RR, and SJFS are non-preemptive 
    //my Host will preempt the Algs themselves when a proc arrives of a higher priority

    Host::Host(long *GT, EPQ** GEQ, Alg** algList, long priorities, FILE* infile, bool v)
            :GT(GT), GEQ(GEQ), algList(algList), priorities(priorities), verbose(v){
        long PID, T, CPUT, priority;
        char IOs[1024], buf[4096];
        char* IOsStart = IOs;
        char* IOsEnd = IOs;
        vector<Event*> eventV;
        
        while(fgets(buf, 4096, infile)){
            IOs[0] = '\0';
            if(3 == sscanf(buf, "P%ld,%ld,%ld(%1023[0123456789,])%ld;", &PID, &T, &CPUT, IOs, &priority)){
                sscanf(buf, "%*[P0123456789,(])%ld;", &priority);
            }
            if(verbose){
                printf("%ld, %ld, %ld, %s, %ld\n", PID, T, CPUT, IOs, priority);
            }
            Proc* tmp = new Proc(PID, T, CPUT, priority);
            while(*IOsEnd != '\0'){
                if(*IOsEnd == ','){
                    *IOsEnd = '\0';
                    tmp->PIOQ.push(new Event(tmp, atol(IOsStart)));
                    IOsStart = IOsEnd+1;
                }
                IOsEnd++;
            }

            if(IOsStart != IOsEnd){
                tmp->PIOQ.push(new Event(tmp, atol(IOsStart)));
            }
            
            tmp->setIOs();
            eventV.push_back(new Event(tmp, T));

            IOsStart = IOs;
            IOsEnd = IOs;
        }
        
        *GEQ = new EPQ(EventComparator(), eventV);
        
    }

    void Host::log(string msg){
        if(verbose){
            cout << "[host GT:" << *GT << "] " << msg << "\n";
        }
    }

    bool Host::algsEmpty(){
        bool ret = true;

        log("----------------->running algsEmpty");
        for(long i = 0; i < priorities; i++){
            log("loop algsEmpty");
            if(!algList[i]->empty()){
                ret = false;
                break;
            }
        }

        return ret;
    }

    bool Host::empty(){
        if((*GEQ)->empty() && algsEmpty()){
            return true;
        }else{
            return false;
        }
    }

    void Host::start(){
        Event* TGE;
        long i = 0;
        bool interupted = true;

        if(!(*GEQ)->empty()){
            log("Host idling");
            (*GT) = (*GEQ)->top()->T;
            log("New GT set");
        }

        while(!empty()){
            log("running step");

            if(interupted && !(*GEQ)->empty()){
                log("new arrival");
                    
                TGE = (*GEQ)->top();
                (*GEQ)->pop();
                
                if(TGE->proc->priority >= priorities){
                    algList[priorities - 1]->push(TGE->proc);
                }else{
                    algList[TGE->proc->priority]->push(TGE->proc);
                }

                delete TGE;

            }else{
                log("control released");
            }

            for(i = priorities - 1; i>=0 && algList[i]->empty(); i--);
            if(i == -1){
                log("Host idling");
                (*GT) = (*GEQ)->top()->T;
                interupted = true;
                continue;
            }
            
            interupted = algList[i]->step();
        }
    }

    Host::~Host(){
        delete *GEQ; 
        for(long i = priorities - 1; i >= 0; i--){
            delete algList[i];
        }
    }
}
