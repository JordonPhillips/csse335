CC=mpicc
CFLAGS=-Wall -g -lm

serial: nbody_serial.c
	$(CC) $(CFLAGS) nbody_serial.c -o nbody_serial

parallel: nbody_parallel.c
	$(CC) $(CFLAGS) nbody_parallel.c -o nbody_parallel

test: serial
	./nbody_serial initial_state.txt out.txt 100 10

clean:
	rm -f nbody_serial nbody_parallel
