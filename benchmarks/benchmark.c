#include "memory_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BENCH_ALLOCATIONS 1000000
#define BENCH_MAX_SIZE 1024

void run_custom_allocator_benchmark() {
    printf("--- Benchmarking Custom Allocator ---\n");
    printf("Performing %d allocations and frees of random sizes (up to %d bytes).\n",
           BENCH_ALLOCATIONS, BENCH_MAX_SIZE);

    if (allocator_init() != 0) {
        fprintf(stderr, "Failed to initialize custom allocator for benchmark.\n");
        return;
    }

    void** ptrs = (void**)malloc(sizeof(void*) * (BENCH_ALLOCATIONS / 2)); // Store half to keep some alive
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate ptrs array for benchmark.\n");
        allocator_cleanup();
        return;
    }
    int current_ptrs = 0;

    clock_t start = clock();

    for (int i = 0; i < BENCH_ALLOCATIONS; ++i) {
        size_t size = (rand() % BENCH_MAX_SIZE) + 1; // Random size between 1 and MAX_SIZE
        void* p = my_malloc(size);
        if (!p) {
            fprintf(stderr, "Custom malloc failed at iteration %d, size %zu\n", i, size);
            break;
        }

        // Simulate a mix of short-lived and long-lived allocations
        if (rand() % 2 == 0 && current_ptrs < (BENCH_ALLOCATIONS / 2)) {
            ptrs[current_ptrs++] = p;
        } else {
            my_free(p);
        }
    }

    // Free any remaining long-lived allocations
    for (int i = 0; i < current_ptrs; ++i) {
        if (ptrs[i]) {
            my_free(ptrs[i]);
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\nCustom Allocator Benchmark Results:\n");
    printf("Total operations: %d allocations + %d frees\n",
           (int)g_allocator.allocation_count, (int)g_allocator.free_count);
    printf("Time taken: %f seconds\n", time_spent);
    print_allocator_stats();

    free(ptrs);
    allocator_cleanup();
}

int main() {
    run_custom_allocator_benchmark();
    return 0;
}