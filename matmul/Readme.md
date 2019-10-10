Matrix.c performs matrix multiplication, first serially and then parallelly using openMP.

It takes command line input in the format:
	filename l m n num_threads
	
	such that C = A*B is generated with sizes
		A[l][m], B[m][n], C[l][n].
		
	Example: ./matrix 2 3 4 10
		
Also number of threads can be spcified in the above input format.

The program also displays the time required for serial as well as parallel computation.
