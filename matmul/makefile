CC=gcc  #compiler
TARGET=matrix #target file name
OPMPFLAG=-fopenmp #open mp flag
 
all:	matrix.o 
	$(CC) matrix.c $(OPMPFLAG) -o $(TARGET)
 
clean:
	rm *.o $(TARGET)
