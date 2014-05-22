CC=gcc
MPICC=mpicc
CFLAGS=-Wall -g -lm

nbody_serial: nbody_serial.o nbody_utils.o
	$(CC) $(CFLAGS) nbody_serial.o nbody_utils.o -o nbody_serial

nbody_serial.o: nbody_serial.c nbody_serial.h nbody_utils.h
	$(CC) $(CFLAGS) nbody_serial.c -o nbody_serial.o


nobdy_parallel: nbody_parallel.o nbody_utils.o
	$(MPICC) $(CFLAGS) nbody_parallel.o nbody_utils.o -o nbody_parallel

nbody_parallel.o: nbody_parallel.c nbody_parallel.h nbody_utils.h
	$(MPICC) $(CFLAGS) nbody_parallel.c -o nbody_parallel.o
	

nbody_utils.o: nbody_utils.c nbody_utils.h
	$(CC) $(CFLAGS) -c nbody_utils.c -o nbody_utils.o

test: serial
	./nbody_serial initial_state.txt out.txt 100 10

clean:
	rm -f nbody_serial nbody_parallel *.o
