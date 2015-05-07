// Copyright (c) 2015 Parker Harris Emerson
// Homework 1 for Prof. Li's Parallel Programming Course

// A global array of size N contains the integers to be sorted.
// A global task queue is initialized with the sort range [0,N-1].

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>    // for getting cpu id

//-------------------------------
// Debug Variable, for TA
//-------------------------------

int debug = 0;
int arrayprinting = 0;

//-------------------------------
// Task and queue representations
//-------------------------------

typedef struct task_t {
  int low;
  int high;
  struct task_t *next;         // and a pointer to next task
} task_t; 

typedef struct queue_ queue_t;
struct queue_
{
  task_t *head;   
  task_t *tail;
  int length;   // current number of tasks
} ; 

//-------------------------------
// Global initializations
//-------------------------------

int *array = NULL;
int arraySize = 0;
int completed = 0;
int MIN_SIZE = 10;
int numThreads = 1;
queue_t quick_queue = { .head = NULL, .tail = NULL, .length = 0 };

pthread_mutex_t queue_mutex;
pthread_mutex_t completed_mutex;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

//-------------------------------
// Prototypes
//-------------------------------

void push_queue(task_t *task);
void print_array();
int sched_getcpu();

//-------------------------------
// Functions
//-------------------------------

int check_completed() {
  int complete_check;
  pthread_mutex_lock(&completed_mutex);
  complete_check = (completed == arraySize);
  if (debug) {
    if (complete_check) {
      printf("Complete Check Passed!\n");
      print_array();
    }
  }
  pthread_mutex_unlock(&completed_mutex);
  pthread_cond_broadcast(&queue_cond);
  return complete_check;
}

void add_completed(int num) {
  pthread_mutex_lock(&completed_mutex);
  if (debug) { printf("Completed: %d = %d + %d.\n", completed + num, completed, num); }
  completed += num;
  pthread_mutex_unlock(&completed_mutex);
}

void swap(int x, int y) {
	if (x == y) {
		return;
	}
  int tmp = array[x];
  array[x] = array[y];
  array[y] = tmp;
}

void bubble_sort(int low, int high) {
  if (debug) { printf("Bubbling %d to %d.\n", low, high); }
  if (low > high) return;
  int index1, index2;
  for (index1 = low; index1 <= high; index1++) {
    for (index2 = index1 + 1; index2 <= high; index2++) {
      if (array[index1] > array[index2]) {
        swap(index1, index2);
      }
    }
  }
  add_completed(high - low + 1);
}

int partition(int low, int high) {
  int pivot = array[low], middle = low, index;
 
  swap(low, high);

  for (index = low; index < high; index++) {
    if (array[index] < pivot) {
      swap(index, middle);
      middle++;
    }
  }
  swap(high, middle);

  if (debug) { printf("Adding partition of %d to completed.\n", middle); }
  add_completed(1);

  return(middle);
}

void add_task(int low, int high) {
  task_t *t = malloc(sizeof(task_t));
  t->low = low;
  t->high = high;
  t->next = NULL;
  if (debug) { printf("Created task %d to %d.\n", low, high); }
  push_queue(t);
}

// Push Task onto Queue
void push_queue(task_t *task) {
  if (debug) { printf("Pushing task (tail) %d to %d.\n", task->low, task->high); }
  
  int broadcast = 0;
  pthread_mutex_lock(&queue_mutex);

  if (quick_queue.tail == NULL) {
    broadcast = 1;
    quick_queue.tail = quick_queue.head = task;
  } else {
    quick_queue.tail->next = task;
    quick_queue.tail = quick_queue.tail->next;
  }

  quick_queue.length++;

  if (debug) {
    printf("Queue Length: %d\n", quick_queue.length);
    printf("Current queue head: %d to %d.\n", quick_queue.head->low, quick_queue.head->high);
    printf("Current queue tail: %d to %d.\n", quick_queue.tail->low, quick_queue.tail->high);
  }
  
  pthread_mutex_unlock(&queue_mutex);
  
  if (broadcast) {
    if (debug) { printf("Broadcast queue is free.\n"); }
    pthread_cond_broadcast(&queue_cond);
  }
}

