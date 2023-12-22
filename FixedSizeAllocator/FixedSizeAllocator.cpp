#include "FixedSizeAllocator.h"

#define ENABLE_GUARDBANDS

// Define the guardband size and pattern
#ifdef ENABLE_GUARDBANDS
const size_t GUARDBAND_SIZE = 4;
const unsigned int GUARDBAND_PATTERN = 0xDEADBEEF;
#else
const size_t GUARDBAND_SIZE = 0; // No guardband
#endif

FixedSizeAllocator* CreateFixedSizeAllocator(size_t blockSize, size_t blockNum, void* heapBaseAddr)
{
    FixedSizeAllocator* pFixedSizeAllocator = static_cast<FixedSizeAllocator*>(heapBaseAddr);

    pFixedSizeAllocator->m_blockSize = blockSize;
    pFixedSizeAllocator->m_blockNum = blockNum;
    pFixedSizeAllocator->m_freeBlockNum = blockNum;
    pFixedSizeAllocator->m_BitArray = *CreateBitArray(&pFixedSizeAllocator->m_BitArray, blockNum, true);
    pFixedSizeAllocator->m_bitArraySize = reinterpret_cast<uintptr_t>(PointerSub(PointerAdd(&pFixedSizeAllocator->m_BitArray.m_pBits, pFixedSizeAllocator->m_BitArray.m_bitLength), reinterpret_cast<size_t>(&pFixedSizeAllocator->m_BitArray)));
    pFixedSizeAllocator->m_blockBaseAddr = PointerAdd(&pFixedSizeAllocator->m_BitArray, pFixedSizeAllocator->m_bitArraySize);
    return pFixedSizeAllocator;
}

FixedSizeAllocator::FixedSizeAllocator(
    const BitArray& bitArray, 
    size_t blockNum, size_t freeBlockNum, 
    size_t blockSize, size_t bitArraySize,
    void* blockBaseAddr)
    : m_blockNum(blockNum), m_freeBlockNum(freeBlockNum), m_blockSize(blockSize),
      m_bitArraySize(bitArraySize), m_blockBaseAddr(blockBaseAddr), m_BitArray(bitArray)
{
    
}

FixedSizeAllocator::~FixedSizeAllocator()
= default;

bool FixedSizeAllocator::Contains(const void* ptr) const
{
    return (ptr >= m_blockBaseAddr) && 
           (ptr < static_cast<char*>(m_blockBaseAddr) + m_blockSize * m_blockNum);
}

bool FixedSizeAllocator::IsAllocated(const void* ptr) const
{
    if (!Contains(ptr))
    {
        return false;
    }

    const size_t blockIndex = (static_cast<const char*>(ptr) - static_cast<const char*>(m_blockBaseAddr)) / m_blockSize;
    return m_BitArray.IsBitSet(blockIndex);
}

void* FixedSizeAllocator::Alloc()
{
    if (m_freeBlockNum == 0)
    {
        return nullptr;
    }

    for (size_t i = 0; i < m_blockNum; ++i)
    {
        if (!m_BitArray.IsBitSet(i))
        {
            m_BitArray.SetBit(i);
            m_freeBlockNum--;

            char* blockPtr = static_cast<char*>(m_blockBaseAddr) + i * (m_blockSize + 2 * GUARDBAND_SIZE);
            
#ifdef ENABLE_GUARDBANDS
            // Set guardband values
            *(reinterpret_cast<unsigned int*>(blockPtr)) = GUARDBAND_PATTERN;
            *(reinterpret_cast<unsigned int*>(blockPtr + GUARDBAND_SIZE + m_blockSize)) = GUARDBAND_PATTERN;
#endif

            return blockPtr + GUARDBAND_SIZE; // Return pointer to the actual block, skipping the front guardband
        }
    }

    return nullptr; // No free block found
}

bool FixedSizeAllocator::Free(void* ptr)
{
    if (!IsAllocated(ptr))
    {
        return false;
    }

    char* actualPtr = static_cast<char*>(ptr) - GUARDBAND_SIZE;
    const size_t blockIndex = (actualPtr - static_cast<char*>(m_blockBaseAddr)) / (m_blockSize + 2 * GUARDBAND_SIZE);

#ifdef ENABLE_GUARDBANDS
    // Check guardband integrity
    const unsigned int frontGuard = *(reinterpret_cast<unsigned int*>(actualPtr));
    const unsigned int backGuard = *(reinterpret_cast<unsigned int*>(actualPtr + GUARDBAND_SIZE + m_blockSize));
    if (frontGuard != GUARDBAND_PATTERN || backGuard != GUARDBAND_PATTERN)
    {
        return false;
    }
#endif

    m_BitArray.ClearBit(blockIndex);
    m_freeBlockNum++;
    return true;
}

void FixedSizeAllocator::Destroy() const
{
    // Cleanup
    m_BitArray.ClearAll();
}










