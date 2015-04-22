//------------------------------------------------------------------------- 
// This is supporting software for CS415/515 Parallel Programming.
// Copyright (c) Portland State University
//------------------------------------------------------------------------- 

// A simple array-sum program using Pthreads. (Full Version)
//
// Usage: ./arraysum
//
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>		// for getting cpu id
#include <unistd.h>             // for getting nprocs

int arraySize;                  // array size, given by user
int numThreads = 1;             // default number of threads 
int *array;                     // shared array 
int sum = 0, idx = 0;           // global sum and idx 
//pthread_mutex_t sumLock;   

// Initialize array to [1,2,...,size]
//
int *init_array(int size) {
  int *array = (int *) malloc(sizeof(int) * size);
  for (int i = 0; i < size; i++)
    array[i] = i + 1;
  printf("Initialized array to [1,2,...,%d]\n", size);
  return array;
}

// The worker routine
//
void slave(long tid) {
  printf("Thread %ld started on %d\n", tid, sched_getcpu());
  int i, psum = 0;
  do {
//    pthread_mutex_lock(&sumLock);    	// read and increment idx
    i = idx++;                        	
//    pthread_mutex_unlock(&sumLock);     
    if (i < arraySize)                  // add one array element
      psum += array[i];
  } while (i < arraySize);

//  pthread_mutex_lock(&sumLock);      	// add local psum to global sum 
  sum += psum;
//  pthread_mutex_unlock(&sumLock);
} 

// The main routine
//
int main(int argc, char **argv) {
  // check command-line for user input
  if (argc < 2) {
    printf ("Usage: ./arraysum4 <arraySize> [<numThreads>]\n");
    exit(0);
  } else if (argc > 2) {
    if ((numThreads=atoi(argv[2])) < 1) {
      printf ("<numThreads> must be greater than 0\n");
      exit(0);
    }
  }
  if ((arraySize=atoi(argv[1])) < 1) {
    printf ("<arraySize> must be greater than 0\n");
    exit(0);
  }
  pthread_t thread[numThreads];
  array = init_array(arraySize); 	// initialize array 
//  pthread_mutex_init(&sumLock, NULL);   // initialize mutex 

  int nprocs = sysconf(_SC_NPROCESSORS_ONLN);
  cpu_set_t cpuset;
  int cid = 0;
  for (long k = 0; k < numThreads; k++) {        // create threads 
    pthread_create(&thread[k], NULL, (void*)slave, (void*)k);
    CPU_ZERO(&cpuset);
    CPU_SET(cid++ % nprocs, &cpuset);
    pthread_setaffinity_np(thread[k], sizeof(cpu_set_t), &cpuset);
  }
  printf("%d threads created\n", numThreads);

  for (long k = 0; k < numThreads; k++)          // join threads 
    pthread_join(thread[k], NULL);
  printf("%d threads joined\n", numThreads);

  printf("The sum of 1 to %i is %d\n", arraySize, sum);
}  

