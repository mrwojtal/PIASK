#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <omp.h>

#include "timers/my_timers.h"

#define SIZE 2000000
#define CUTOFF 4096   // Threshold below which sequential quicksort is used
                     // to avoid excessive task creation overhead

void swap(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

int partition(int* arr, int minIndex, int maxIndex) {
  int i = minIndex-1;
  int j = minIndex;

  do {
    if (arr[j] < arr[maxIndex]) {
      i++;
      swap(&arr[j], &arr[i]); // Move elements smaller than pivot to the left
    }
    j++;
  }
  while(j <= maxIndex-1);

  swap(&arr[i+1], &arr[maxIndex]); // Place pivot in its final position
  return i+1;
}

void quickSortSeq(int* arr, int minIndex, int maxIndex) {
  if (minIndex >= maxIndex) return;
  int pivot = partition(arr, minIndex, maxIndex);
  quickSortSeq(arr, minIndex, pivot-1);
  quickSortSeq(arr, pivot+1, maxIndex);
}

void quickSortTasks(int* arr, int minIndex, int maxIndex) {
  if (minIndex >= maxIndex) return;

  // For small subarrays, use sequential version to reduce task overhead
  if (maxIndex-minIndex <= CUTOFF) {
    return quickSortSeq(arr, minIndex, maxIndex);
  }
  else {
    int pivot = partition(arr, minIndex, maxIndex);

    // Create a new OpenMP task for the left subarray
    // The task may be executed by any available thread
    #pragma omp task shared(arr)
      quickSortTasks(arr, minIndex, pivot-1);

    // Create a new OpenMP task for the right subarray
    #pragma omp task shared(arr)
      quickSortTasks(arr, pivot+1, maxIndex);

    // Wait until both child tasks are completed
    #pragma omp taskwait
  }
}

void quickSortOMP(int* arr, int minIndex, int maxIndex) {
  // Start a parallel region with a team of threads
  #pragma omp parallel
  {
    // Ensure only one thread creates the initial task tree
    // Other threads will execute generated tasks
    #pragma omp single
      quickSortTasks(arr, minIndex, maxIndex);
  }
}

void sortedTest(int* arr, int size) {
  int resultFlag = 1;
  for (int i = 0; i < size-1; i++) {
    if (arr[i] > arr[i+1]) {
      resultFlag = 0;
    }
  }
  if (resultFlag) {
    printf("SUCCESS: Array sorted succesfully\n");
  }
  else {
    printf("FAIL: Array is not sorted\n");
  }
}

int main() {
  srand(time(NULL));

  int arr[SIZE];
  for (int i = 0; i < SIZE; i++) {
    arr[i] = rand()%1000000 + 1;
  }

  // Use the maximum number of available OpenMP threads
  omp_set_num_threads(omp_get_max_threads());

  start_time();
  quickSortOMP(arr, 0, SIZE-1);
  stop_time();

  sortedTest(arr, SIZE);
  print_time("Elapsed:");

  return 0;
}
