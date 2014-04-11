#include <mpi.h>
#include <stdio.h>

#define MY_BROADCAST_TAG 46

void MY_Broadcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);

    int total_procs;
    MPI_Comm_size(comm, &total_procs);

    int adjusted_rank = rank - root;
    if (adjusted_rank < 0)
        adjusted_rank += total_procs;

    if (rank != root) {
        int sender = adjusted_rank%2 == 0 ? (adjusted_rank/2)-1 : (adjusted_rank-1)/2;
	printf("%d receiving from %d (adj)\n", adjusted_rank, sender);
        sender = (sender + root)%total_procs;
        printf("%d recieving from %d\n", rank, sender);
        MPI_Status status;
        MPI_Recv(buffer, count, datatype, sender, MY_BROADCAST_TAG, comm, &status);
    }

    int target1 = (2*adjusted_rank) + 1;
    int target2 = 2*(adjusted_rank + 1);
    if (target1 < total_procs) {
        target1 = (root+target1)%total_procs;
        printf("%d sending to %d\n", rank, target1);
        MPI_Send(buffer, count, datatype, target1, MY_BROADCAST_TAG, comm);

        if (target2 < total_procs) {
            printf("%d sending to %d\n", rank, target2);
            target2 = (root+target2)%total_procs;
            MPI_Send(buffer, count, datatype, target2, MY_BROADCAST_TAG, comm);
        }
    }
    fflush(stdout);
}

