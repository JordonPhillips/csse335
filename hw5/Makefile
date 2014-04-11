CC=mpicc
CFLACGS=

all: scatter gather

scatter: MY_Scatter.c MY_Scatter_driver.c
	$(CC) $(CFLAGS) MY_Scatter.c MY_Scatter_driver.c -o scatter

gather: MY_Gather.c MY_Gather_driver.c
	$(CC) $(CFLAGS) MY_Gather.c MY_Gather_driver.c -o gather

clean:
	rm scatter gather

ctest: all clean
