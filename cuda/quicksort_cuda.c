#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

extern "C" {
    #include "my_timers.h"
}

#define SIZE (1 << 21) 
#define THREADS_PER_BLOCK 256

void checkCudaError(cudaError_t err, const char* msg) {
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA Error: %s: %s\n", msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

__global__ void bitonic_sort_step(int *dev_values, int j, int k) {
    unsigned int i, ixj; 

    i = threadIdx.x + blockDim.x * blockIdx.x;

    ixj = i ^ k; 

    if (ixj > i && ixj < SIZE) {

        bool ascending = ((i & j) == 0);
        
        int val1 = dev_values[i];
        int val2 = dev_values[ixj];

        if (ascending) {
            if (val1 > val2) {
                dev_values[i] = val2;
                dev_values[ixj] = val1;
            }
        } else {
            if (val1 < val2) {
                dev_values[i] = val2;
                dev_values[ixj] = val1;
            }
        }
    }
}

void sortedTest(int* arr, int size) {
    int resultFlag = 1;
    for (int i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            resultFlag = 0;
            printf("Fail at index %d: %d > %d\n", i, arr[i], arr[i+1]);
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
    printf("GPU: %s\n", prop.name);

    int* h_arr = (int*)malloc(SIZE * sizeof(int));
    if (!h_arr) {
        fprintf(stderr, "Host malloc failed\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < SIZE; i++) {
        h_arr[i] = rand() % 1000000 + 1;
    }

    int* d_arr;
    checkCudaError(cudaMalloc((void**)&d_arr, SIZE * sizeof(int)), "cudaMalloc");
    checkCudaError(cudaMemcpy(d_arr, h_arr, SIZE * sizeof(int), cudaMemcpyHostToDevice), "cudaMemcpy H2D");

    dim3 blocks(SIZE / THREADS_PER_BLOCK); 
    dim3 threads(THREADS_PER_BLOCK);

    start_time();

    for (int j = 2; j <= SIZE; j <<= 1) {
        for (int k = j >> 1; k > 0; k = k >> 1) {
            bitonic_sort_step<<<blocks, threads>>>(d_arr, j, k);
        }
    }

    checkCudaError(cudaGetLastError(),"");
    checkCudaError(cudaDeviceSynchronize(),"");

    stop_time();

    checkCudaError(cudaMemcpy(h_arr, d_arr, SIZE * sizeof(int), cudaMemcpyDeviceToHost), "cudaMemcpy D2H");

    print_time((char*)"CUDA Elapsed:");

    sortedTest(h_arr, SIZE);

    cudaFree(d_arr);
    free(h_arr);

    return 0;
}
