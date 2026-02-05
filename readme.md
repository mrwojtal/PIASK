# PIASK Project

The **PIASK** project is an implementation of the **Quicksort** algorithm using various parallel processing technologies: **OpenMP (OMP)**, **MPI**, and **CUDA**.

## Project Description

The goal of the project is to implement the **Quicksort** algorithm in different parallel environments in order to compare performance and efficiency across various parallelization methods.

Quicksort is a well-known **divide and conquer** sorting algorithm which, in its parallel form, allows efficient utilization of modern CPUs and GPUs.

---

## OMP

The **OpenMP (OMP)** implementation uses CPU multithreading.
- Parallel sorting of array partitions
- Dynamic task assignment to threads
- Scalability dependent on the number of CPU cores

---

## MPI

The **MPI (Message Passing Interface)** implementation enables sorting in a distributed environment.
- Data distribution across cluster nodes
- Exchange of results between processes
- Communication optimization to minimize latency

---

## CUDA

The **CUDA** implementation enables acceleration of the algorithm on NVIDIA GPUs.
- Utilization of thousands of GPU threads for parallel sorting
- Minimization of data transfers between CPU and GPU
- Optimization of shared and global memory usage

---

## How to Run
1. OMP:
    ```bash
    cd labwork/project/omp
    module add intel
    icc -qopenmp my_timers.c quicksort_omp.c -o quicksort_omp
    ./quicksort_omp
    ```
2. MPI:
    ```bash
    cd labwork/project/mpi
    module add openmpi
    mpicc quicksort_mpi.c timers/my_timers.c -o quicksort_mpi
    mpirun -np 8 ./quicksort_mpi
    ```
3. CUDA:
      ```bash
    cd labwork/project/cuda
    module add cuda/12.2.0
    nvcc -rdc=true -arch=sm_70 -O3 quicksort_cuda.cu my_timers.c -o quicksort_cuda
      ```
---






