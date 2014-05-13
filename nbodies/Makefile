CC=mpicc
CFLAGS=-Wall -g

serial: nbody_serial.c
	$(CC) $(CFLAGS) nbody_serial.c -o nbody_serial

parallel: nbody_parallel.c
	$(CC) $(CFLAGS) nbody_parallel.c -o nbody_parallel

clean:
	rm -f nbody_serial nbody_parallel
