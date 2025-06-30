#include "memory_allocator.h" // For our custom allocator
#include <stdio.h>
#include <stdlib.h> // For standard malloc/free
#include <time.h>

#define COMPARE_ALLOCATIONS 1000000
#define COMPARE_MAX_SIZE 1024

void run_standard_malloc_benchmark() {
    printf("--- Benchmarking Standard malloc/free ---\n");
    printf("Performing %d allocations and frees of random sizes (up to %d bytes).\n",
           COMPARE_ALLOCATIONS, COMPARE_MAX_SIZE);

    srand(time(NULL)); // Ensure same seed for fair comparison

    void** ptrs = (void**)malloc(sizeof(void*) * (COMPARE_ALLOCATIONS / 2));
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate ptrs array for benchmark.\n");
        return;
    }
    int current_ptrs = 0;

    clock_t start = clock();

    for (int i = 0; i < COMPARE_ALLOCATIONS; ++i) {
        size_t size = (rand() % COMPARE_MAX_SIZE) + 1;
        void* p = malloc(size);
        if (!p) {
            fprintf(stderr, "Standard malloc failed at iteration %d, size %zu\n", i, size);
            break;
        }

        if (rand() % 2 == 0 && current_ptrs < (COMPARE_ALLOCATIONS / 2)) {
            ptrs[current_ptrs++] = p;
        } else {
            free(p);
        }
    }

    for (int i = 0; i < current_ptrs; ++i) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\nStandard malloc/free Benchmark Results:\n");
    printf("Time taken: %f seconds\n", time_spent);

    free(ptrs);
}

int main() {
    // Run custom allocator benchmark first
    run_custom_allocator_benchmark();
    printf("\n----------------------------------------\n\n");
    
    // Reset random seed for a fair comparison with standard malloc
    srand(time(NULL)); 
    run_standard_malloc_benchmark();
    
    return 0;
}