CC=mpicc
CFLACGS=

all: scatter gather bcast

scatter: MY_Scatter.c MY_Scatter_driver.c
	$(CC) $(CFLAGS) MY_Scatter.c MY_Scatter_driver.c -o scatter

gather: MY_Gather.c MY_Gather_driver.c
	$(CC) $(CFLAGS) MY_Gather.c MY_Gather_driver.c -o gather

bcast: MY_Broadcast.c MY_Broadcast_driver.c
	$(CC) $(CFLAGS) MY_Broadcast.c MY_Broadcast_driver.c -o bcast

clean:
	rm scatter gather bcast

ctest: all clean
