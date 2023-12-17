#include "HeapManager.h"
#include <cstdio>
#include <tuple>

#include "../Utilities/PointerMath.h"


HeapManager* CreateHeapManager(void* pHeapMemory, size_t heapSize, unsigned int numDescriptors)
{
	assert(pHeapMemory != nullptr);
	assert(heapSize > 0);
	
	HeapManager* pHeapManager = static_cast<HeapManager*>(pHeapMemory);
	pHeapManager->m_heapSize = heapSize;
	pHeapManager->m_pHeapBaseAddress = pHeapMemory;
	
	void* pFirstMemoryBlock = static_cast<char*>(pHeapMemory) + sizeof(HeapManager);

	// Initialize the linked list of free memory blocks
	pHeapManager->m_pFreeMemoryBlockList = static_cast<MemoryBlock*>(pFirstMemoryBlock);
	pHeapManager->m_pFreeMemoryBlockList->pBaseAddress = PointerAdd(pFirstMemoryBlock, sizeof(MemoryBlock));
	pHeapManager->m_pFreeMemoryBlockList->BlockSize = pHeapManager->m_heapSize - sizeof(HeapManager) - sizeof(MemoryBlock);
	pHeapManager->m_pFreeMemoryBlockList->pNextBlock = nullptr;

	// Initialize the linked list of outstanding allocations (empty at the start)
	pHeapManager->m_pOutstandingAllocationList = nullptr;

	return pHeapManager;
}

void Destroy(const HeapManager* pHeapManager)
{
	assert(pHeapManager != nullptr);
	
	pHeapManager->Destroy();
}

HeapManager::HeapManager(void* pHeapMemory, size_t heapSize, unsigned numDescriptors)
{
	
}

HeapManager::~HeapManager()
= default;

void* HeapManager::Alloc(const size_t size, size_t alignment)
{
	assert(size > 0);
	
	const auto BlockPair = findFreeBlock(size + alignment);
	MemoryBlock* suitableBlock = BlockPair.first;
	MemoryBlock* previousBlock = BlockPair.second;
	
	// If no suitable block is found, attempt to de-fragment the heap
	if (!suitableBlock)
	{
		Collect();
		std::tie(suitableBlock, previousBlock) = findFreeBlock(size + alignment);
	}

	// If a suitable block is still not found after defragmentation, return nullptr
	if (!suitableBlock)
	{
		return nullptr;
	}

	// Calculate alignment offset
	size_t alignmentOffset;
	if (alignment == 0)
	{
		alignmentOffset = 0;
	}
	else
	{
		alignmentOffset = reinterpret_cast<uintptr_t>(suitableBlock->pBaseAddress) % alignment;
		if (alignmentOffset > 0)
		{
			alignmentOffset = alignment - alignmentOffset;
		}
	}

	// Create a new MemoryBlock structure to manage the allocated memory
	MemoryBlock* newBlock = static_cast<MemoryBlock*>(suitableBlock->pBaseAddress);
	newBlock->pBaseAddress = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(suitableBlock->pBaseAddress) + alignmentOffset + sizeof(MemoryBlock));
	newBlock->BlockSize = size;
	newBlock->AlignmentOffset = alignmentOffset;

	// Adjust the suitableBlock to represent the remaining free memory after the allocation
	suitableBlock->pBaseAddress = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(newBlock->pBaseAddress) + size + sizeof(MemoryBlock));
	suitableBlock->BlockSize -= (size + alignmentOffset + sizeof(MemoryBlock));

	// If the block's size is reduced to 0, remove it from the free list
	if (suitableBlock->BlockSize == 0)
	{
		if (previousBlock)
		{
			previousBlock->pNextBlock = suitableBlock->pNextBlock;
		}
		else
		{
			m_pFreeMemoryBlockList = suitableBlock->pNextBlock;
		}
	}

	// track allocation
	if (newBlock != m_pOutstandingAllocationList) {
		newBlock->pNextBlock = m_pOutstandingAllocationList;
		m_pOutstandingAllocationList = newBlock;
	} else {
		newBlock->pNextBlock = nullptr;
	}

	return newBlock->pBaseAddress;
}

