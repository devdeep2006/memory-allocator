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

