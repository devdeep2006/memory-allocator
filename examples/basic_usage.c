#include "memory_allocator.h"
#include <stdio.h>
#include <string.h>

int main() {
    if (allocator_init() != 0) {
        fprintf(stderr, "Failed to initialize allocator\n");
        return 1;
    }
    
    printf("--- Basic Usage Example ---\n\n");

    // Allocate a small block
    char* str1 = (char*)my_malloc(30);
    if (str1) {
        strcpy(str1, "Hello from custom malloc!");
        printf("Allocated str1: '%s' at %p\n", str1, (void*)str1);
    } else {
        printf("Failed to allocate str1.\n");
    }

    // Allocate a medium block
    int* arr = (int*)my_malloc(10 * sizeof(int));
    if (arr) {
        for (int i = 0; i < 10; ++i) {
            arr[i] = i * 100;
        }
        printf("Allocated arr: ");
        for (int i = 0; i < 10; ++i) {
            printf("%d ", arr[i]);
        }
        printf("at %p\n", (void*)arr);
    } else {
        printf("Failed to allocate arr.\n");
    }

    // Allocate a large block (should use buddy system)
    char* large_buffer = (char*)my_malloc(5000);
    if (large_buffer) {
        strcpy(large_buffer, "This is a larger buffer allocated via the buddy system.");
        printf("Allocated large_buffer: '%s' at %p\n", large_buffer, (void*)large_buffer);
    } else {
        printf("Failed to allocate large_buffer.\n");
    }

    printf("\nCurrent Allocator Stats before freeing:\n");
    print_allocator_stats();

    // Free the allocated blocks
    my_free(str1);
    printf("\nFreed str1.\n");

    my_free(arr);
    printf("Freed arr.\n");

    my_free(large_buffer);
    printf("Freed large_buffer.\n");

    printf("\nCurrent Allocator Stats after freeing:\n");
    print_allocator_stats();

    // Test realloc
    printf("\nTesting my_realloc...\n");
    char* realloc_test = (char*)my_malloc(10);
    if (realloc_test) {
        strcpy(realloc_test, "Short");
        printf("Original realloc_test: '%s' at %p\n", realloc_test, (void*)realloc_test);

        realloc_test = (char*)my_realloc(realloc_test, 20);
        if (realloc_test) {
            strcat(realloc_test, " and longer!");
            printf("Reallocated (grow) realloc_test: '%s' at %p\n", realloc_test, (void*)realloc_test);
        } else {
            printf("Failed to reallocate (grow) realloc_test.\n");
        }

        realloc_test = (char*)my_realloc(realloc_test, 5);
        if (realloc_test) {
            printf("Reallocated (shrink) realloc_test: '%.5s' at %p\n", realloc_test, (void*)realloc_test); // Print only 5 chars
        } else {
            printf("Failed to reallocate (shrink) realloc_test.\n");
        }

        my_free(realloc_test);
        printf("Freed realloc_test.\n");
    } else {
        printf("Failed to allocate initial realloc_test.\n");
    }

    printf("\nFinal Allocator Stats:\n");
    print_allocator_stats();

    allocator_cleanup();
    printf("\nAllocator cleanup completed.\n");
    
    return 0;
}