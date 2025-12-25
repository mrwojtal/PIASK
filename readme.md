# Projekt PIASK

Projekt **PIASK** jest implementacją algorytmu **Quicksort** z wykorzystaniem różnych technologii równoległego przetwarzania: **OpenMP (OMP)**, **MPI** oraz **CUDA**.  

## Opis projektu
Celem projektu jest implementacja algorytmu **Quicksort** w różnych środowiskach równoległych w celu porównania wydajności i efektywności przy różnych metodach paralelizacji.  

Algorytm Quicksort jest znanym algorytmem sortowania typu **divide and conquer**, który w wersji równoległej pozwala na efektywne wykorzystanie nowoczesnych procesorów i kart graficznych.  

---

## OMP
Implementacja z użyciem **OpenMP (OMP)** wykorzystuje wielowątkowość procesora CPU.  
- Równoległe sortowanie partycji tablicy  
- Dynamiczne przydzielanie zadań do wątków  
- Skalowalność zależna od liczby rdzeni procesora  

---

## MPI
Implementacja z użyciem **MPI (Message Passing Interface)** pozwala na sortowanie w środowisku rozproszonym.  
- Podział danych między węzły klastra  
- Wymiana wyników między procesami  
- Optymalizacja komunikacji w celu minimalizacji opóźnień  

---

## CUDA
Implementacja z użyciem **CUDA** umożliwia przyspieszenie algorytmu na kartach graficznych NVIDIA.  
- Wykorzystanie tysięcy wątków GPU do równoległego sortowania  
- Minimalizacja transferu danych między CPU a GPU  
- Optymalizacja pamięci współdzielonej i globalnej  

---

## Instrukcja uruchomienia
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
      todo
      ```
---
