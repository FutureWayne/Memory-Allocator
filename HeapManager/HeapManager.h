#ifndef HEAP_ALLOCATOR_H
#define HEAP_ALLOCATOR_H

#include <cstddef>
#include <utility>
#include <cassert>

/**
 * @struct MemoryBlock
 * @brief Represents a block of memory.
 *
 * The MemoryBlock struct provides information about a block of memory,
 * including its base address, size, alignment adjustment, and a pointer to the next block.
 */
struct MemoryBlock
{
    /**
     * @brief Actual address of the memory that this block manages.
     *
     * @note pBaseAddress always equals the address of the MemoryBlock itself plus the size of the MemoryBlock struct(defined as MEMORY_BLOCK_OVERHEAD).
     */
    void* pBaseAddress;

    /**
     * @brief The size of the block.
     *
     * This variable represents the size of the block. It is of type size_t, which is an unsigned integral type.
     * The value of BlockSize determines the size of memory that this block manages.
     */
    size_t BlockSize;


    /**
     * @brief The AlignmentAdjustment variable is of type size_t and stores the adjustment needed to align memory addresses.
     *
     * The AlignmentAdjustment value represents the number of bytes that should be added to the address to align it correctly.
     *
     * @note This variable represents the size of alignment gap before the memory block.
     */
    size_t AlignmentAdjustment;
    
    /**
     * @brief Pointer to the next memory block.
     *
     * This variable is used to store a pointer to the next memory block in a linked list structure.
     * Used in both the free memory block list and the outstanding allocation list.
     */
    MemoryBlock* pNextBlock;
};

class HeapManager
{
public:
    HeapManager();
    ~HeapManager();

    size_t m_heapSize;
    void* m_pHeapBaseAddress;
    MemoryBlock* m_pFreeMemoryBlockList;        // Linked list of free blocks
    MemoryBlock* m_pOutstandingAllocationList;  // Linked list of allocated blocks
    
    /**
    * Allocates a block of memory with the specified size and alignment.
    *
    * @param size The size of the memory block to allocate (in bytes).
    * @param alignment The alignment requirement for the memory block.
    *
    * @return A pointer to the allocated memory block, or nullptr if the allocation failed.
    *
    * @note The alignment must be a power of 2.
    * @note If the alignment parameter is zero, it will be treated as no alignment requirement.
    *
    * @warning The returned pointer should be properly casted to the desired type before using it.
    * @warning The caller is responsible for freeing the allocated memory block using the appropriate method.
    *
    * @see HeapManager::Free
    * @see HeapManager::Collect
    */
    void* Alloc(size_t size, size_t alignment);
    
    /**
    * @brief Frees the memory pointed to by the given pointer.
    *
    * This method searches for the memory block associated with the given pointer in the outstanding allocation list.
    * If found, the block is removed from the outstanding allocation list and inserted back into the free memory block list
    * in the correct position.
    *
    * @param ptr A pointer to the memory block to be freed.
    * @return true if the memory block was successfully freed, false otherwise.
    */
    bool Free(const void* ptr);

    /**
     * @brief Collects and merges adjacent free memory blocks in the heap.
     *
     * This method iterates through the list of free memory blocks and checks if any adjacent blocks can be merged.
     * If an adjacent block is found, the blocks are merged and the next block is removed from the free block list.
     * This process continues until no more merges can be performed.
     */
    void Collect() const;
    
    void Init(void* pHeapBaseAddress, size_t heapSize, unsigned int numDescriptors);
    void ShowFreeBlocks() const;
    void ShowOutstandingAllocations() const;
    bool Contains(void* ptr) const;
    bool IsAllocated(const void* ptr) const;
    size_t GetLargestFreeBlockSize() const;
    size_t GetAllOutstandingBlockSize() const;
    size_t GetAllFreeBlockSize() const;
    
    void Destroy() const;
    
private:
    /**
    * @brief Finds a memory block that is suitable for the given size and alignment.
    *
    * @param size The size of the memory block to find.
    * @param alignment The alignment requirement of the memory block to find.
    * @return A pair of MemoryBlock pointers. The first pointer points to the suitable block, and the second pointer points to the previous block.
    *         If a suitable block is not found, both pointers will be nullptr.
    */
    std::pair<MemoryBlock*, MemoryBlock*> findSuitableBlock(size_t size, size_t alignment) const;


    /**
     * Creates a new memory block.
     *
     * @param pBlockAddress The address of the block.
     * @param size The size of the block, excluding MemoryBlock overhead.
     * @return A pointer to the created MemoryBlock.
     */
    static MemoryBlock* createNewBlock(void* pBlockAddress, size_t size);


    /**
     * \brief Shrinks a memory block to a specified size.
     *
     * \param pCurBlock Pointer to the current memory block.
     * \param pPrevBlock Pointer to the previous memory block.
     * \param size The desired size for the memory block.
     *
     * This method is used to shrink a memory block to a specified size. It first checks if the alignment gap is sufficient
     * to satisfy the requested size. If so, it adjusts the alignment gap accordingly. If not, it shrinks the block by creating
     * a new block with the desired size and updating the relevant pointers. If the block becomes empty after shrinking, it is
     * removed from the free memory block list.
     *
     * \pre pCurBlock must not be nullptr.
     * \pre size must be greater than 0.
     * \pre The sum of pCurBlock's block size and alignment adjustment must be greater than or equal to size.
     * \post If the alignment gap is used up, pCurBlock's alignment adjustment is set to 0.
     * \post If a new block is created, it is linked to pPrevBlock and pCurBlock's next block.
     * \post If the block becomes empty, it is removed from the free memory block list.
     */
    void shrinkBlock(MemoryBlock* pCurBlock, MemoryBlock* pPrevBlock, size_t size);
};

HeapManager* CreateHeapManager(void* pHeapBaseAddress, size_t heapSize, unsigned int numDescriptors);

inline void Destroy(const HeapManager* pHeapManager)
{
    assert(pHeapManager != nullptr);
	    
    pHeapManager->Destroy();
}

inline void* Alloc(HeapManager* pHeapManager, const size_t size)
{
    assert(pHeapManager != nullptr);

    return pHeapManager->Alloc(size, 1);
}

inline void* Alloc(HeapManager* pHeapManager, size_t size, size_t alignment)
{
    assert(pHeapManager != nullptr);

    return pHeapManager->Alloc(size, alignment);
}

inline bool Free(HeapManager* pHeapManager, const void* ptr)
{
    return pHeapManager->Free(ptr);
}

inline void Collect(const HeapManager* pHeapManager)
{
    pHeapManager->Collect();
}

inline void ShowFreeBlocks(const HeapManager* pHeapManager)
{
    pHeapManager->ShowFreeBlocks();
}

inline void ShowOutstandingAllocations(const HeapManager* pHeapManager)
{
    pHeapManager->ShowOutstandingAllocations();
}

inline size_t GetLargestFreeBlock(const HeapManager* pHeapManager)
{
    return pHeapManager->GetLargestFreeBlockSize();
}

inline bool Contains(const HeapManager* pHeapManager, void* ptr)
{
    return pHeapManager->Contains(ptr);
}

inline bool IsAllocated(const HeapManager* pHeapManager, const void* ptr)
{
    return pHeapManager->IsAllocated(ptr);
}

inline size_t GetAllOutstandingBlockSize(const HeapManager* pHeapManager)
{
	return pHeapManager->GetAllOutstandingBlockSize();
}

inline size_t GetAllFreeBlockSizes(const HeapManager* pHeapManager)
{
	return pHeapManager->GetAllFreeBlockSize();
}

#endif // HEAP_ALLOCATOR_H
