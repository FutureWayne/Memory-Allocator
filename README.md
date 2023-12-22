# Memory Allocator

This project implements a custom memory allocator system in C++, designed to offer efficient memory management for various use cases. The system comprises two primary components: the `FixedSizeAllocator` and the `HeapManager`, each tailored to handle specific memory allocation requirements.

## FixedSizeAllocator

The `FixedSizeAllocator` is a specialized memory allocator designed for allocating memory blocks of a fixed size. This allocator is particularly efficient for numerous allocations and deallocations of objects of the same size.

### How It Works

- **BitArray Utilization:** The FixedSizeAllocator uses a `BitArray` to track the allocation status of each block in its memory pool efficiently. The BitArray is a compact data structure that uses individual bits to represent the availability of each fixed-size block.
- **Guardbands:** To enhance memory safety, the FixedSizeAllocator employs guardbands. These are small memory regions placed before and after each allocated block to detect and prevent buffer overflows and underflows. 
- **Macro-Enabled Guardbands:** The use of guardbands can be controlled through preprocessor macros. This allows for flexibility in debugging and release builds, where guardbands can be enabled for additional safety checks during development and disabled in production builds for performance optimization.
- **Allocation and Deallocation:** Allocation involves scanning the BitArray for a free block, marking it as occupied, and returning its address. Deallocation simply marks the block as free in the BitArray.

## HeapManager

The `HeapManager` serves as a dynamic memory allocator, capable of handling memory requests of varying sizes. It offers flexibility and efficiency in managing dynamic memory allocations.

### How It Works

- **Linked List Structure:** The HeapManager uses a linked list to manage its memory blocks, with each block containing size and allocation status information, along with pointers to adjacent blocks.
- **Alignment Gaps Utilization:** A key feature of the HeapManager is its ability to utilize alignment gaps for memory allocation. This approach maximizes memory space usage by aligning allocated blocks to specific memory addresses, reducing wasted space due to alignment requirements.
- **Dynamic Allocation with Alignment:** When allocating memory, the HeapManager considers alignment requirements and finds or splits blocks accordingly, ensuring efficient use of memory space and reducing fragmentation.
- **Deallocation and Coalescing:** Deallocation involves marking blocks as free and coalescing adjacent free blocks into larger ones, further optimizing memory usage.
