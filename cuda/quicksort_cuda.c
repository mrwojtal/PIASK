#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "timers/my_timers.h"

#define SIZE 2000000

void swap(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

int partition(int* arr, int minIndex, int maxIndex) {
  //initial pivot is maxIndex
  //int pivot = arr[maxIndex];
  int i = minIndex-1;
  int j = minIndex;
  
  do {
    if (arr[j] < arr[maxIndex]) {
      i++;
      swap(&arr[j], &arr[i]); //place elements higher than pivot at the right side of the pivot
    }
    j++;
  }
  while(j <= maxIndex-1);
  swap(&arr[i+1], &arr[maxIndex]); //place pivot in right spot
  return i+1; //return pivot index
}

void quickSort(int* arr, int minIndex, int maxIndex) {
  if (minIndex >= maxIndex) return;
  int pivot = partition(arr, minIndex, maxIndex);
  quickSort(arr, minIndex, pivot-1);
  quickSort(arr, pivot+1, maxIndex);
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
  srand(time(NULL)); //init pseudo random seed
  
  int arr[SIZE];
  for (int i = 0; i < SIZE; i++) {
    arr[i] = rand()%1000000 + 1; // numbers in range 1-1000000
  }
  
  /*SORTING START*/
  start_time();
  quickSort(arr, 0, SIZE-1);
  stop_time();
  /*SORTING END*/
  
  sortedTest(arr, SIZE);
  
  print_time("Elapsed:");
  
  return 0;
}