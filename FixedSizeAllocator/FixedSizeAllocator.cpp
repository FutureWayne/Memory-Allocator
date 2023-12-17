#include "FixedSizeAllocator.h"

FixedSizeAllocator* CreateFixedSizeAllocator(size_t blockSize, size_t blockNum, void* heapBaseAddr)
{
    FixedSizeAllocator* pFixedSizeAllocator = static_cast<FixedSizeAllocator*>(heapBaseAddr);

    pFixedSizeAllocator->m_blockSize = blockSize;
    pFixedSizeAllocator->m_blockNum = blockNum;
    pFixedSizeAllocator->m_freeBlockNum = blockNum;
    pFixedSizeAllocator->m_pBitArray = CreateBitArray(&pFixedSizeAllocator->m_pBitArray, blockNum, true);
    pFixedSizeAllocator->m_bitArraySize = reinterpret_cast<uintptr_t>(

    return pFixedSizeAllocator;
}


FixedSizeAllocator::FixedSizeAllocator(
    BitArray* bitArray, 
    size_t blockNum, size_t freeBlockNum, 
    size_t blockSize, size_t bitArraySize,
    void* blockBaseAddr)
    : m_pBitArray(bitArray), m_blockNum(blockNum), m_freeBlockNum(freeBlockNum),
      m_blockSize(blockSize), m_bitArraySize(bitArraySize), m_blockBaseAddr(blockBaseAddr)
{
    
}

FixedSizeAllocator::~FixedSizeAllocator()
{
    
}

bool FixedSizeAllocator::Contains(const void* ptr) const
{
    return (ptr >= m_blockBaseAddr) && 
           (ptr < static_cast<char*>(m_blockBaseAddr) + m_blockSize * m_blockNum);
}

bool FixedSizeAllocator::IsAllocated(const void* ptr) const
{
    if (!Contains(ptr)) {
        return false;
    }

    const size_t blockIndex = (static_cast<const char*>(ptr) - static_cast<const char*>(m_blockBaseAddr)) / m_blockSize;
    return m_pBitArray->IsBitSet(blockIndex);
}

void* FixedSizeAllocator::Alloc()
{
    if (m_freeBlockNum == 0) {
        return nullptr;
    }

    for (size_t i = 0; i < m_blockNum; ++i) {
        if (!m_pBitArray->IsBitSet(i)) {
            m_pBitArray->SetBit(i);
            m_freeBlockNum--;
            return static_cast<char*>(m_blockBaseAddr) + i * m_blockSize;
        }
    }

    return nullptr; // No free block found
}

bool FixedSizeAllocator::Free(void* ptr)
{
    if (!IsAllocated(ptr)) {
        return false;
    }

    const size_t blockIndex = (static_cast<char*>(ptr) - static_cast<char*>(m_blockBaseAddr)) / m_blockSize;
    m_pBitArray->ClearBit(blockIndex);
    m_freeBlockNum++;
    return true;
}

void FixedSizeAllocator::Destroy() const
{
    // Cleanup
    m_pBitArray->ClearAll();
    delete m_pBitArray;
}










