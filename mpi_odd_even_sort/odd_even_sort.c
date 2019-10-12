#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

/*
Note:
	For now the value of N(total number of elements to be sorted 
	can only be defined in the program with #define N <val>.
	
	However number of processes can be given through command line as
		"-np <val>".
EXECUTION:
	$ mpicc -o <fileExecName> odd_even_sort.c
	$ mpirun -np <num_processes> ./fileExecName

ABOUT:	
	This program implements odd even transposition sort using mpi.
	All the processes first locally sort the elements in their local array, 
	with each process sorting nearly n/np elements.
	Then they communicate to merge sorted elements in order,
	in "odd-even sort" fashion.
	
REF:
	An Introduction to Parallel Programming, Peter Pacheco, Morgan Kaufmann Publishers, 2011
*/


#define N 16      
#define IDEAL -10
#define MY_RAND_MAX 1000

int ceil_div(int x, int y);
int Compare(const void* val_a, const void* val_b);
int get_size_arr(int rank, int np, int n);
int * generate_local_array(int rank, int n, int np);
void print_global_array(int* my_arr, int my_arr_size,
         int my_rank, int np, int n, MPI_Comm comm);
void print_array(int * arr, int arr_size);
void print_array_with_rank(int * arr, int arr_size, int rank);

int compute_partner(int phase, int my_rank, int np);
void sort(int my_rank, int* local_arr, int size_local_arr, int n, int np, MPI_Comm comm) ;
void Merge_low(int* my_keys, int* recv_keys, int* temp_keys,
				int my_arr_size, int partner__arr_size);
void Merge_high(int* my_keys, int* recv_keys, int* temp_keys,
				int my_arr_size, int partner__arr_size);


int main(int argc, char** argv)
{
	//Initialization
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	int np, rank, i;
	int size_local_arr;
	MPI_Comm_size(comm, &np);
	MPI_Comm_rank(comm, &rank);
	
	
	//Randomly creating array of non-negative integers
	int* local_arr;
	local_arr = generate_local_array(rank, N, np);
	size_local_arr = get_size_arr(rank, np, N);

	MPI_Barrier(comm);	

	printf("Unsorted array:\n");
	MPI_Barrier(comm);	
	print_global_array(local_arr, size_local_arr, rank, np, N, comm);
	MPI_Barrier(comm);
	
	
	//Sort everything using odd-even sort
	sort(rank, local_arr, size_local_arr, N, np, comm);


	//printing sorted array
	printf("Sorted array:\n");
	MPI_Barrier(comm);
	
	print_global_array(local_arr, size_local_arr, rank, np, N, comm);
	
  	MPI_Finalize();	
}



/*
	Odd even sort implementation
*/
void sort(int my_rank, int* my_keys, int my_arr_size, int n, int np, MPI_Comm comm) 
{
	int phase, partner;
	
	//Sort the local array
	qsort(my_keys, my_arr_size, sizeof(int), Compare);
	
	//Phasing
	for (phase = 0; phase < np; phase++) {
		//find partner
		partner = compute_partner(phase, my_rank, np);
		if (partner != IDEAL) {
			//Send my keys to partner;
			//Receive keys from partner;
			int partner_arr_size = get_size_arr(partner, np, n);
			int* recv_keys = (int *)malloc(partner_arr_size*sizeof(int));
			int* temp_keys = (int *)malloc(my_arr_size*sizeof(int));
			
			MPI_Sendrecv(my_keys, my_arr_size, MPI_INT, partner, 0,
			recv_keys, partner_arr_size, MPI_INT, partner, 0, comm, 
			MPI_STATUS_IGNORE);
			
			if (my_rank < partner)
			{
				//Keep smaller keys;
				Merge_low(my_keys, recv_keys, temp_keys, my_arr_size,
							partner_arr_size);
			}
			else
			{
				//Keep larger keys;
				Merge_high(my_keys, recv_keys, temp_keys, my_arr_size,
							partner_arr_size);
			}
		}
		MPI_Barrier(comm);
		
	}
}

/*
	Find partner based on process rank and phase
*/
int compute_partner(int phase, int my_rank, int np)
{
	int partner;
	if (phase % 2 == 0) 	//even phase
	{	
		
		if (my_rank % 2 != 0) 	//odd rank
		{	
			partner = my_rank - 1;
		}
		else	// Even rank
		{	 
			partner = my_rank + 1;
		}
	}	
	else		// Odd phase 
	{
		if (my_rank % 2 != 0)	//Odd rank 
		{
			partner = my_rank + 1;
		}
		else	//Even rank 
		{
			partner = my_rank - 1;
		}
	}
	
	//Check if partner rank is out of boundary 
	// i.e. process remains ideal
	if (partner == -1 || partner == np)
	{
		partner = IDEAL;
	}
			
	return partner;
}

