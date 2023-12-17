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
        size_t AlignmentOffset;
    };

    class HeapManager{
    public:
        HeapManager(void* pHeapMemory, size_t heapSize, unsigned int numDescriptors);
        ~HeapManager();

        size_t m_heapSize;
        void* m_pHeapBaseAddress;
        MemoryBlock* m_pFreeMemoryBlockList; // Linked list of free blocks
        MemoryBlock* m_pOutstandingAllocationList; // Linked list of allocated blocks
        
        void ShowFreeBlocks() const;
        void ShowOutstandingAllocations() const;
        size_t GetLargestFreeBlock() const;
        bool Contains(void* ptr) const;
        bool IsAllocated(void* ptr) const;
        void Collect() const;

        void* Alloc(const size_t size, size_t alignment);
        bool Free(void* ptr);
        void Destroy() const;

    private:
        std::pair<MemoryBlock*, MemoryBlock*> findFreeBlock(size_t size);

        MemoryBlock* createNewBlock(void* pBaseAddress, size_t size, MemoryBlock* pNextBlock);
    };

    HeapManager* CreateHeapManager(void* pHeapMemory, size_t heapSize, unsigned int numDescriptors);

    void Destroy(const HeapManager* pHeapManager);

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
        return pHeapManager->GetLargestFreeBlock();
    }

    inline bool Contains(HeapManager* pHeapManager, void* ptr)
    {
        return pHeapManager->Contains(ptr);
    }

    inline bool IsAllocated(HeapManager* pHeapManager, void* ptr)
    {
        return pHeapManager->IsAllocated(ptr);
    }

#endif // HEAP_ALLOCATOR_H
