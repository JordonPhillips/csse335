#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#define MY_REDUCE_TAG 51

int adj_to_rank(int adj, int root, int total_procs);

void MY_Reduce(void *sendbuf, void *recvbuf, int count,
    MPI_Datatype datatype, int OP, int root, MPI_Comm comm) {
    int rank, total_procs;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &total_procs);

    int adjusted_rank = rank - root;
    if (adjusted_rank < 0)
        adjusted_rank += total_procs;

    int adjusted_total = total_procs%2 == 0 ? total_procs : total_procs - 1;
    MPI_Status status;
    int send_size, neighbor;

    MPI_Type_size(datatype, &send_size);
    int *buff = (int*)malloc(send_size*count);
    memcpy(buff, sendbuf, send_size*count);

    int i = 1, j,k;
    while (i < total_procs) {
        i *= 2;
        if (adjusted_rank % i == 0) {
            neighbor = adjusted_rank + i;
            if (neighbor < total_procs) {
                neighbor = adj_to_rank(neighbor, root, total_procs);
                MPI_Recv(buff, count, datatype, neighbor, MY_REDUCE_TAG, comm, &status);
                for (j = 0; j < count; j++) {
                    k = (int)*((char*)recvbuf+(i*send_size));
                    if (OP == 1) {
                        buff[i] += k;
                    } else if (OP == 2) {
                        buff[i] = buff[i] < k ? buff[i] : k;
                    }
                }
            }
        } else {
            neighbor = adj_to_rank(adjusted_rank - i, root, total_procs);
            MPI_Send(buff, count, datatype, neighbor, MY_REDUCE_TAG, comm);
            break;
        }
    }

    if (rank == root) {
        memcpy(recvbuf, buff, send_size*count);
    }
    free(buff);
}

int adj_to_rank(int adj, int root, int total_procs) {
    int rank = adj + root;
    if (rank > total_procs)
        rank -= total_procs;
    return rank;
}