/*
Merge function to keep the smallest elements among 
local array and array received from the partner.
*/
void Merge_low(int* my_keys, int* recv_keys, int* temp_keys,
				int my_arr_size, int partner_arr_size)
{
	int mi, ri, ti;
	int* full_arr = (int *)malloc((partner_arr_size+my_arr_size)*sizeof(int));
	mi = ri = ti = 0;
	
	/*
		Last process may have lesser number of elements
		Hence the two cases are handled separately for the sake of merging.
	*/
	if(partner_arr_size == my_arr_size)
	{
		while(ti<my_arr_size)
		{
			if(*(my_keys+mi) <= *(recv_keys+ri))
			{
				*(temp_keys+ti) = *(my_keys+mi);
				ti++;
				mi++;
			}
			else
			{
				*(temp_keys+ti) = *(recv_keys+ri);
				ti++;
				ri++;
			}
		}
	}
	else
	{
		int zi = 0;
		for(zi=0; zi<my_arr_size; zi++)
		{
			*(full_arr + zi) = *(my_keys + zi);
		}
		for(zi=my_arr_size; zi < (my_arr_size+partner_arr_size); zi++)
		{
			*(full_arr + zi) = *(recv_keys + zi -my_arr_size);
		}
		
		qsort(full_arr, my_arr_size+partner_arr_size, sizeof(int), Compare);
		
		
		for(zi=0; zi<my_arr_size; zi++)
		{
			*(temp_keys + zi) = *(full_arr + zi);
		}
		
		
	}
	
	for(mi=0; mi<my_arr_size; mi++)
	{
		*(my_keys + mi) = *(temp_keys + mi);
	}
	free(full_arr);
}


	
/*
Merge function to keep the largest elements among 
local array and array received from the partner.
*/
void Merge_high(int* my_keys, int* recv_keys, int* temp_keys,
				int my_arr_size, int partner_arr_size)
{
	int mi, ri, ti;
	int* full_arr = (int *)malloc((partner_arr_size+my_arr_size)*sizeof(int));
	mi = ri = ti = my_arr_size-1;
	
	/*
		Last process may have lesser number of elements
		Hence the two cases are handled separately for the sake of merging.
	*/
	if(partner_arr_size == my_arr_size)	
	{
		while(ti>=0)
		{
			if(*(my_keys+mi) >= *(recv_keys+ri))
			{
				*(temp_keys+ti) = *(my_keys+mi);
				ti--;
				mi--;
			}
			else
			{
				*(temp_keys+ti) = *(recv_keys+ri);
				ti--;
				ri--;
			}
		}
	}
	else
	{
		int zi = 0;
		for(zi=0; zi<my_arr_size; zi++)
		{
			*(full_arr + zi) = *(my_keys + zi);
		}
		for(zi=my_arr_size; zi < (my_arr_size+partner_arr_size); zi++)
		{
			*(full_arr + zi) = *(recv_keys + zi-my_arr_size);
		}
		
		qsort(full_arr, my_arr_size+partner_arr_size, sizeof(int), Compare);
		
		for(zi=0; zi<my_arr_size; zi++)
		{
			*(temp_keys + zi) = *(full_arr + partner_arr_size + zi);
		}
		
	}
	
	for(mi=0; mi<my_arr_size; mi++)
	{
		*(my_keys + mi) = *(temp_keys + mi);
	}
	
	free(full_arr);
}

/*
My ceiling function
*/
int ceil_div(int x, int y)
{
	return x/y + (x % y != 0);
}

/*
Compare function for qsort
*/
int Compare(const void* val_a, const void* val_b) {
   int a = *((int*)val_a);
   int b = *((int*)val_b);

   if (a < b)
      return -1;
   else if (a == b)
      return 0;
   else 
      return 1;
}  


/*
Calculates size of array associated with a process.
Processes and their sizes:
	Rank --->   size
	0 to np-2	ceiling(n/np)
	np-1		remaining elements
*/
int get_size_arr(int rank, int np, int n) 
{
	int size;
	if(rank!=np-1)
		size = ceil_div(n, np);
	else
		size = n - (np-1)*ceil_div(n, np);
	return size;
}

/*
Randomly generates elements for local array for each process.
*/
int * generate_local_array(int rank, int n, int np)
{
	srand(rank+7);
	int size = get_size_arr(rank, np, n);
	int* arr = (int *)malloc(size*sizeof(int));
	for(int i=0; i<size; i++)
	{
		*(arr+i) = rand()%MY_RAND_MAX;
	}
	return arr;
}


/*
Prints the global arrays using the local copies
from all the processes.
*/
void print_global_array(int* my_arr, int my_arr_size,
         int my_rank, int np, int n, MPI_Comm comm) {
   int* tmp;
   int tmp_size = my_arr_size;
   int src;
   MPI_Status status;

   if (my_rank == 0)
   {
      tmp = (int*) malloc(my_arr_size*sizeof(int));
      print_array(my_arr, my_arr_size);
      for (src = 1; src < np; src++) {
      	 if(src==np-1)
      	 {
      	 	tmp_size = get_size_arr(src, np, n);
      	 }
         MPI_Recv(tmp, tmp_size, MPI_INT, src, 0, comm, &status);
         print_array(tmp, tmp_size);
      }
      free(tmp);
      printf("\n\n");
   } 
   else 
   {
      MPI_Send(my_arr, my_arr_size, MPI_INT, 0, 0, comm);
   }
} 


/*
Prints array with rank of the process.
*/
void print_array_with_rank(int * arr, int arr_size, int rank) 
{
	
	printf("Printing array of %d ranked process:\n", rank);
	for(int i=0; i<arr_size; i++)
	{
		printf("%d ", *(arr+i));
	}
	printf("\n");
}

/*
Prints array given pointer and size of the array.
*/
void print_array(int * arr, int arr_size) 
{
	for(int i=0; i<arr_size; i++)
	{
		printf("%d ", *(arr+i));
	}
}






























