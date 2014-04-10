void MY_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm);

void MY_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm);

void MY_Broadcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm);

void MY_AllToAllBroadcast(void *sendbuf, int sendcount,
    MPI_Datatype sendtype, void *recvbuf, int recvcount,
    MPI_Datatype recvtype, MPI_Comm comm);

void MY_Reduce(void *sendbuf, void *recvbuf, int count,
    MPI_Datatype datatype, int OP, int root, MPI_Comm comm);
