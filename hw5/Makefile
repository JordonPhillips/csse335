CC=mpicc
CFLACGS=

all: scatter

scatter: MY_Scatter.c MY_Scatter_driver.c
	$(CC) $(CFLAGS) MY_Scatter.c MY_Scatter_driver.c -o scatter

clean:
	rm scatter
