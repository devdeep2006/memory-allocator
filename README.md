# 🧠 Custom Memory Allocator

A high-performance memory allocator in C that combines the **buddy system** and **segregated free lists** to minimize fragmentation and optimize performance.

---

## 📖 Overview

This memory allocator implements two complementary strategies:

- **Buddy System**: For **larger allocations (>4KB)**, offering excellent coalescing.
- **Segregated Free Lists**: For **smaller allocations (≤4KB)**, enabling fast and efficient allocation.

---

## ✨ Features

- 🔀 **Hybrid Strategy**: Automatically picks the best method based on size.
- 🧩 **Low Fragmentation**: Buddy system merges adjacent blocks.
- ⚡ **Fast Small Allocations**: O(1) allocation with segregated lists.
- 📊 **Memory Stats**: Tracks usage patterns and fragmentation.
- 🔁 **Automatic Coalescing**: Both systems support merging.
- 📐 **Memory Alignment**: All allocations are 8-byte aligned.

---

## 🏗 Architecture

### Memory Layout
──┐
│ Total Heap (1MB) │
├──────────────────────────┬──────────────────────────────┤
│ Buddy System Heap │ Segregated Lists Heap │
│ (512KB) │ (512KB) │
└──────────────────────────┴──────────────────────────────┘

### Buddy System
- **Order Range**: 0–19 (16B to 8MB blocks)
- **Usage**: Allocations > 4KB
- **Supports**: Coalescing, splitting

### Segregated Free Lists
- **Size Classes**: 12 (from 16B to 32KB)
- **Usage**: Allocations ≤ 4KB
- **Supports**: Fast lookup, block splitting

---

## 🧩 API

### Core Functions
void* my_malloc(size_t size);
void  my_free(void* ptr);
void* my_realloc(void* ptr, size_t new_size);

# 🧠 Custom Memory Allocator

A high-performance memory allocator implementation in C that combines the **Buddy System** and **Segregated Free Lists** to minimize fragmentation and optimize memory operations.

---

## 📖 Overview

This allocator uses:

- 🧱 **Buddy System** for large allocations (>4KB)
- ⚡ **Segregated Free Lists** for small allocations (≤4KB)

Both work together to reduce memory fragmentation and improve performance.

---

## ✨ Features

- 🔁 Hybrid allocation strategy (buddy + segregated lists)
- 🔄 Automatic coalescing of adjacent free blocks
- 🧠 Memory statistics tracking
- 📏 8-byte memory alignment
- 💡 Fast, low-fragmentation small allocations
- 📉 Efficient large memory management

---

## 🧩 API

### Core Functions
void* my_malloc(size_t size);
void  my_free(void* ptr);
void* my_realloc(void* ptr, size_t new_size);

## 🧪 Test Suite
The test suite includes:

✅ Basic allocation and deallocation

✨ String usage and memory validation

🧱 Fragmentation handling

🔀 Mixed size allocation testing

📊 Real-time memory statistics

## ⚠ Limitations
❌ Fixed 1MB heap size

❌ No thread safety

❌ Basic realloc without resizing optimizations

❌ Linux-only (uses mmap)

❌ No buffer overflow protection (no guard pages)

## 🚀 Future Enhancements
✅ Add thread safety using mutexes

📈 Support dynamic heap growth

🛡 Add guard pages for overflow protection

🧹 Leak/corruption detection tools

📊 Performance profiling

🧠 Hooks for garbage collection systems

🪟 Cross-platform support (e.g., Windows/macOS)
