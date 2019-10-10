#include <stdio.h>
#include <stdlib.h>
#include "omp.h"

#define MY_RAND_MAX 100


/*
** Matrix multiplication 
** Input : Matrix A, Matrix B
** Output : Matrix C = A*B
**
** First the multiplication is performed in serial fashion
** and then parallelly.
** 
*/


//A function to print a matrix, given address and size
void print_mat(double* A, int m, int n)
{
	for(int i=0; i<m; i++)
	{
		for(int j=0; j<n; j++)
			printf("%lf ", *(A + m*i + j));
		printf("\n");
	}
	printf("\n\n");
}



void main(int argc, char **argv)
{

	double *A, *B, *C;	/* A[l][m], B[m][n], C[l][n] */
	int l, m, n;		
	double t_begin, t_parallel, t_serial;
	int num_threads;     /* Number of threads can be specified by user */
	
	int i, j, k;
	double tmp;
	
	l= atoi(argv[1]);
	m= atoi(argv[2]);
	n= atoi(argv[3]);
	num_threads = atoi(argv[4]);
	
	
	A = (double *)malloc(l*m*sizeof(double));
	B = (double *)malloc(m*n*sizeof(double));
	C = (double *)malloc(l*n*sizeof(double));
	
	
	//generate random array elements
	//generate A
	for(i=0; i<l; i++)
	{
		for( j=0; j<m; j++)
			*(A + l*i + j) = rand()%MY_RAND_MAX;
	}
	//generate B
	for( i=0; i<m; i++)
	{
		for( j=0; j<n; j++)
			*(B + m*i + j) = rand()%MY_RAND_MAX;
	}
	
	//print A and B
	printf("Matrix A:\n");
	print_mat(A, l, m);
	printf("Matrix B:\n");
	print_mat(B, m, n);
	
	//multiply A and B serially
	t_begin = omp_get_wtime();  //start timer
	
	for( i=0; i<l; i++)
	{
		for( j=0; j<n; j++)
		{
			tmp = 0;
			for( k=0; k<m; k++)
			{
				tmp += *(A + l*i + k) * *(B + m*k + j);
			}
			*(C + l*i + j) = tmp;
		}
			
	}
	
	t_serial = omp_get_wtime() - t_begin;
	printf("Total time taken in serial computation = %lf\n", t_serial);
	
	//printing resultant C after serial multiplication
	printf("Resultant matrix C:\n");
	print_mat(C, l, n);
	
	
	

	//parallel matrix multiplication
	omp_set_num_threads(num_threads);
	t_begin = omp_get_wtime();
	
	#pragma omp parallel for private(i, j, k, tmp)
	for(i=0; i<l; i++)
	{
		for(j=0; j<n; j++)
		{
			tmp = 0;
			for(k=0; k<m; k++)
			{
				tmp += *(A + l*i + k) * *(B + m*k + j);
			}
			*(C + l*i + j) = tmp;
		}
			
	}
	
	t_parallel = omp_get_wtime() - t_begin;
	printf("Total time taken in computing parallelly = %lf\n", t_parallel);
	
	//printing resultant C after parallel multiplication
	printf("Resultant matrix C:\n");
	print_mat(C, l, n);
		
	
}
