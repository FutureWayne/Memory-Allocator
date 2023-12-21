#include <Windows.h>

#include "MemorySystem.h"
#include "FixedSizeAllocator/FixedSizeAllocator.h"
#include "Utilities/BitArray.h"

#include <assert.h>
#include <algorithm>
#include <random>
#include <vector>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

bool MemorySystem_UnitTest();
bool BitArray_UnitTest();
bool FixSizeAllocator_UnitTest();

int main(int i_arg, char **)
{
	const size_t 		sizeHeap = 1024 * 1024;

	// you may not need this if you don't use a descriptor pool
	const unsigned int 	numDescriptors = 2048;

	// Allocate memory for my test heap.
	void * pHeapMemory = HeapAlloc(GetProcessHeap(), 0, sizeHeap);
	assert(pHeapMemory);

	// Create your HeapManager and FixedSizeAllocators.
	InitializeMemorySystem(pHeapMemory, sizeHeap, numDescriptors);

	bool success = MemorySystem_UnitTest();
	assert(success);

	success = BitArray_UnitTest();
	assert(success);

	success = FixSizeAllocator_UnitTest();
	assert(success);

	if (success)
	{
		printf("All unit test passed.\n");
	}

	// Clean up your Memory System (HeapManager and FixedSizeAllocators)
	DestroyMemorySystem();

	HeapFree(GetProcessHeap(), 0, pHeapMemory);

	// in a Debug build make sure we didn't leak any memory.
#if defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

	return 0;
}

bool MemorySystem_UnitTest()
{
	const size_t maxAllocations = 10 * 1024;
	std::vector<void *> AllocatedAddresses;

	long	numAllocs = 0;
	long	numFrees = 0;
	long	numCollects = 0;

	size_t totalAllocated = 0;

	// reserve space in AllocatedAddresses for the maximum number of allocation attempts
	// prevents new returning null when std::vector expands the underlying array
	AllocatedAddresses.reserve(10 * 1024);

	// allocate memory of random sizes up to 1024 bytes from the heap manager
	// until it runs out of memory
	do
	{
		const size_t		maxTestAllocationSize = 1024;

		size_t			sizeAlloc = 1 + (rand() & (maxTestAllocationSize - 1));

		void * pPtr = malloc(sizeAlloc);

		// if allocation failed see if garbage collecting will create a large enough block
		if (pPtr == nullptr)
		{
			Collect();

			pPtr = malloc(sizeAlloc);

			// if not we're done. go on to cleanup phase of test
			if (pPtr == nullptr)
				break;
		}

		AllocatedAddresses.push_back(pPtr);
		numAllocs++;

		totalAllocated += sizeAlloc;

		// randomly free and/or garbage collect during allocation phase
		const unsigned int freeAboutEvery = 0x07;
		const unsigned int garbageCollectAboutEvery = 0x07;

		if (!AllocatedAddresses.empty() && ((rand() % freeAboutEvery) == 0))
		{
			void * pPtrToFree = AllocatedAddresses.back();
			AllocatedAddresses.pop_back();

			free(pPtrToFree);
			numFrees++;
		}
		else if ((rand() % garbageCollectAboutEvery) == 0)
		{
			Collect();

			numCollects++;
		}

	} while (numAllocs < maxAllocations);

	// now free those blocks in a random order
	if (!AllocatedAddresses.empty())
	{
		// randomize the addresses
		std::shuffle(AllocatedAddresses.begin(), AllocatedAddresses.end(), std::default_random_engine());

		// return them back to the heap manager
		while (!AllocatedAddresses.empty())
		{
			void * pPtrToFree = AllocatedAddresses.back();
			AllocatedAddresses.pop_back();

			delete[] pPtrToFree;
		}

		// do garbage collection
		Collect();
		// our heap should be one single block, all the memory it started with

		// do a large test allocation to see if garbage collection worked
		void * pPtr = malloc(totalAllocated / 2);

		if (pPtr)
		{
			free(pPtr);
		}
		else
		{
			// something failed
			return false;
		}
	}
	else
	{
		return false;
	}

	// this new [] / delete [] pair should run through your allocator
	char * pNewTest = new char[1024];
	
	delete[] pNewTest;

	// we succeeded
	return true;
}

bool BitArray_UnitTest()
{
	size_t numBits = 64; // Example size
	BitArray* bitArray = CreateBitArray(malloc(sizeof(BitArray) + sizeof(t_BitData) * (numBits / 8)), numBits, true);

	// Test ClearAll and AreAllBitsClear
	bitArray->ClearAll();
	assert(bitArray->AreAllBitsClear() == true);

	// Test SetAll and AreAllBitsSet
	bitArray->SetAll();
	assert(bitArray->AreAllBitsSet() == true);

	// Test SetBit and IsBitSet
	bitArray->ClearAll();
	bitArray->SetBit(5);
	assert(bitArray->IsBitSet(5) == true);

	// Test ClearBit and IsBitClear
	bitArray->SetAll();
	bitArray->ClearBit(5);
	assert(bitArray->IsBitClear(5) == true);

	// Test FindFirstSetBit
	size_t firstSetBitIndex;
	bitArray->ClearAll();
	bitArray->SetBit(10);
	assert(bitArray->FindFirstSetBit(firstSetBitIndex) == true && firstSetBitIndex == 10);

	// Test FindFirstClearBit
	size_t firstClearBitIndex;
	bitArray->SetAll();
	bitArray->ClearBit(10);
	assert(bitArray->FindFirstClearBit(firstClearBitIndex) == true && firstClearBitIndex == 10);

	free(bitArray);

	return true;
}

bool FixSizeAllocator_UnitTest()
{
	// Setup
	const size_t blockSize = 32; // Example block size
	const size_t blockNum = 10;  // Number of blocks
	char heapBase[1024];         // Simulated heap space

	// Create a FixedSizeAllocator instance
	FixedSizeAllocator* allocator = CreateFixedSizeAllocator(blockSize, blockNum, heapBase);
	assert(allocator != nullptr);

	// Test allocation
	void* block1 = allocator->Alloc();
	assert(block1 != nullptr);

	// Test block is within the expected range
	assert(block1 >= heapBase && block1 < heapBase + sizeof(heapBase));

	// Test allocation until full
	void* lastBlock = nullptr;
	for (size_t i = 1; i < blockNum; ++i) {
		lastBlock = allocator->Alloc();
		assert(lastBlock != nullptr);
	}

	// Test over-allocation (should return nullptr)
	void* overflowBlock = allocator->Alloc();
	assert(overflowBlock == nullptr);

	// Test deallocation
	bool freeResult = allocator->Free(block1);
	assert(freeResult);

	// Test double free (should return false)
	freeResult = allocator->Free(block1);
	assert(!freeResult);

	// Test freeing a block not allocated (should return false)
	freeResult = allocator->Free(heapBase);
	assert(!freeResult);

	// Cleanup
	allocator->Destroy();

	return true;
}
