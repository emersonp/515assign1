// A global array of size N contains the integers to be sorted.
// A global task queue is initialized with the sort range [0,N-1].

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

//-------------------------------
// Task and queue representations
//-------------------------------

typedef struct task_t {
  int low;
  int high;
  struct task_t *next;         // and a pointer to next task
} task_t; 

typedef struct queue_t
{
  task_t *head;   
  task_t *tail;
  int length;   // current number of tasks
} queue_t; 

int debug = 1;


//-------------------------------
// Global initializations
//-------------------------------

int *array = NULL;
int N = 0;
int completed = 0;
int MIN_SIZE = 10;
queue_t quick_queue;

pthread_mutex_t queue_mutex;
pthread_mutex_t array_mutex;
pthread_mutex_t completed_mutex;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

//-------------------------------
// Prototypes
//-------------------------------

void push_queue(task_t *task);
void print_array();

//-------------------------------
// Functions
//-------------------------------

int check_completed() {
  pthread_mutex_lock(&completed_mutex);
  int complete_check = (completed == N);
  if (debug) { if (complete_check) { printf("Complete Check Passed!\n"); } }
  pthread_mutex_unlock(&completed_mutex); 
  return complete_check;
}

void add_completed(int num) {
  pthread_mutex_lock(&completed_mutex);
  if (debug) { printf("Completed: %d = %d + %d.\n", completed + num, completed, num); }
  completed += num;
  pthread_mutex_unlock(&completed_mutex);
}

void swap(int x, int y) {
  pthread_mutex_lock(&array_mutex);
  int tmp = array[x];
  array[x] = array[y];
  array[y] = tmp;
  pthread_mutex_unlock(&array_mutex);
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
  //task_t t = {.low = low, .high = high}
  t->low = low;
  t->high = high;
  t->next = NULL;
  if (debug) { printf("Created task %d to %d.\n", low, high); }
  pthread_mutex_lock(&queue_mutex);
  push_queue(t);
  pthread_mutex_unlock(&queue_mutex);
  //pthread_cond_signal(&queue_cond);
}

void push_queue(task_t *task) {
  if (debug) { printf("Pushing task (tail) %d to %d.\n", task->low, task->high); }
  if (quick_queue.tail == NULL) {
    quick_queue.tail = task;
    quick_queue.head = task;
    pthread_cond_broadcast(&queue_cond);
  } else {
    quick_queue.tail->next = task;
    quick_queue.tail = quick_queue.tail->next;
    quick_queue.length++;
  }
  if (debug) { printf("Current queue head: %d to %d.\n", quick_queue.head->low, quick_queue.head->high); }
  if (debug) { printf("Current queue tail: %d to %d.\n", quick_queue.tail->low, quick_queue.tail->high); }
}

task_t* pop_task() {
  if (debug) { printf("Popping task (head) %d to %d.\n", quick_queue.head->low, quick_queue.head->high); }

  task_t *temp_task = quick_queue.head;
  quick_queue.head = quick_queue.head->next;
  if (quick_queue.head == NULL) {
    quick_queue.tail = NULL;
  }
  quick_queue.length--;
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
  int loopcount = 0;
  if (debug) { printf("-->Worker %ld started.\n", wid); }
  while (!check_completed()) {
    loopcount++;
    if (loopcount > 1) { printf("##### MULTIPLE LOOPS.\n"); }

    task_t *task = malloc(sizeof(task_t));
    
    pthread_mutex_lock(&queue_mutex);
    
    while ( quick_queue.head == NULL && !check_completed() ) {
      if (debug) { printf("Waiting on queue condition.\n"); }
      pthread_cond_wait(&queue_cond, &queue_mutex);
    }
    task = pop_task();
    
    pthread_mutex_unlock(&queue_mutex);
    
    if (debug) { printf("Worker %ld starting on range %d to %d.\n", wid, task->low, task->high); }
    
    quicksort(task->low, task->high);
    
    free(task);
  }
}

void verify_array() {
  if (debug) { printf("Verifying array...\n"); }
  for (int index = 0; index < N - 1; index++) {
    if (array[index] > array[index + 1]) {
      printf("ERROR: Array[%d] == %d, Array[%d] == %d.\n", index, array[index], index + 1, array[index + 1]) ;
      return;
    }
  }
  printf("Success!\n");
  print_array();
}

void randomize_array() {
  for (int index = 0; index < N; index++) {
    array[index] = rand() % N;
  }
  print_array();
}

void print_array() {
  for (int index = 0; index < N; index++) {
    printf("%d ", array[index]);
  }
  printf("\n\n");
}

//-------------------------------
// Main
//-------------------------------
int main(int argc, char **argv) {
  // read in command-line arguments, N and numThreads;
  int numThreads = 4;
  N = 100;
  pthread_t thread[numThreads];

  // initialize array, queue, and other shared variables
  array = malloc(N * sizeof(int));
  randomize_array();

  task_t first_task;
  first_task.low = 0;
  first_task.high = N - 1;
  first_task.next = NULL;
  
  quick_queue.head = &first_task;
  quick_queue.tail = &first_task;

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
  verify_array();
}
