#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>



void mpi_iittp_barrier(MPI_Comm mpi_comm)
{
	int rank, size, j;
	MPI_Comm_rank(mpi_comm, &rank);
	MPI_Comm_size(mpi_comm, &size);
	
	/*
	Rank 0 process first waits to receive message
	from all the processes and then sends a signal for all the processes
	to continue execution
	*/
	if(rank == 0)
	{
		int msg;
		//Wait to receive '0' from all ranks
		for(j=1; j<size; j++)
		{
			MPI_Recv(&msg, 1, MPI_INT, j, 0, mpi_comm, MPI_STATUS_IGNORE);
		}
		
		msg = 1;
		//Release all methods by sending msg as '1'
		for(j=1; j<size; j++)
		{
			MPI_Send(&msg, 1, MPI_INT, j, 0, mpi_comm);
		}
	}
	
	else 
	{
		/*
		All processes send 0 to process with rank 0
		and wait to receive 1
		*/
		int msg = 0;
		MPI_Send(&msg, 1, MPI_INT, 0, 0, mpi_comm);
		MPI_Recv(&msg, 1, MPI_INT, 0, 0, mpi_comm, MPI_STATUS_IGNORE);
	}
	
}


int main(int argc, char** argv)
{
	MPI_Init(NULL, NULL);
	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	//	Testing mpi barrier
	//  If all the processes print their rank in order them mpi barrier works
	// Else it is erroneous
	for ( int i = 0; i < world_size; i++ ) {
		if ( world_rank == i ) {
			printf("My process ID is %d\n", world_rank);
		}
		mpi_iittp_barrier(MPI_COMM_WORLD);
	}
  
  
  MPI_Finalize();	
}
