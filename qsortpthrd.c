// A global array of size N contains the integers to be sorted.
// A global task queue is initialized with the sort range [0,N-1].

int main(int argc, char **argv) {
	// read in command-line arguments, N and numThreads;

	// initialize array, queue, and other shared variables

	// create numThreads-1 worker threads, each executes a copy
	// of the worker() routine; each copy has an integer id,
	// ranging from 0 to numThreads-2.
	//
	for (long k = 0; k < numThreads-1; k++)
	pthread_create(&thread[k], NULL, (void*)worker, (void*)k);

	// the main thread also runs a copy of the worker() routine;
	// its copy has the last id, numThreads-1
	worker(numThreads-1);

	// the main thread waits for worker threads to join back
	for (long k = 0; k < numThreads-1; k++)
	pthread_join(thread[k], NULL);

	// verify the result
	verify_array(array, N);
}

void worker(long wid) {
	while (<termination condition> is not met) {
		task = remove_task();
		quicksort(array, task->low, task->high);
	}
}
