/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include <pthread.h>
#include "mergesort.h"

// needs to be extern as theyredeclated in h and need to be used here
extern int *A; // Main array being sorted
extern int *B; // temporary array used to support when we merge the subarrays
extern int cutoff; // cutoff or otherwise the maximum depth of threads we will create

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){

	if (leftstart > leftend && rightstart > rightend) {
		return; // no merging needed
	}

	int numberOfElementsToCopy = rightend - leftstart + 1;

	// copy a into b
	int i;
	for (i = leftstart; i <= rightend; i++) {
		B[i] = A[i];
	}

	int leftPointer = leftstart;
	int rightPointer = rightstart;
	int currentPointer = leftstart;

	while (leftPointer <= leftend && rightPointer <= rightend) {
		if (B[leftPointer] <= B[rightPointer]) {
			A[currentPointer++] = B[leftPointer++];
		} else {
			A[currentPointer++] = B[rightPointer++];
		}
	}

	while (leftPointer <= leftend)  {
		A[currentPointer++] = B[leftPointer++];
	}
	while (rightPointer <= rightend) {
		A[currentPointer++] = B[rightPointer++];
	}
}

/* this function will be called by parallel_mergesort() as its base case. */
void my_mergesort(int left, int right){
	if (left >= right) { return; } // base case

	int mid = left + (right - left) / 2;

	my_mergesort(left, mid);
	my_mergesort(mid + 1, right);
	merge(left, mid, mid + 1, right);
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
	struct argument *args = arg;

	int left = args->left;
	int right = args->right;
	int level = args->level;

	if (left >= right) {
		return NULL; // base case: array of size 0 or 1
	}

	if (level >= cutoff) {
		my_mergesort(left, right);
		return NULL;
	}

	int midPointOfArray = left + (right - left) / 2;

	struct argument *leftArgs = buildArgs(left, midPointOfArray, level + 1);
	struct argument *rightArgs = buildArgs(midPointOfArray + 1, right, level + 1);

	pthread_t leftThread;
	pthread_t rightThread;

	int createLeftThread = pthread_create(&leftThread, NULL, parallel_mergesort, (void *) leftArgs);
	int createRightThread = pthread_create(&rightThread, NULL, parallel_mergesort, (void *) rightArgs);

	if (createLeftThread != 0) {
		// Failed to create left thread; fall back to sequential sort
		parallel_mergesort((void *) leftArgs);
	}
	if (createRightThread != 0) {
		// Failed to create right thread; fall back to sequential sort
		parallel_mergesort((void *) rightArgs);
	}


	// wait for threads
	if (createLeftThread == 0) {
		pthread_join(leftThread, NULL);
	}
	if (createRightThread == 0) {
		pthread_join(rightThread, NULL);
	}

	free(leftArgs);
	free(rightArgs);


	merge(left, midPointOfArray, midPointOfArray + 1, right);
	return NULL;

}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
	struct argument *args = calloc(1, sizeof(struct argument));
	args->left = left;
	args->right = right;
	args->level = level;
	return args;
}

