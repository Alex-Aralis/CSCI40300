CFLAGS=-std=c++14
CC=g++

all: RR SJFS FCFS MLQ

infile1: procgen
	./procgen 100 100 10 4 1 > infile

infile2: procgen
	./procgen 100 100 10 4 2 > infile

MLQ: libprocsim.a MLQ.cpp
	$(CC) $(CFLAGS) MLQ.cpp libprocsim.a -o MLQ

RR: libprocsim.a RR.cpp
	$(CC) $(CFLAGS) RR.cpp libprocsim.a -o RR

SJFS: libprocsim.a SJFS.cpp
	$(CC) $(CFLAGS) SJFS.cpp libprocsim.a -o SJFS

FCFS: libprocsim.a FCFS.cpp
	$(CC) $(CFLAGS) FCFS.cpp libprocsim.a -o FCFS

libprocsim.a: libprocsim.o
	ar rvs libprocsim.a libprocsim.o

libprocsim.o: libprocsim.cpp libprocsim.h
	$(CC) $(CFLAGS) -c libprocsim.cpp -o libprocsim.o

clean:
	rm *.o *.a RR SJFS FCFS MLQ
