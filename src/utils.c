#include "allocator.h" // For MIN_BLOCK_SIZE, MAX_ORDER, NUM_SIZE_CLASSES
#include <stddef.h> // For size_t

// Utility functions

// Aligns size to 8 bytes. This is typically important for data alignment
// to avoid performance penalties and ensure proper access for certain data types.
size_t align_size(size_t size) {
    return (size + 7) & ~7; // Align to 8 bytes
}

// Determines the buddy system order for a given size.
// The block sizes are 2^(order + 4) because MIN_BLOCK_SIZE is 16 (2^4).
// So, order 0 corresponds to 16 bytes, order 1 to 32 bytes, etc.
int get_order(size_t size) {
    int order = 0;
    size_t block_size = MIN_BLOCK_SIZE;
    
    // Find the smallest block size (power of 2) that can accommodate the requested size
    while (block_size < size && order < MAX_ORDER - 1) { // -1 to not exceed MAX_ORDER index
        block_size <<= 1; // Multiply by 2
        order++;
    }
    return order;
}

// Maps a given size to a segregated list size class index.
// The size classes are powers of 2 from 16 up to 32768 (2^15).
size_t get_size_class_index(size_t size) {
    // Map sizes to size classes (powers of 2 from 16 to 16 * 2^(NUM_SIZE_CLASSES-1))
    // This assumes class_sizes[i] = MIN_BLOCK_SIZE << i.
    // So, MIN_BLOCK_SIZE (16) is index 0.
    // 32 is index 1.
    // ...
    // 16 * 2^10 = 16384 is index 10.
    // 16 * 2^11 = 32768 is index 11.

    if (size <= g_allocator.class_sizes[0]) return 0; // 16
    if (size <= g_allocator.class_sizes[1]) return 1; // 32
    if (size <= g_allocator.class_sizes[2]) return 2; // 64
    if (size <= g_allocator.class_sizes[3]) return 3; // 128
    if (size <= g_allocator.class_sizes[4]) return 4; // 256
    if (size <= g_allocator.class_sizes[5]) return 5; // 512
    if (size <= g_allocator.class_sizes[6]) return 6; // 1024
    if (size <= g_allocator.class_sizes[7]) return 7; // 2048
    if (size <= g_allocator.class_sizes[8]) return 8; // 4096
    if (size <= g_allocator.class_sizes[9]) return 9; // 8192
    if (size <= g_allocator.class_sizes[10]) return 10; // 16384
    return NUM_SIZE_CLASSES - 1; // 32768 and above (maps to the largest class)
}