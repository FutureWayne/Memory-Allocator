#pragma once

#include "HeapManager/HeapManager.h"
#include "FixedSizeAllocator/FixedSizeAllocator.h"

struct FSAInitData
{
   size_t blockSize;
   size_t blockNum;
};

FSAInitData g_FixedSizeAllocatorsInitData[] = {
   { 16, 100 },
   { 32, 100 },
   { 96, 100 },
   { 256, 100 },
   { 1024, 100 },
};

HeapManager* g_pHeapManager = nullptr;
FixedSizeAllocator** g_pFixedSizeAllocators = nullptr;
constexpr unsigned int g_FixedSizeAllocatorsCount = sizeof(g_FixedSizeAllocatorsInitData) / sizeof(FSAInitData);

// InitializeMemorySystem - initialize your memory system including your HeapManager and some FixedSizeAllocators
bool InitializeMemorySystem(void * i_pHeapMemory, size_t i_sizeHeapMemory, unsigned int i_OptionalNumDescriptors);

// Collect - coalesce free blocks in attempt to create larger blocks
void Collect();

// DestroyMemorySystem - destroy your memory systems
void DestroyMemorySystem();