// Pop Task, queue_mutex should be locked
task_t* pop_task() {

  task_t *temp_task = NULL;
  if (quick_queue.length > 0)
    {
    if (debug) { printf("Popping task (head) %d to %d.\n", quick_queue.head->low, quick_queue.head->high); }
  
    temp_task = quick_queue.head;
    if (quick_queue.head == quick_queue.tail) {
      quick_queue.head = quick_queue.tail = NULL;
    } else {
    quick_queue.head = quick_queue.head->next;
    }
  quick_queue.length--;
  }
  if (debug) { printf("Queue Length: %d\n", quick_queue.length); }
  return temp_task;
}

// Quicksort
void quicksort(int low, int high) {
  if (high - low < MIN_SIZE) {
    bubble_sort(low, high);
    return;
  }
  int middle = partition(low, high);
  add_task(low, middle - 1);
  quicksort(middle + 1, high);
}

// Worker
void worker(long wid) {
  //printf("------>Worker %ld started on CPU %d.\n", wid, sched_getcpu() );
  while (!check_completed()) {

    pthread_mutex_lock(&queue_mutex);
    
    while ( quick_queue.length == 0 && !check_completed() ) {
      if (debug) { printf("Waiting on queue condition.\n"); }
      pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    task_t *task = pop_task();
    
    pthread_mutex_unlock(&queue_mutex);
    
    if (task) {
      if (debug) { printf("Worker %ld starting on range %d to %d.\n", wid, task->low, task->high); }
    
      quicksort(task->low, task->high);
    
      free(task);
    }
  }
}

// Array Functions
void verify_array() {
  if (debug) { printf("Verifying array...\n"); }
  for (int index = 0; index < arraySize - 1; index++) {
    if (array[index] > array[index + 1]) {
      printf("ERROR: Array[%d] == %d, Array[%d] == %d.\n", index, array[index], index + 1, array[index + 1]) ;
      return;
    }
  }
  printf("\nSuccess!\n\n");
  print_array();
}

void randomize_array() {
  srand( time(NULL) );
  for (int index = 0; index < arraySize; index++) {
    array[index] = rand() % arraySize;
  }
  print_array();
}

void print_array() {
  if (arrayprinting) {
		if (!check_completed()) {
			printf("\nRandomized Array\n\n");
		} else {
			printf("\nSorted Array\n\n");
		}
		for (int index = 0; index < arraySize; index++) {
			printf("%d ", array[index]);
		}
		printf("\n\n");
	}
}

//-------------------------------
// Main
//-------------------------------
int main(int argc, char **argv) {
  // read in command-line arguments, N and numThreads;

	if (argc < 2) {
		printf("Usage: ./qsortpthd <arraySize> [<numThreads>]\n");
		exit(0);
	} else if (argc > 2) {
		if ((numThreads = atoi(argv[2])) < 1) {
			printf("<numThreads> must be greater than 0.\n");
			exit(0);
		}
	}
	if ((arraySize = atoi(argv[1])) < 1) {
		printf("<arraySize> must be greater than 0.\n");
		exit(0);
	}
  pthread_t thread[numThreads];

  // initialize array, queue, and other shared variables
  array = malloc(arraySize * sizeof(int));
  randomize_array();

  task_t *first_task = malloc(sizeof(task_t));
  first_task->low = 0;
  first_task->high = arraySize - 1;
  first_task->next = NULL;
  
  quick_queue.head = first_task;
  quick_queue.tail = first_task;
  quick_queue.length = 1;

  // create numThreads-1 worker threads, each executes a copy
  // of the worker() routine; each copy has an integer id,
  // ranging from 0 to numThreads-2.
  //

  for (long k = 0; k < numThreads - 1; k++) {
    pthread_create(&thread[k], NULL, (void*) worker, (void*)k);
  }

  // the main thread also runs a copy of the worker() routine;
  // its copy has the last id, numThreads-1
	printf("\n\n");
  worker(numThreads - 1);

  // the main thread waits for worker threads to join back
  for (long k = 0; k < numThreads - 1; k++) {
    pthread_join(thread[k], NULL);
  }

  // verify the result
  verify_array();

  // Clean up
  free(array);
}

