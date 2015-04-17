// A global array of size N contains the integers to be sorted.
// A global task queue is initialized with the sort range [0,N-1].

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

//-------------------------------
// Task and queue representations
//-------------------------------

typedef struct task_ task_t;
struct task_ {
  int low;
  int high;
  task_t *next;         // and a pointer to next task
}; 

typedef struct queue_ queue_t;
struct queue_ {
  task_t *head;   
  task_t *tail;
  int limit;    // queue size limit
  int length;   // current number of tasks
} quick_queue; 

int debug = 1;


//-------------------------------
// Global initializations
//-------------------------------

int *array = NULL;
int N = 0;
int completed = 0;
int MIN_SIZE = 10;

pthread_mutex_t queue_mutex;
pthread_mutex_t completed_mutex;
pthread_cond_t queue_cond;

//-------------------------------
// Prototypes
//-------------------------------

void push_queue(task_t *task);

//-------------------------------
// Functions
//-------------------------------

int check_completed() {
  pthread_mutex_lock(&completed_mutex);
  int complete_check = (completed == N);
  pthread_mutex_unlock(&completed_mutex); 
  return complete_check;
}

void add_completed(int num) {
  pthread_mutex_lock(&completed_mutex);
  completed += num;
  pthread_mutex_unlock(&completed_mutex);
}

void swap(int x, int y) {
  int tmp = array[x];
  array[x] = array[y];
  array[y] = tmp;
}

void bubble_sort(int low, int high) {
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

  add_completed(1);

  return(middle);
}

void add_task(int low, int high) {
  task_t *t = malloc(sizeof(task_t));
  t->low = low;
  t->high = high;
  pthread_mutex_lock(&queue_mutex);
  push_queue(t);
  pthread_cond_signal(&queue_cond);
  pthread_mutex_unlock(&queue_mutex);
  if (debug) { printf("Added %d to %d.\n", low, high); }
}

void push_queue(task_t *task) {
  if (quick_queue.tail == NULL) {
    quick_queue.tail = task;
    quick_queue.head = task;
  } else {
    quick_queue.tail->next = task;
    quick_queue.tail = quick_queue.tail->next;
  }
}

task_t* pop_task() {
  if (quick_queue.tail == NULL) {
    return NULL;
  }
  task_t *temp_task = quick_queue.head;
  quick_queue.head = quick_queue.head->next;
  if (quick_queue.head == NULL) {
    quick_queue.tail == quick_queue.head;
  }
  return temp_task;
}

void quicksort(int low, int high) {
  if (high - low < MIN_SIZE) {
    bubble_sort(low, high);
    return;
  }
  int middle = partition(low, high);
  add_task(low, middle - 1);
  quicksort(middle + 1, high);
}
  

void worker(long wid) {
  if (debug) { printf("Worker %ld started.\n", wid); }
  while (!check_completed()) {
    task_t *task = pop_task();
    quicksort(task->low, task->high);
  }
}

//-------------------------------
// Main
//-------------------------------
int main(int argc, char **argv) {
  // read in command-line arguments, N and numThreads;
  int numThreads = 4;
  int N = 10;
  int thread[numThreads];

  // initialize array, queue, and other shared variables

  // create numThreads-1 worker threads, each executes a copy
  // of the worker() routine; each copy has an integer id,
  // ranging from 0 to numThreads-2.
  //
  for (long k = 0; k < numThreads - 1; k++) {
    pthread_create(&thread[k], NULL, (void*)worker, (void*)k);
  }

  // the main thread also runs a copy of the worker() routine;
  // its copy has the last id, numThreads-1
  worker(numThreads - 1);

  // the main thread waits for worker threads to join back
  for (long k = 0; k < numThreads - 1; k++) {
    pthread_join(thread[k], NULL);
  }

  // verify the result
  verify_array(array, N);
}

