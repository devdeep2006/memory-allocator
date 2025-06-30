#include "allocator.h" // Includes allocator_t and block_t definitions
#include <stdio.h>
#include <assert.h> // For debugging assertions

// Forward declarations for utility functions
extern size_t align_size(size_t size);
extern size_t get_size_class_index(size_t size);

// Segregated list allocation (internal)
void* seg_alloc_internal(size_t size) {
    // Align requested size and add space for the block_t header
    size = align_size(size + sizeof(block_t));
    
    // Determine the initial size class to search
    size_t class_idx = get_size_class_index(size);
    
    // Look for a suitable block starting from the appropriate size class
    // and moving to larger classes if necessary (first-fit within classes, then best-fit across classes implicitly)
    for (size_t i = class_idx; i < NUM_SIZE_CLASSES; i++) {
        block_t* block = g_allocator.size_classes[i];
        
        while (block) {
            if (block->free && block->size >= size) {
                // Found a suitable free block
                
                // Remove from its current free list
                if (block->prev) {
                    block->prev->next = block->next;
                } else {
                    g_allocator.size_classes[i] = block->next;
                }
                if (block->next) {
                    block->next->prev = block->prev;
                }
                
                // Split block if it's significantly larger
                // We split if the remainder is large enough to form a new usable free block (at least MIN_BLOCK_SIZE + header)
                if (block->size >= size + sizeof(block_t) + MIN_BLOCK_SIZE) {
                    block_t* new_block = (block_t*)((char*)block + size);
                    new_block->size = block->size - size;
                    new_block->free = 1;
                    new_block->next = NULL;
                    new_block->prev = NULL;
                    
                    // Add remainder to appropriate size class
                    size_t new_class_idx = get_size_class_index(new_block->size);
                    if (new_class_idx >= NUM_SIZE_CLASSES) {
                        new_class_idx = NUM_SIZE_CLASSES - 1; // Cap to the largest class
                    }
                    
                    // Add to the head of the new block's free list
                    new_block->next = g_allocator.size_classes[new_class_idx];
                    new_block->prev = NULL;
                    if (g_allocator.size_classes[new_class_idx]) {
                        g_allocator.size_classes[new_class_idx]->prev = new_block;
                    }
                    g_allocator.size_classes[new_class_idx] = new_block;
                    
                    block->size = size; // The current block now has the requested size
                } else {
                    // If not splitting, the entire block is used, leading to internal fragmentation
                    // g_allocator.fragmentation_count++; // Could increment fragmentation here
                }
                
                block->free = 0; // Mark as allocated
                block->next = NULL; // Clear list pointers
                block->prev = NULL;
                
                g_allocator.allocation_count++;
                g_allocator.total_allocated += block->size;
                g_allocator.total_free -= block->size;
                
                return (char*)block + sizeof(block_t); // Return pointer to user data
            }
            block = block->next;
        }
    }
    
    // No suitable block found in segregated lists
    return NULL;
}

// Segregated list deallocation (internal)
void seg_free_internal(void* ptr) {
    if (!ptr) return;
    
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    
    // Basic validation
    if (block->free) {
        fprintf(stderr, "Double free detected or freeing an already free segregated block: %p\n", ptr);
        assert(0 && "Double free or freeing already free segregated block");
        return;
    }

    block->free = 1;
    
    g_allocator.free_count++;
    g_allocator.total_allocated -= block->size;
    g_allocator.total_free += block->size;
    
    // Simple coalescing with adjacent blocks
    // This is a simplified version. A full implementation would involve:
    // 1. Checking if the previous physical block is free and merge.
    // 2. Checking if the next physical block is free and merge.
    // This requires iterating through memory addresses to find neighbors,
    // which can be slow, or maintaining a sorted list of all free blocks.
    // For now, we'll only add to the free list.
    
    // Add to appropriate size class (head insertion)
    size_t class_idx = get_size_class_index(block->size);
    if (class_idx >= NUM_SIZE_CLASSES) {
        class_idx = NUM_SIZE_CLASSES - 1; // Cap to the largest class
    }
    
    block->next = g_allocator.size_classes[class_idx];
    block->prev = NULL;
    if (g_allocator.size_classes[class_idx]) {
        g_allocator.size_classes[class_idx]->prev = block;
    }
    g_allocator.size_classes[class_idx] = block;
}