#include "HeapManager.h"
#include "../Utilities/PointerMath.h"
#include <cstdio>
#include <tuple>

# define HEAP_MANAGER_OVERHEAD sizeof(HeapManager)
# define MEMORY_BLOCK_OVERHEAD sizeof(MemoryBlock)

HeapManager* CreateHeapManager(void* pHeapBaseAddress, size_t heapSize, unsigned int numDescriptors)
{
	assert(pHeapBaseAddress != nullptr);
	assert(heapSize > 0);
	
	HeapManager* pHeapManager = static_cast<HeapManager*>(pHeapBaseAddress);
	pHeapManager->Init(pHeapBaseAddress, heapSize, numDescriptors);

	return pHeapManager;
}


HeapManager::HeapManager()
= default;

HeapManager::~HeapManager()
= default;

void HeapManager::Init(void* pHeapBaseAddress, size_t heapSize, unsigned numDescriptors)
{
	m_pHeapBaseAddress = pHeapBaseAddress;
	m_heapSize = heapSize;
	
	// Initialize the linked list of free memory blocks
	MemoryBlock* pFirstMemoryBlock = createNewBlock(PointerAdd(pHeapBaseAddress, HEAP_MANAGER_OVERHEAD), heapSize - HEAP_MANAGER_OVERHEAD - MEMORY_BLOCK_OVERHEAD);
	m_pFreeMemoryBlockList = pFirstMemoryBlock;

	// Initialize the linked list of outstanding allocations (empty at the start)
	m_pOutstandingAllocationList = nullptr;
}

void* HeapManager::Alloc(size_t size, size_t alignment)
{
	assert(size > 0);
	
	MemoryBlock* pSuitableBlock;
	MemoryBlock* pPreviousBlock;
	std::tie(pSuitableBlock, pPreviousBlock) = findFreeBlock(size + MEMORY_BLOCK_OVERHEAD);
	
	// If no suitable block is found, attempt to de-fragment the heap
	if (!pSuitableBlock)
	{
		Collect();
		std::tie(pSuitableBlock, pPreviousBlock) = findFreeBlock(size);
	}

	// If a suitable block is still not found after defragmentation, return nullptr
	if (!pSuitableBlock)
	{
		return nullptr;
	}
	
	// Shrink the suitable block
	shrinkBlock(pSuitableBlock, pPreviousBlock, size);

	// Create allocated block
	MemoryBlock* pNewBlock = createNewBlock(pSuitableBlock, size);
	
	// track allocation
	pNewBlock->pNextBlock = m_pOutstandingAllocationList;
	m_pOutstandingAllocationList = pNewBlock;

	return pNewBlock->pBaseAddress;
}

bool HeapManager::Free(void* ptr)
	{
		assert(ptr);

		// Find the block in the outstanding allocation list
		MemoryBlock* pCurrentBlock = m_pOutstandingAllocationList;
		MemoryBlock* pPreviousBlock = nullptr;

		while (pCurrentBlock)
		{
			if (pCurrentBlock->pBaseAddress == ptr)
			{
				// Found the block to free

				// Remove the block from the outstanding allocation list
				if (pPreviousBlock)
				{
					pPreviousBlock->pNextBlock = pCurrentBlock->pNextBlock;
				}
				else
				{
					m_pOutstandingAllocationList = pCurrentBlock->pNextBlock;
				}

				// Insert the block back to the free memory block list in the correct position
				MemoryBlock* pFreeMemoryBlock = m_pFreeMemoryBlockList;
				MemoryBlock* pPrevMemoryBlock = nullptr;
				while (pFreeMemoryBlock && reinterpret_cast<uintptr_t>(pFreeMemoryBlock) < reinterpret_cast<uintptr_t>(pCurrentBlock))
				{
					pPrevMemoryBlock = pFreeMemoryBlock;
					pFreeMemoryBlock = pFreeMemoryBlock->pNextBlock;
				}

				if (pPrevMemoryBlock)
				{
					pPrevMemoryBlock->pNextBlock = pCurrentBlock;
				}
				else
				{
					m_pFreeMemoryBlockList = pCurrentBlock;
				}
				pCurrentBlock->pNextBlock = pFreeMemoryBlock;

				return true;
			}

			pPreviousBlock = pCurrentBlock;
			pCurrentBlock = pCurrentBlock->pNextBlock;
		}

		// Block not found in the outstanding allocation list
		return false;
	}

