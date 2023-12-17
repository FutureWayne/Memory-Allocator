#include "MemorySystem.h"


bool InitializeMemorySystem(void * i_pHeapMemory, size_t i_sizeHeapMemory, unsigned int i_OptionalNumDescriptors)
{
	// Create FixedSizeAllocators
	g_pFixedSizeAllocators = new FixedSizeAllocator*[g_FixedSizeAllocatorsCount];
	for (unsigned int i = 0; i < g_FixedSizeAllocatorsCount; i++)
	{
		const size_t fixedSizeAllocatorSize = g_FixedSizeAllocatorsInitData[i].blockSize * g_FixedSizeAllocatorsInitData[i].blockNum;
		
		// Check if there is enough heap memory to create a FixedSizeAllocator
		if (i_sizeHeapMemory < fixedSizeAllocatorSize)
			return false;
		
		g_pFixedSizeAllocators[i] = CreateFixedSizeAllocator(g_FixedSizeAllocatorsInitData[i].blockSize, g_FixedSizeAllocatorsInitData[i].blockNum, i_pHeapMemory);
		if (g_pFixedSizeAllocators[i] == nullptr)
			return false;
		
		// Update heap memory pointer and keep track of remaining heap memory size
		i_pHeapMemory = static_cast<char*>(i_pHeapMemory) + fixedSizeAllocatorSize;
		i_sizeHeapMemory -= fixedSizeAllocatorSize;
	}

	// Create HeapManager
	g_pHeapManager = CreateHeapManager(i_pHeapMemory, i_sizeHeapMemory, i_OptionalNumDescriptors);

	return g_pHeapManager != nullptr;
}

void Collect()
{
	g_pHeapManager->Collect();
}

void DestroyMemorySystem()
{
	// Destroy your HeapManager and FixedSizeAllocators
	for (unsigned int i = 0; i < g_FixedSizeAllocatorsCount; i++)
	{
		g_pFixedSizeAllocators[i]->Destroy();
	}
	Destroy(g_pHeapManager);
}

