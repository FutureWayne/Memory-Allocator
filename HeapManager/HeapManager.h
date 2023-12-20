#ifndef HEAP_ALLOCATOR_H
#define HEAP_ALLOCATOR_H

#include <cstddef>
#include <utility>
#include <cassert>

struct MemoryBlock
{
    void* pBaseAddress;
    size_t BlockSize;
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

        void Init(void* pHeapBaseAddress, size_t heapSize, unsigned int numDescriptors);
        void ShowFreeBlocks() const;
        void ShowOutstandingAllocations() const;
        bool Contains(void* ptr) const;
        bool IsAllocated(void* ptr) const;
        size_t GetLargestFreeBlockSize() const;
		size_t GetAllOutstandingBlockSize() const;
		size_t GetAllFreeBlockSize() const;
    
        void* Alloc(size_t size, size_t alignment);
        bool Free(void* ptr);
        void Collect();
        void Destroy() const;

    private:
        std::pair<MemoryBlock*, MemoryBlock*> findFreeBlock(size_t size) const;
        static MemoryBlock* createNewBlock(void* pBlockAddress, size_t size);
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

    return pHeapManager->Alloc(size, 0);
}

inline void* Alloc(HeapManager* pHeapManager, size_t size, size_t alignment)
{
    assert(pHeapManager != nullptr);

    return pHeapManager->Alloc(size, alignment);
}

inline bool Free(HeapManager* pHeapManager, void* ptr)
{
    return pHeapManager->Free(ptr);
}

inline void Collect(HeapManager* pHeapManager)
{
    pHeapManager->Collect();
}

inline void ShowFreeBlocks(HeapManager* pHeapManager)
{
    pHeapManager->ShowFreeBlocks();
}

inline void ShowOutstandingAllocations(HeapManager* pHeapManager)
{
    pHeapManager->ShowOutstandingAllocations();
}

inline size_t GetLargestFreeBlock(HeapManager* pHeapManager)
{
    return pHeapManager->GetLargestFreeBlockSize();
}

inline bool Contains(HeapManager* pHeapManager, void* ptr)
{
    return pHeapManager->Contains(ptr);
}

inline bool IsAllocated(HeapManager* pHeapManager, void* ptr)
{
    return pHeapManager->IsAllocated(ptr);
}

inline size_t GetAllOutstandingBlockSize(HeapManager* pHeapManager)
{
	return pHeapManager->GetAllOutstandingBlockSize();
}

inline size_t GetAllFreeBlockSizes(HeapManager* pHeapManager)
{
	return pHeapManager->GetAllFreeBlockSize();
}

#endif // HEAP_ALLOCATOR_H