bool HeapManager::Free(void* ptr)
	{
		assert(ptr);

		// Find the block in the outstanding allocation list
		MemoryBlock* currentBlock = m_pOutstandingAllocationList;
		MemoryBlock* previousBlock = nullptr;

		while (currentBlock)
		{
			if (currentBlock->pBaseAddress == ptr)
			{
				// Found the block to free

				// Remove the block from the outstanding allocation list
				if (previousBlock)
				{
					previousBlock->pNextBlock = currentBlock->pNextBlock;
				}
				else
				{
					m_pOutstandingAllocationList = currentBlock->pNextBlock;
				}

				// Adjust the base address and size based on the alignment offset
				uintptr_t adjustedAddress = reinterpret_cast<uintptr_t>(currentBlock->pBaseAddress) - currentBlock->AlignmentOffset - sizeof(MemoryBlock);
				currentBlock->pBaseAddress = reinterpret_cast<void*>(adjustedAddress);
				currentBlock->BlockSize += currentBlock->AlignmentOffset + sizeof(MemoryBlock);

				// Insert the block back to the free memory block list in the correct position
				MemoryBlock* freeBlock = m_pFreeMemoryBlockList;
				MemoryBlock* prevFreeBlock = nullptr;
				while (freeBlock && reinterpret_cast<uintptr_t>(freeBlock->pBaseAddress) < reinterpret_cast<uintptr_t>(ptr))
				{
					prevFreeBlock = freeBlock;
					freeBlock = freeBlock->pNextBlock;
				}

				if (prevFreeBlock)
				{
					prevFreeBlock->pNextBlock = currentBlock;
				}
				else
				{
					m_pFreeMemoryBlockList = currentBlock;
				}
				currentBlock->pNextBlock = freeBlock;

				return true;
			}

			previousBlock = currentBlock;
			currentBlock = currentBlock->pNextBlock;
		}

		// Block not found in the outstanding allocation list
		return false;
	}

void HeapManager::Destroy() const
{
	// Free all outstanding allocations
	const MemoryBlock* currentBlock = m_pOutstandingAllocationList;
	while (currentBlock)
	{
		const MemoryBlock* nextBlock = currentBlock->pNextBlock;
		delete currentBlock;
		currentBlock = nextBlock;
	}

	// Free all free memory blocks
	currentBlock = m_pFreeMemoryBlockList;
	while (currentBlock)
	{
		const MemoryBlock* nextBlock = currentBlock->pNextBlock;
		delete currentBlock;
		currentBlock = nextBlock;
	}
}

void HeapManager::ShowFreeBlocks() const
	{
		printf("Free Blocks:\n");
		MemoryBlock* currentBlock = m_pFreeMemoryBlockList;
		while (currentBlock)
		{
			printf("Free block base Address: %p, Size: %zu bytes\n", currentBlock->pBaseAddress, currentBlock->BlockSize);
			currentBlock = currentBlock->pNextBlock;
		}
	}

void HeapManager::ShowOutstandingAllocations() const
{
	printf("Outstanding Allocations:\n");
	MemoryBlock* currentBlock = m_pOutstandingAllocationList;
	while (currentBlock)
	{
		printf("Outstanding block base Address: %p, Size: %zu bytes\n", currentBlock->pBaseAddress, currentBlock->BlockSize);
		currentBlock = currentBlock->pNextBlock;
	}
}

size_t HeapManager::GetLargestFreeBlock() const
{
	size_t largestSize = 0;
	MemoryBlock* currentBlock = m_pFreeMemoryBlockList;
	while (currentBlock)
	{
		if (currentBlock->BlockSize > largestSize)
		{
			largestSize = currentBlock->BlockSize;
		}
		currentBlock = currentBlock->pNextBlock;
	}
	return largestSize;
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
	MemoryBlock* currentBlock = m_pOutstandingAllocationList;
	while (currentBlock)
	{
		if (currentBlock->pBaseAddress == ptr)
		{
			return true;
		}
		currentBlock = currentBlock->pNextBlock;
	}
	return false;
}

void HeapManager::Collect() const
{
	bool bShouldMerge;

	do
	{
		bShouldMerge = false;
		MemoryBlock* currentBlock = m_pFreeMemoryBlockList;

		while (currentBlock && currentBlock->pNextBlock)
		{
			uintptr_t currentBlockEnd = reinterpret_cast<uintptr_t>(currentBlock->pBaseAddress) + currentBlock->BlockSize + sizeof(MemoryBlock);
			uintptr_t nextBlockStart = reinterpret_cast<uintptr_t>(currentBlock->pNextBlock->pBaseAddress);

			// Check if the current block and the next block are adjacent
			if (currentBlockEnd == nextBlockStart)
			{
				// Merge the blocks
				currentBlock->BlockSize += currentBlock->pNextBlock->BlockSize + sizeof(MemoryBlock);

				// Remove the next block from the list
				MemoryBlock* blockToRemove = currentBlock->pNextBlock;
				currentBlock->pNextBlock = blockToRemove->pNextBlock;

				// merging two blocks might create a new opportunity for further merging
				bShouldMerge = true;
			}
			else
			{
				currentBlock = currentBlock->pNextBlock;
			}
		}
	} while (bShouldMerge);
}

std::pair<MemoryBlock*, MemoryBlock*> HeapManager::findFreeBlock(size_t size)
{
	MemoryBlock* currentBlock = m_pFreeMemoryBlockList;
	MemoryBlock* previousBlock = nullptr;
	while (currentBlock)
	{
		if (currentBlock->BlockSize >= size)
		{
			return {currentBlock, previousBlock};
		}
		previousBlock = currentBlock;
		currentBlock = currentBlock->pNextBlock;
	}
	return {nullptr, nullptr};
}

	

