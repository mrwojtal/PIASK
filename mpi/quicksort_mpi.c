#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <mpi.h>

#include "timers/my_timers.h"

#define SIZE 2000000
#define CUTOFF 4096   // Not used here, kept for consistency with other versions

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
            swap(&arr[j], &arr[i]); // Move smaller elements to the left of the pivot
        }
        j++;
    } while (j <= maxIndex - 1);
    swap(&arr[i + 1], &arr[maxIndex]); // Place pivot in its final position
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

    // Merge two sorted subarrays: [left, mid) and [mid, right)
    while (i < mid && j < right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i < mid) temp[k++] = arr[i++];
    while (j < right) temp[k++] = arr[j++];

    // Copy merged result back to the original array
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
    for (int i = 0; i < SIZE; i++)
        arr[i] = rand() % 1000000 + 1;

    int myrank, mysize;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Get rank of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Get total number of MPI processes
    MPI_Comm_size(MPI_COMM_WORLD, &mysize);

    start_time();

    // Compute local array size per process
    int local_n = SIZE / mysize;

    // Allocate buffer for local chunk
    int* local_arr = (int*)malloc(local_n * sizeof(int));

    // Distribute equal-sized chunks of the array to all processes
    // Only rank 0 sends data, all ranks receive their local part
    MPI_Scatter(arr, local_n, MPI_INT,
                local_arr, local_n, MPI_INT,
                0, MPI_COMM_WORLD);

    // Each process sorts its local chunk sequentially
    quickSortSeq(local_arr, 0, local_n - 1);

    // Gather sorted chunks back to the root process (rank 0)
    MPI_Gather(local_arr, local_n, MPI_INT,
               arr, local_n, MPI_INT,
               0, MPI_COMM_WORLD);

    // Final merge of sorted chunks performed only by rank 0
    if (myrank == 0) {
        int* temp = (int*)malloc(SIZE * sizeof(int));
        int step = local_n;

        // Iteratively merge larger and larger sorted blocks
        while (step < SIZE) {
            for (int i = 0; i < SIZE; i += 2 * step) {
                int left = i;
                int mid = i + step;
                int right = i + 2 * step;

                if (mid < SIZE)
                    merge(arr, temp, left, mid,
                          (right < SIZE ? right : SIZE));
            }
            step *= 2;
        }
        free(temp);
    }

    free(local_arr);

    // Synchronize all processes before stopping the timer
    MPI_Barrier(MPI_COMM_WORLD);

    stop_time();

    // Only root process checks correctness and prints timing
    if (myrank == 0) {
        sortedTest(arr, SIZE);
        print_time("Elapsed:");
    }

    // Clean up MPI environment
    MPI_Finalize();
    return 0;
}
