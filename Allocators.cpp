#include <inttypes.h>
#include <malloc.h>

#include <stdio.h>

#include "MemorySystem.h"


void * __cdecl malloc(size_t i_size)
{
	// Try to allocate memory from FixedSizeAllocators
	for (unsigned int i = 0; i < g_FixedSizeAllocatorsCount; i++)
	{
		if (i_size <= g_pFixedSizeAllocators[i]->m_blockSize)
		{
			void* ptr = g_pFixedSizeAllocators[i]->Alloc();
			if (ptr != nullptr)
				return ptr;
		}
	}

	// Too big for FixedSizeAllocators, try HeapManager
	return g_pHeapManager->Alloc(i_size, 4);
}

void __cdecl free(void * i_ptr)
{
	// Try to free memory from FixedSizeAllocators
	for (unsigned int i = 0; i < g_FixedSizeAllocatorsCount; i++)
	{
		if (g_pFixedSizeAllocators[i]->Contains(i_ptr))
		{
			g_pFixedSizeAllocators[i]->Free(i_ptr);
			return;
		}
	}

	// Try to free memory from HeapManager
	g_pHeapManager->Free(i_ptr);
}

void * operator new(size_t i_size)
{
	return malloc(i_size);
}

void operator delete(void * i_ptr)
{
	free(i_ptr);
}

void * operator new[](size_t i_size)
{
	return malloc(i_size);
}

void operator delete [](void * i_ptr)
{
	free(i_ptr);
}