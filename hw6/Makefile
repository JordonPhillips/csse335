CC=mpicc
CFLAGS=-lm

all: p1 p2

p1: problem1.c
	$(CC) $(CFLAGS) problem1.c -o p1

p2: problem2.c
	$(CC) $(CFLAGS) problem2.c -o p2

matrix:
	./wrand A.txt 10 10 0 10

vector:
	./wrand x.txt 10 1 0 10

clean:
	rm p1 p2

problem1: p1 matrix vector
	./p1 A.txt x.txt b.txt

problem2: p2 matrix vector
	./p2 A.txt x.txt b.txt

print_matrix:
	cat A.txt | sed 's/ /,/g'