void HeapManager::Collect()
{
	bool bShouldMerge;

	do
	{
		bShouldMerge = false;
		MemoryBlock* pCurrentBlock = m_pFreeMemoryBlockList;

		while (pCurrentBlock && pCurrentBlock->pNextBlock)
		{
			const uintptr_t currentBlockEnd = reinterpret_cast<uintptr_t>(pCurrentBlock->pBaseAddress) + pCurrentBlock->BlockSize;
			const uintptr_t nextBlockStart = reinterpret_cast<uintptr_t>(pCurrentBlock->pNextBlock);

			// Check if the current block and the next block are adjacent
			if (currentBlockEnd == nextBlockStart)
			{
				// Merge the blocks
				pCurrentBlock->BlockSize += pCurrentBlock->pNextBlock->BlockSize + MEMORY_BLOCK_OVERHEAD;

				// Remove next block from the free block list
				const MemoryBlock* pNextBlock = pCurrentBlock->pNextBlock;
				pCurrentBlock->pNextBlock = pNextBlock->pNextBlock;
				pNextBlock = nullptr;
				
				// merging two blocks might create a new opportunity for further merging
				bShouldMerge = true;
			}
			else
			{
				pCurrentBlock = pCurrentBlock->pNextBlock;
			}
		}
	} while (bShouldMerge);
}

void HeapManager::Destroy() const
{
	// Free all outstanding allocations
	const MemoryBlock* pCurrentBlock = m_pOutstandingAllocationList;
	while (pCurrentBlock)
	{
		const MemoryBlock* nextBlock = pCurrentBlock->pNextBlock;
		delete pCurrentBlock;
		pCurrentBlock = nextBlock;
	}

	// Free all free memory blocks
	pCurrentBlock = m_pFreeMemoryBlockList;
	while (pCurrentBlock)
	{
		const MemoryBlock* nextBlock = pCurrentBlock->pNextBlock;
		pCurrentBlock = nullptr;
		pCurrentBlock = nextBlock;
	}
}

void HeapManager::ShowFreeBlocks() const
	{
		printf("Free Blocks:\n");
		MemoryBlock* pCurrentBlock = m_pFreeMemoryBlockList;
		while (pCurrentBlock)
		{
			printf("Free block Address: %p, Free block base Address: %p, Size: %zu bytes\n", pCurrentBlock, pCurrentBlock->pBaseAddress, pCurrentBlock->BlockSize);
			pCurrentBlock = pCurrentBlock->pNextBlock;
		}
	}

void HeapManager::ShowOutstandingAllocations() const
{
	printf("Outstanding Allocations:\n");
	MemoryBlock* pCurrentBlock = m_pOutstandingAllocationList;
	while (pCurrentBlock)
	{
		printf("Outstanding block Address: %p, Outstanding block base Address: %p, Size: %zu bytes\n", pCurrentBlock, pCurrentBlock->pBaseAddress, pCurrentBlock->BlockSize);
		pCurrentBlock = pCurrentBlock->pNextBlock;
	}
}

size_t HeapManager::GetLargestFreeBlockSize() const
{
	size_t largestSize = 0;
	MemoryBlock* pCurrentBlock = m_pFreeMemoryBlockList;
	while (pCurrentBlock)
	{
		if (pCurrentBlock->BlockSize > largestSize)
		{
			largestSize = pCurrentBlock->BlockSize;
		}
		pCurrentBlock = pCurrentBlock->pNextBlock;
	}
	return largestSize;
}

