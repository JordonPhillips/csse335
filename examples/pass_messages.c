#include<stdio.h>
#include<mpi.h>

int main(int argc, char** argv){
  MPI_Init(&argc,&argv);
  int total_procs;
  int rank;
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  if (rank==0){
    int i;
    char* message="I am your master!";
    char recv_buffer[10];
    for (i=1;i<total_procs;i++){
      MPI_Send(message,17,MPI_CHAR,i,0,MPI_COMM_WORLD);
    }
  }
  else{
    MPI_Status status;
    char recv_buffer[20];
    MPI_Recv(recv_buffer,20,MPI_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    recv_buffer[17]='\0';
    printf("Process %d: Message from %d:%s\n",rank,status.MPI_SOURCE,recv_buffer);
  }
  MPI_Finalize();

}
