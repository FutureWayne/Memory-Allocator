#pragma once

#include "HeapManager/HeapManager.h"
#include "FixedSizeAllocator/FixedSizeAllocator.h"

struct FSAInitData
{
   size_t blockSize;
   size_t blockNum;
};

extern HeapManager* g_pHeapManager;
extern FixedSizeAllocator** g_pFixedSizeAllocators;
extern const unsigned int g_FixedSizeAllocatorsCount;

// InitializeMemorySystem - initialize your memory system including your HeapManager and some FixedSizeAllocators
bool InitializeMemorySystem(void * i_pHeapMemory, size_t i_sizeHeapMemory, unsigned int i_OptionalNumDescriptors);

// Collect - coalesce free blocks in attempt to create larger blocks
void Collect();

// DestroyMemorySystem - destroy your memory systems
void DestroyMemorySystem();
