#include "memory_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This example will demonstrate more complex allocation patterns,
// potentially involving many small allocations, large allocations,
// and interleaved frees to observe allocator behavior.

void run_advanced_example() {
    printf("--- Advanced Usage Example ---\n\n");

    // Scenario 1: Many small allocations to stress segregated lists
    printf("Scenario 1: Many small allocations (50 bytes each)\n");
    const int num_small_allocs = 5000;
    void* small_ptrs[num_small_allocs];

    for (int i = 0; i < num_small_allocs; ++i) {
        small_ptrs[i] = my_malloc(50);
        if (!small_ptrs[i]) {
            printf("Failed to allocate small_ptr[%d]\n", i);
            break;
        }
        // Optionally fill with data
        memset(small_ptrs[i], (i % 26) + 'A', 49);
        ((char*)small_ptrs[i])[49] = '\0';
    }
    printf("Completed %d small allocations.\n", num_small_allocs);
    print_allocator_stats();

    // Free half of them to create fragmentation in segregated lists
    printf("\nFreeing half of the small blocks (every other)...\n");
    for (int i = 0; i < num_small_allocs; i += 2) {
        if (small_ptrs[i]) {
            my_free(small_ptrs[i]);
            small_ptrs[i] = NULL;
        }
    }
    print_allocator_stats();

    // Allocate some large blocks after small allocations and fragmentation
    printf("\nScenario 2: Allocating a few large blocks (10KB each)\n");
    void* large_blocks[3];
    for (int i = 0; i < 3; ++i) {
        large_blocks[i] = my_malloc(10240); // 10KB, likely buddy system
        if (large_blocks[i]) {
            printf("Allocated large block %d at %p\n", i, large_blocks[i]);
        } else {
            printf("Failed to allocate large block %d\n", i);
        }
    }
    print_allocator_stats();

    // Free remaining small blocks
    printf("\nFreeing remaining small blocks...\n");
    for (int i = 0; i < num_small_allocs; ++i) {
        if (small_ptrs[i]) {
            my_free(small_ptrs[i]);
        }
    }
    print_allocator_stats();

    // Free large blocks
    printf("\nFreeing large blocks...\n");
    for (int i = 0; i < 3; ++i) {
        if (large_blocks[i]) {
            my_free(large_blocks[i]);
        }
    }
    print_allocator_stats();

    printf("\n--- Advanced Usage Example Finished ---\n");
}

int main() {
    if (allocator_init() != 0) {
        fprintf(stderr, "Failed to initialize allocator\n");
        return 1;
    }

    run_advanced_example();

    allocator_cleanup();
    printf("Allocator cleanup completed.\n");

    return 0;
}