#----------------------------------------------------------------------
# CS415/515 Assignment 1
#----------------------------------------------------------------------

FLAGS = -std=c99 -pthread -g
CC = gcc

arraysum: arraysum.c
	$(CC) $(FLAGS) -o arraysum arraysum.c

prodcons: prodcons.c
	$(CC) $(FLAGS) -o prodcons prodcons.c

qsort: qsort.c
	$(CC) -std=c99 -g -o qsort qsort.c

qsortpthd: qsortpthd.c
	clear
	clear
	$(CC) $(FLAGS) -o qsortpthd qsortpthd.c

