#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h> // For size_t
#include <stdint.h> // For uintptr_t

// Configuration constants
#define HEAP_SIZE (1024 * 1024)     // 1MB heap
#define MIN_BLOCK_SIZE 16           // Minimum allocation size
#define MAX_ORDER 20                // Maximum buddy system order
#define NUM_SIZE_CLASSES 12         // Number of segregated list size classes

// Block header structure for segregated lists
typedef struct block {
    size_t size;            // Size of the block (including header)
    int free;               // 1 if free, 0 if allocated
    struct block* next;     // Next block in free list
    struct block* prev;     // Previous block in free list
} block_t;

// Buddy system node
typedef struct buddy_node {
    int order;                      // Order of the block (2^order bytes)
    int free;                       // 1 if free, 0 if allocated
    struct buddy_node* next;        // Next in free list
    struct buddy_node* prev;        // Previous in free list
} buddy_node_t;

// Memory allocator structure
typedef struct {
    // Segregated free lists
    block_t* size_classes[NUM_SIZE_CLASSES];
    size_t class_sizes[NUM_SIZE_CLASSES];
    
    // Buddy system
    buddy_node_t* buddy_free_lists[MAX_ORDER];
    void* buddy_heap;
    size_t buddy_heap_size;
    
    // General heap management
    void* heap_start;
    void* heap_end;
    size_t total_allocated;
    size_t total_free;
    
    // Statistics
    size_t allocation_count;
    size_t free_count;
    size_t fragmentation_count;
} allocator_t;

// External declaration for the global allocator instance
extern allocator_t g_allocator;

// Function prototypes for internal use (declared in specific .c files, but useful to know they exist)
// buddy_system.c
void* buddy_alloc_internal(size_t size);
void buddy_free_internal(void* ptr);

// segregated_lists.c
void* seg_alloc_internal(size_t size);
void seg_free_internal(void* ptr);

// utils.c
size_t align_size(size_t size);
int get_order(size_t size);
size_t get_size_class_index(size_t size);

#endif // ALLOCATOR_H