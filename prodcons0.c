//------------------------------------------------------------------------- 
// This is supporting software for CS415/515 Parallel Programming.
// Copyright (c) Portland State University
//------------------------------------------------------------------------- 

// A starter version for the producer-consumer program.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//-------------------------------
// Task and queue representations
//-------------------------------

typedef struct task_ task_t;
struct task_ {
  int val;		// each task holds an integer value
  task_t *next;	        // and a pointer to next task
}; 

typedef struct queue_ queue_t;
struct queue_ {
  task_t *head;		
  task_t *tail;
  int limit;		// queue size limit
  int length;		// current number of tasks
}; 

//--------------------------------
// Task and queue related routines
//--------------------------------

// Create a new task
//
task_t *create_task(int val) {
  task_t *task = (task_t *) malloc(sizeof(task_t));
  task->val = val;
  task->next = NULL;
  return task;
}

// Create the task queue
//
queue_t *init_queue(int limit) {
  queue_t *queue = (queue_t *) malloc(sizeof(queue_t));
  queue->head = queue->tail = NULL;
  queue->limit = limit;
  queue->length = 0;
  return queue;
}

// Add a task to the tail of the queue
//
void add_task(queue_t *queue, task_t *task) {
  if (!queue->tail) {
    queue->head = queue->tail = task;
  } else {
    queue->tail->next = task;
    queue->tail = task;
  }
  queue->length++;
}

// Remove a task from the head of the queue
// (Return null if the queue is empty)
//
task_t *remove_task(queue_t *queue) {
  task_t *task = NULL;
  if (queue->length > 0) {
    task = queue->head;
    if (queue->head == queue->tail)
      queue->head = queue->tail = NULL;
    else
      queue->head = queue->head->next;
    queue->length--;
  }
  return task;
}

//-------------------------------
// Shared global variables
//-------------------------------

#define BUFSIZE  20 		// the queue size
#define NUMTASKS 100		// the total number of tasks to create

int numConsumers = 1; 		// number of consumers (default to 1)
queue_t *queue; 		// the main task queue
pthread_mutex_t queueLock; 	// mutex lock for the queue


// The producer routine
// 
void producer() { 
   
  // ... need code ...

}


// The consumer routine
// 
void consumer(long tid) { 
   
  // ... need code ...

}

// The main routine
// 
int main(int argc, char **argv) {
   
  // ... need code ...

}
