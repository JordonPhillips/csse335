CC=mpicc
CFLACGS=

all: scatter gather bcast alltoall

scatter: MY_Scatter.c MY_Scatter_driver.c
	$(CC) $(CFLAGS) MY_Scatter.c MY_Scatter_driver.c -o scatter

gather: MY_Gather.c MY_Gather_driver.c
	$(CC) $(CFLAGS) MY_Gather.c MY_Gather_driver.c -o gather

bcast: MY_Broadcast.c MY_Broadcast_driver.c
	$(CC) $(CFLAGS) MY_Broadcast.c MY_Broadcast_driver.c -o bcast

alltoall: MY_AllToAllBroadcast.c MY_AllToAllBroadcast_driver.c
	$(CC) $(CFLAGS) MY_AllToAllBroadcast.c MY_AllToAllBroadcast_driver.c -o alltoall

clean:
	rm scatter gather bcast alltoall

ctest: all clean
