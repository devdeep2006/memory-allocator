#include "allocator.h" // Includes allocator_t and buddy_node_t definitions
#include <stdio.h>
#include <assert.h> // For debugging assertions

// Forward declarations for utility functions
extern int get_order(size_t size);
extern size_t align_size(size_t size); // Although buddy system works with powers of 2,
                                        // internal alignment might still be relevant for headers.

// Buddy system allocation (internal)
void* buddy_alloc_internal(size_t size) {
    // Calculate the required order, including space for the buddy_node_t header.
    // The actual allocated block size will be 2^(order + 4) (since MIN_BLOCK_SIZE is 16 = 2^4)
    size_t required_block_size_with_header = align_size(size + sizeof(buddy_node_t));
    int order = get_order(required_block_size_with_header);
    
    if (order >= MAX_ORDER) {
        fprintf(stderr, "Requested size %zu is too large for buddy system (max order %d)\n", size, MAX_ORDER -1);
        return NULL;
    }
    
    // Find a free block of appropriate order
    int current_order = order;
    while (current_order < MAX_ORDER && 
           g_allocator.buddy_free_lists[current_order] == NULL) {
        current_order++;
    }
    
    if (current_order >= MAX_ORDER) {
        // No suitable block found, potentially out of memory or highly fragmented
        g_allocator.fragmentation_count++;
        return NULL; 
    }
    
    // Remove block from free list
    buddy_node_t* block = g_allocator.buddy_free_lists[current_order];
    g_allocator.buddy_free_lists[current_order] = block->next;
    if (block->next) {
        block->next->prev = NULL;
    }
    
    // Split block if necessary until it reaches the requested order
    while (current_order > order) {
        current_order--;
        
        // Calculate buddy address
        // The block size for the current order is 2^(current_order + 4)
        size_t block_size_at_current_order = 1UL << (current_order + 4); 
        buddy_node_t* buddy = (buddy_node_t*)((char*)block + block_size_at_current_order);
        
        // Initialize buddy
        buddy->order = current_order;
        buddy->free = 1;
        buddy->next = g_allocator.buddy_free_lists[current_order];
        buddy->prev = NULL;
        
        if (g_allocator.buddy_free_lists[current_order]) {
            g_allocator.buddy_free_lists[current_order]->prev = buddy;
        }
        g_allocator.buddy_free_lists[current_order] = buddy;
    }
    
    block->free = 0;
    block->order = order; // Assign the correct order to the allocated block
    
    size_t allocated_size_with_header = (1UL << (order + 4));
    g_allocator.allocation_count++;
    g_allocator.total_allocated += allocated_size_with_header;
    g_allocator.total_free -= allocated_size_with_header;
    
    return (char*)block + sizeof(buddy_node_t); // Return pointer to user data
}

// Buddy system deallocation (internal)
void buddy_free_internal(void* ptr) {
    if (!ptr) return;
    
    // Get the block header from the user pointer
    buddy_node_t* block = (buddy_node_t*)((char*)ptr - sizeof(buddy_node_t));
    
    // Basic validation
    if (block->free) {
        fprintf(stderr, "Double free detected or freeing an already free buddy block: %p\n", ptr);
        assert(0 && "Double free or freeing already free buddy block");
        return;
    }
    
    block->free = 1;
    
    size_t block_size = 1UL << (block->order + 4); // Actual size of this block
    g_allocator.free_count++;
    g_allocator.total_allocated -= block_size;
    g_allocator.total_free += block_size;
    
    // Try to merge with buddy
    while (block->order < MAX_ORDER - 1) {
        // Calculate buddy address
        uintptr_t block_addr = (uintptr_t)block;
        
        // The buddy's address is found by XORing the block's address with its block size.
        // This works because buddies are always aligned to their block size.
        uintptr_t buddy_addr = block_addr ^ block_size;
        
        // Check if buddy is within heap bounds
        if (buddy_addr < (uintptr_t)g_allocator.buddy_heap ||
            buddy_addr >= (uintptr_t)g_allocator.buddy_heap + g_allocator.buddy_heap_size) {
            break; // Buddy is out of bounds, cannot merge
        }
        
        buddy_node_t* buddy = (buddy_node_t*)buddy_addr;
        
        // Check if buddy is free and of the same order
        if (!buddy->free || buddy->order != block->order) {
            break; // Cannot merge
        }
        
        // Merge condition met: remove buddy from its free list
        if (buddy->prev) {
            buddy->prev->next = buddy->next;
        } else {
            g_allocator.buddy_free_lists[buddy->order] = buddy->next;
        }
        if (buddy->next) {
            buddy->next->prev = buddy->prev;
        }
        
        // Update the block pointer to the lower address of the merged pair
        if (buddy_addr < block_addr) {
            block = buddy;
        }
        
        block->order++; // Increment order of the merged block
        block_size <<= 1; // Double the block size for the new order
    }
    
    // Add merged block to its new, potentially higher, order free list
    block->next = g_allocator.buddy_free_lists[block->order];
    block->prev = NULL;
    if (g_allocator.buddy_free_lists[block->order]) {
        g_allocator.buddy_free_lists[block->order]->prev = block;
    }
    g_allocator.buddy_free_lists[block->order] = block;
}