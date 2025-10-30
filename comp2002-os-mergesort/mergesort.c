/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <pthread.h>
#include "mergesort.h"

// I dont know if these need ot be extern?? but i had issues with non decleration so this works
extern int *A; // Main array 
extern int *B; // temporary array 
extern int cutoff; // cutoff or otherwise the maximum depth of threads we will create

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){

	if (leftstart > leftend && rightstart > rightend) {
		return; 
	}
	

	int numberOfElementsToCopy = rightend - leftstart + 1;

	// copy a into b (a is the main array which is always sorted, b is the temporary array)
	int i;
	for (i = leftstart; i <= rightend; i++) {
		B[i] = A[i];
	}

	int leftPointer = leftstart; // pointer used for the left subarray
	int rightPointer = rightstart; // pointer used for the right subarray
	int currentPointer = leftstart; // pointer for merged array

	// Functionality used to merge the two sub arrays into A
	while (leftPointer <= leftend && rightPointer <= rightend) {
		// compare each element and move into A
		if (B[leftPointer] <= B[rightPointer]) {
			A[currentPointer++] = B[leftPointer++];
		} else {
			A[currentPointer++] = B[rightPointer++];
		}
	}
	// any remaining elements from either subarray are copied into A
	while (leftPointer <= leftend)  { A[currentPointer++] = B[leftPointer++]; }
	while (rightPointer <= rightend) { A[currentPointer++] = B[rightPointer++]; }
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

	// parse arguments
	struct argument *args = arg;
	int left = args->left;
	int right = args->right;
	int level = args->level;

	if (left >= right) {
		return NULL; // array is size 0 or 1
	}

	if (level >= cutoff) {
		my_mergesort(left, right);
		return NULL;
	}

	int midPointOfArray = left + (right - left) / 2;

	// build arguments for left and right subarrays
	struct argument *leftArgs = buildArgs(left, midPointOfArray, level + 1);
	struct argument *rightArgs = buildArgs(midPointOfArray + 1, right, level + 1);

	// We create and initialize threads for left and right subarrays
	pthread_t leftThread;
	pthread_t rightThread;
	int createLeftThread = pthread_create(&leftThread, NULL, parallel_mergesort, (void *) leftArgs);
	int createRightThread = pthread_create(&rightThread, NULL, parallel_mergesort, (void *) rightArgs);

	// safety in case a thread fails to be created (does it sequentially inside the already running thread?)
	if (createLeftThread != 0) { parallel_mergesort((void *) leftArgs); }
	if (createRightThread != 0) { parallel_mergesort((void *) rightArgs); }


	// wait for both child threads to finish
	if (createLeftThread == 0) {
		pthread_join(leftThread, NULL);
	}
	if (createRightThread == 0) {
		pthread_join(rightThread, NULL);
	}
	
	// free the arguments and merge the two sorted subarrays
	free(leftArgs);
	free(rightArgs);
	merge(left, midPointOfArray, midPointOfArray + 1, right);
	
	return NULL;

}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
	// allocate correct memory for args
	// This is very important!! it retuns a pointer so if any stack values (left, right, level) are incorrecly sized or go out of scope it will
	// make the whole function have undefined behaviour.
	struct argument *args = calloc(1, sizeof(struct argument));
	args->left = left;
	args->right = right;
	args->level = level;
	return args;
}

