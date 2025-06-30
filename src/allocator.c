#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h> // For assertions in development

// Global allocator instance
allocator_t g_allocator = {0};

// Forward declarations for functions defined in other source files
// These are not exposed in memory_allocator.h, but used internally by allocator.c
extern void* buddy_alloc_internal(size_t size);
extern void buddy_free_internal(void* ptr);
extern void* seg_alloc_internal(size_t size);
extern void seg_free_internal(void* ptr);
extern size_t get_size_class_index(size_t size);
extern int get_order(size_t size);


// Initialize the memory allocator
int allocator_init() {
    // Initialize size classes
    for (int i = 0; i < NUM_SIZE_CLASSES; i++) {
        g_allocator.size_classes[i] = NULL;
        // Calculate class sizes: 16, 32, 64, ..., 16 * 2^(NUM_SIZE_CLASSES-1)
        g_allocator.class_sizes[i] = MIN_BLOCK_SIZE << i;
    }
    
    // Initialize buddy system free lists
    for (int i = 0; i < MAX_ORDER; i++) {
        g_allocator.buddy_free_lists[i] = NULL;
    }
    
    // Allocate main heap using mmap
    g_allocator.heap_start = mmap(NULL, HEAP_SIZE, 
                                  PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (g_allocator.heap_start == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }
    
    g_allocator.heap_end = (char*)g_allocator.heap_start + HEAP_SIZE;
    
    // Initialize buddy system heap (use half of total heap)
    g_allocator.buddy_heap = g_allocator.heap_start;
    g_allocator.buddy_heap_size = HEAP_SIZE / 2;
    
    // Create initial buddy block
    // The initial block for the buddy system covers its entire allocated sub-heap.
    // Its order is determined by the largest power of 2 that fits its size,
    // ensuring it's a valid starting point for splits.
    buddy_node_t* initial_buddy_block = (buddy_node_t*)g_allocator.buddy_heap;
    initial_buddy_block->order = get_order(g_allocator.buddy_heap_size);
    // Ensure the initial buddy block size is a power of 2 and aligns with HEAP_SIZE / 2
    size_t actual_buddy_block_size = (1UL << (initial_buddy_block->order + 4));
    assert(actual_buddy_block_size >= g_allocator.buddy_heap_size / 2); // Should cover at least half
    
    initial_buddy_block->free = 1;
    initial_buddy_block->next = NULL;
    initial_buddy_block->prev = NULL;
    
    g_allocator.buddy_free_lists[initial_buddy_block->order] = initial_buddy_block;
    
    // Initialize segregated list heap (use other half)
    void* seg_heap_start = (char*)g_allocator.heap_start + HEAP_SIZE / 2;
    size_t seg_heap_size = HEAP_SIZE / 2;
    
    // Create initial free block for segregated lists
    block_t* initial_seg_block = (block_t*)seg_heap_start;
    initial_seg_block->size = seg_heap_size; // The initial seg block takes up the entire second half
    initial_seg_block->free = 1;
    initial_seg_block->next = NULL;
    initial_seg_block->prev = NULL;
    
    // Add to appropriate size class (this initial block will likely be in the largest size class)
    size_t class_idx = get_size_class_index(seg_heap_size);
    if (class_idx >= NUM_SIZE_CLASSES) class_idx = NUM_SIZE_CLASSES - 1; // Cap at max class
    g_allocator.size_classes[class_idx] = initial_seg_block;
    
    g_allocator.total_free = HEAP_SIZE;
    g_allocator.total_allocated = 0; // Initially nothing is allocated by the user
    g_allocator.allocation_count = 0;
    g_allocator.free_count = 0;
    g_allocator.fragmentation_count = 0;

    printf("Memory allocator initialized:\n");
    printf("  Total heap size: %zu bytes\n", (size_t)HEAP_SIZE);
    printf("  Buddy system heap: %zu bytes\n", g_allocator.buddy_heap_size);
    printf("  Segregated lists heap: %zu bytes\n", seg_heap_size);
    
    return 0;
}

// Public allocation interface
void* my_malloc(size_t size) {
    if (size == 0) return NULL;
    
    // Use buddy system for larger allocations, segregated lists for smaller ones
    // The threshold (4096 bytes) can be tuned.
    if (size > 4096 - sizeof(buddy_node_t)) { // Account for header size
        return buddy_alloc_internal(size);
    } else {
        void* ptr = seg_alloc_internal(size);
        if (!ptr) {
            // Fallback to buddy system if segregated list fails (e.g., no suitable block)
            // This might lead to more fragmentation for smaller blocks if the seg list is exhausted.
            // A more sophisticated allocator might try to get a larger block from buddy and
            // then split it for the segregated list.
            ptr = buddy_alloc_internal(size); // Note: buddy_alloc might return a larger block than requested
        }
        return ptr;
    }
}

void my_free(void* ptr) {
    if (!ptr) return;
    
    // Determine which allocator was used based on address
    // The buddy heap starts at g_allocator.heap_start and occupies buddy_heap_size
    // The segregated list heap starts after the buddy heap.
    if (ptr >= g_allocator.buddy_heap && 
        ptr < (char*)g_allocator.buddy_heap + g_allocator.buddy_heap_size) {
        buddy_free_internal(ptr);
    } else if (ptr > (char*)g_allocator.buddy_heap + g_allocator.buddy_heap_size &&
               ptr < g_allocator.heap_end) {
        seg_free_internal(ptr);
    } else {
        // This indicates an attempt to free memory not allocated by this allocator
        fprintf(stderr, "Attempt to free unmanaged memory address: %p\n", ptr);
        assert(0 && "Attempt to free unmanaged memory");
    }
}

// Realloc implementation
void* my_realloc(void* ptr, size_t new_size) {
    if (!ptr) return my_malloc(new_size);
    if (new_size == 0) {
        my_free(ptr);
        return NULL;
    }
    
    // In a real realloc, you'd need to know the *original* size of the block
    // pointed to by `ptr`. Since our block headers are *before* the user data,
    // we can attempt to retrieve the size.
    size_t old_size;
    
    // Determine which allocator was used to get old_size
    if (ptr >= g_allocator.buddy_heap && 
        ptr < (char*)g_allocator.buddy_heap + g_allocator.buddy_heap_size) {
        buddy_node_t* block = (buddy_node_t*)((char*)ptr - sizeof(buddy_node_t));
        old_size = (1UL << (block->order + 4)) - sizeof(buddy_node_t); // Payload size
    } else {
        block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
        old_size = block->size - sizeof(block_t); // Payload size
    }

    void* new_ptr = my_malloc(new_size);
    if (!new_ptr) return NULL;
    
    // Copy old data, copying the minimum of the old and new payload size
    memcpy(new_ptr, ptr, (old_size < new_size) ? old_size : new_size);
    my_free(ptr);
    
    return new_ptr;
}

// Statistics and debugging
void print_allocator_stats() {
    printf("\n=== Memory Allocator Statistics ===\n");
    printf("Total allocations: %zu\n", g_allocator.allocation_count);
    printf("Total frees: %zu\n", g_allocator.free_count);
    printf("Currently allocated: %zu bytes\n", g_allocator.total_allocated);
    printf("Currently free: %zu bytes\n", g_allocator.total_free);
    printf("Fragmentation events: %zu\n", g_allocator.fragmentation_count);
    
    printf("\nBuddy System Free Lists:\n");
    for (int i = 0; i < MAX_ORDER; i++) {
        int count = 0;
        buddy_node_t* block = g_allocator.buddy_free_lists[i];
        while (block) {
            count++;
            block = block->next;
        }
        if (count > 0) {
            printf("  Order %d (block size %zu bytes, payload %zu bytes): %d blocks\n", 
                     i, (1UL << (i + 4)), (1UL << (i + 4)) - sizeof(buddy_node_t), count);
        }
    }
    
    printf("\nSegregated Free Lists:\n");
    for (int i = 0; i < NUM_SIZE_CLASSES; i++) {
        int count = 0;
        block_t* block = g_allocator.size_classes[i];
        while (block) {
            count++;
            block = block->next;
        }
        if (count > 0) {
            printf("  Size class index %d (target size %zu bytes): %d blocks\n", 
                     i, g_allocator.class_sizes[i], count);
        }
    }
}

// Cleanup function
void allocator_cleanup() {
    if (g_allocator.heap_start) {
        if (munmap(g_allocator.heap_start, HEAP_SIZE) == -1) {
            perror("munmap failed during cleanup");
        }
        memset(&g_allocator, 0, sizeof(g_allocator)); // Reset allocator state
    }
}