size_t HeapManager::GetAllOutstandingBlockSize() const
{
	size_t totalSize = 0;
	MemoryBlock* pCurrentBlock = m_pOutstandingAllocationList;
	while (pCurrentBlock)
	{
		totalSize += pCurrentBlock->BlockSize + MEMORY_BLOCK_OVERHEAD;
		pCurrentBlock = pCurrentBlock->pNextBlock;
	}
	return totalSize;
}

size_t HeapManager::GetAllFreeBlockSize() const
{
	size_t totalSize = 0;
	MemoryBlock* pCurrentBlock = m_pFreeMemoryBlockList;
	while (pCurrentBlock)
	{
		totalSize += pCurrentBlock->BlockSize + MEMORY_BLOCK_OVERHEAD;
		pCurrentBlock = pCurrentBlock->pNextBlock;
	}
	return totalSize;
}

bool HeapManager::Contains(void* ptr) const
{
	uintptr_t heapStart = reinterpret_cast<uintptr_t>(m_pHeapBaseAddress);
	uintptr_t heapEnd = heapStart + m_heapSize;
	uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
	return address >= heapStart && address < heapEnd;
}

bool HeapManager::IsAllocated(void* ptr) const
{
	MemoryBlock* pCurrentBlock = m_pOutstandingAllocationList;
	while (pCurrentBlock)
	{
		if (pCurrentBlock->pBaseAddress == ptr)
		{
			return true;
		}
		pCurrentBlock = pCurrentBlock->pNextBlock;
	}
	return false;
}

std::pair<MemoryBlock*, MemoryBlock*> HeapManager::	findFreeBlock(size_t size) const
{
	MemoryBlock* pCurrentBlock = m_pFreeMemoryBlockList;
	MemoryBlock* pPreviousBlock = nullptr;
	while (pCurrentBlock)
	{
		if (pCurrentBlock->BlockSize >= size)
		{
			return {pCurrentBlock, pPreviousBlock};
		}
		pPreviousBlock = pCurrentBlock;
		pCurrentBlock = pCurrentBlock->pNextBlock;
	}
	return {nullptr, nullptr};
}

MemoryBlock* HeapManager::createNewBlock(void* pBlockAddress, size_t size)
{
	MemoryBlock* newBlock = static_cast<MemoryBlock*>(pBlockAddress);
	newBlock->pBaseAddress = PointerAdd(pBlockAddress, MEMORY_BLOCK_OVERHEAD);
	newBlock->BlockSize = size;
	return newBlock;
}

void HeapManager::shrinkBlock(MemoryBlock* pCurBlock, MemoryBlock* pPrevBlock, size_t size)
{
	assert(pCurBlock != nullptr);
	assert(size > 0);
	assert(pCurBlock->BlockSize >= size);

	if (pCurBlock->BlockSize > size)
	{
		MemoryBlock* pShrunkBlock = nullptr;
		pShrunkBlock = createNewBlock(PointerAdd(pCurBlock, (size + MEMORY_BLOCK_OVERHEAD)), pCurBlock->BlockSize - size - MEMORY_BLOCK_OVERHEAD);

		if (pPrevBlock)
		{
			pPrevBlock->pNextBlock = pShrunkBlock;
			pShrunkBlock->pNextBlock = pCurBlock->pNextBlock;
		}
		else
		{
			pShrunkBlock->pNextBlock = pCurBlock->pNextBlock;
			m_pFreeMemoryBlockList = pShrunkBlock;
		}
	}
	else
	{
		// Remove the block from the free list
		if (pPrevBlock)
		{
			pPrevBlock->pNextBlock = pCurBlock->pNextBlock;
		}
		else
		{
			m_pFreeMemoryBlockList = pCurBlock->pNextBlock;
		}
	}
}