#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <mpi.h>

#include "timers/my_timers.h"

#define SIZE 2000000
#define CUTOFF 4096

void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int partition(int* arr, int minIndex, int maxIndex) {
    int i = minIndex - 1;
    int j = minIndex;
    do {
        if (arr[j] < arr[maxIndex]) {
            i++;
            swap(&arr[j], &arr[i]);
        }
        j++;
    } while (j <= maxIndex - 1);
    swap(&arr[i + 1], &arr[maxIndex]);
    return i + 1;
}

void quickSortSeq(int* arr, int minIndex, int maxIndex) {
    if (minIndex >= maxIndex) return;
    int pivot = partition(arr, minIndex, maxIndex);
    quickSortSeq(arr, minIndex, pivot - 1);
    quickSortSeq(arr, pivot + 1, maxIndex);
}

void merge(int* arr, int* temp, int left, int mid, int right) {
    int i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i < mid) temp[k++] = arr[i++];
    while (j < right) temp[k++] = arr[j++];
    for (i = left; i < right; i++) arr[i] = temp[i];
}

void sortedTest(int* arr, int size) {
    int resultFlag = 1;
    for (int i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) resultFlag = 0;
    }
    if (resultFlag) printf("SUCCESS: Array sorted succesfully\n");
    else printf("FAIL: Array is not sorted\n");
}

int main(int argc, char** argv) {
    srand(time(NULL));

    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) arr[i] = rand() % 1000000 + 1;

    int myrank, mysize;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &mysize);

    start_time();

    int local_n = SIZE / mysize;
    int* local_arr = (int*)malloc(local_n * sizeof(int));

    // Scatter
    MPI_Scatter(arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Local sort
    quickSortSeq(local_arr, 0, local_n - 1);

    // Gather sorted chunks to root
    MPI_Gather(local_arr, local_n, MPI_INT, arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Merge at rank 0
    if (myrank == 0) {
        int* temp = (int*)malloc(SIZE * sizeof(int));
        int step = local_n;
        while (step < SIZE) {
            for (int i = 0; i < SIZE; i += 2 * step) {
                int left = i;
                int mid = i + step;
                int right = i + 2 * step;
                if (mid < SIZE)
                    merge(arr, temp, left, mid, (right < SIZE ? right : SIZE));
            }
            step *= 2;
        }
        free(temp);
    }

    free(local_arr);

    MPI_Barrier(MPI_COMM_WORLD);
    stop_time();

    if (myrank == 0) {
        sortedTest(arr, SIZE);
        print_time("Elapsed:");
    }

    MPI_Finalize();
    return 0;
}
