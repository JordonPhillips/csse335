CC=mpicc
CFLAGS=-lm

all: p1 matrix vector

p1: problem1.c
	$(CC) $(CFLAGS) problem1.c -o p1

matrix:
	./wrand A.txt 10 10 0 10

vector:
	./wrand x.txt 10 1 0 10

clean:
	rm p1 A.txt x.txt

ctest: all clean
