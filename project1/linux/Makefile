all: p1p2 p1p1 matrix.out stuff

p1p2:project1part2.c libmatrix.a
	gcc  -O2 -o p1p2 -lpthread  project1part2.c libmatrix.a 

stuff: p1p1
	./p1p1

p1p1:project1part1.c
	gcc -O2 -o p1p1 -lpthread project1part1.c

libmatrix.o: libmatrix.c matrix.h
	gcc -O2 -c libmatrix.c 

libmatrix.a:libmatrix.o
	ar rcs libmatrix.a libmatrix.o

matrix.out: matrix.c matrix.h libmatrix.a
	gcc -O2  -o matrix.out matrix.c libmatrix.a

clean:
	rm p1p1 p1p2 matrix.out libmatrix.a libmatrix.o stuff

