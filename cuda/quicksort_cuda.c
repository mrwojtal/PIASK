#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

extern "C" {
    #include "my_timers.h"
}

#define SIZE 2000000

// Helper function to check for CUDA errors
void checkCudaError(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA Error: %s: %s\n", msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

__device__ void swap_device(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

__device__ int partition_device(int* arr, int minIndex, int maxIndex) {
    int i = minIndex - 1;
    int j = minIndex;

    // pivotValue jako ostatni element
    int pivotValue = arr[maxIndex];

    do {
        if (arr[j] < pivotValue) {
            i++;
            swap_device(&arr[j], &arr[i]);
        }
        j++;
    } while (j <= maxIndex - 1);

    swap_device(&arr[i + 1], &arr[maxIndex]);
    return i + 1;
}

// Pomocnicza funkcja sortująca małe fragmenty lokalnie bez tworzenia nowych kerneli
__device__ void selectionSort_device(int* arr, int size) {
    for (int i = 0; i < size - 1; ++i) {
        int minIdx = i;
        for (int j = i + 1; j < size; ++j) {
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        swap_device(&arr[i], &arr[minIdx]);
    }
}

__global__ void quickSortKernel(int* arr, int minIndex, int maxIndex) {

    if (minIndex >= maxIndex) return;

    while (minIndex < maxIndex) {
        int size = maxIndex - minIndex + 1;
        if (size <= 32) {
            selectionSort_device(arr + minIndex, size);
            return;
        }

        int pivot = partition_device(arr, minIndex, maxIndex);

        int leftSize = pivot - minIndex;     
        int rightSize = maxIndex - pivot;   

        if (leftSize < rightSize) {
            if (leftSize > 0) {
                quickSortKernel<<<1, 1>>>(arr, minIndex, pivot - 1);
            }
            minIndex = pivot + 1;
        } else {
            if (rightSize > 0) {
                quickSortKernel<<<1, 1>>>(arr, pivot + 1, maxIndex);
            }
            maxIndex = pivot - 1;
        }

    }
}

void sortedTest(int* arr, int size) {
    int resultFlag = 1;
    for (int i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            resultFlag = 0;
            break;
        }
    }
    if (resultFlag) {
        printf("SUCCESS: Array sorted successfully\n");
    } else {
        printf("FAIL: Array is not sorted\n");
    }
}

int main() {
    srand((unsigned)time(NULL));
    
    int dev = 0;
    cudaDeviceProp prop;
    checkCudaError(cudaGetDevice(&dev), "cudaGetDevice");
    checkCudaError(cudaGetDeviceProperties(&prop, dev), "cudaGetDeviceProperties");
    printf("GPU: %s (compute capability %d.%d)\n", prop.name, prop.major, prop.minor);
    if (prop.major < 3 || (prop.major == 3 && prop.minor < 5)) {
        fprintf(stderr, "Error: GPU does not support Dynamic Parallelism (requires compute capability >= 3.5)\n");
        return EXIT_FAILURE;
    }

    int* h_arr = (int*)malloc(SIZE * sizeof(int));
    if (!h_arr) {
        fprintf(stderr, "Host malloc failed\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < SIZE; i++) {
        h_arr[i] = rand() % 1000000 + 1;
    }

    printf("Sorting array of size %d using CUDA Dynamic Parallelism...\n", SIZE);

    int* d_arr;
    checkCudaError(cudaMalloc((void**)&d_arr, SIZE * sizeof(int)), "cudaMalloc");

    checkCudaError(cudaMemcpy(d_arr, h_arr, SIZE * sizeof(int), cudaMemcpyHostToDevice), "cudaMemcpy H2D");

    start_time();

    quickSortKernel<<<1, 1>>>(d_arr, 0, SIZE - 1);

    checkCudaError(cudaGetLastError(), "Kernel launch quickSortKernel");

    checkCudaError(cudaDeviceSynchronize(), "cudaDeviceSynchronize");

    stop_time();

    checkCudaError(cudaMemcpy(h_arr, d_arr, SIZE * sizeof(int), cudaMemcpyDeviceToHost), "cudaMemcpy D2H");

    print_time((char*)"CUDA Elapsed:");

    sortedTest(h_arr, SIZE);

    cudaFree(d_arr);
    free(h_arr);

    return 0;
}
