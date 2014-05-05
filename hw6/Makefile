CC=mpicc
CFLAGS=-lm

all: p1 p2 p3 p4

p1: problem1.c
	$(CC) $(CFLAGS) problem1.c -o p1

p2: problem2.c
	$(CC) $(CFLAGS) problem2.c -o p2

p3: problem3.c
	$(CC) $(CFLAGS) problem3.c -o p3

p4: problem4.c
	$(CC) $(CFLAGS) problem4.c -o p4

matrix:
	./wrand A.txt 10 10 0 10

vector:
	./wrand x.txt 10 1 0 10

clean:
	rm -f b.txt p1 p2 p3 p4 *.orig

format:
	astyle --options=../code_format *.c

problem1: p1 matrix vector
	mpirun p1 A.txt x.txt b.txt

problem2: p2 matrix vector
	mpirun p2 A.txt x.txt b.txt

problem3: p3 matrix
	mpirun p3 A.txt A.txt B.txt

problem4: p4 matrix
	mpirun p4 A.txt A.txt B.txt

print_matrix:
	cat A.txt | sed 's/ /,/g'
