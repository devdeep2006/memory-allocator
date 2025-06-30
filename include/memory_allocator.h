#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <stddef.h> // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the memory allocator. Must be called once before any other allocator function.
// Returns 0 on success, -1 on failure.
int allocator_init(void);

// Allocates 'size' bytes of memory and returns a pointer to the allocated block.
// Returns NULL if allocation fails.
void* my_malloc(size_t size);

// Frees the memory block pointed to by 'ptr'. If 'ptr' is NULL, no operation is performed.
void my_free(void* ptr);

// Changes the size of the memory block pointed to by 'ptr' to 'new_size' bytes.
// The contents of the old block are preserved up to the minimum of the old and new sizes.
// If 'ptr' is NULL, behaves like my_malloc(new_size).
// If 'new_size' is 0, behaves like my_free(ptr).
// Returns a pointer to the reallocated memory block, or NULL if reallocation fails.
void* my_realloc(void* ptr, size_t new_size);

// Prints current statistics about the memory allocator's state.
void print_allocator_stats(void);

// Cleans up and deinitializes the memory allocator.
// Should be called at the end of the program to release mapped memory.
void allocator_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_ALLOCATOR_H