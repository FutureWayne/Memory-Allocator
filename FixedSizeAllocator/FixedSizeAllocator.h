﻿#pragma once

#include "../Utilities/BitArray.h"

class FixedSizeAllocator
{
public:
    FixedSizeAllocator(
        const BitArray& bitArray, 
        size_t blockNum = 0, size_t freeBlockNum = 0, 
        size_t blockSize = 0, size_t bitArraySize = 0,
        void* blockBaseAddr = nullptr);
    
    ~FixedSizeAllocator();
    
    size_t m_blockNum;
    size_t m_freeBlockNum;
    size_t m_blockSize;
    size_t m_bitArraySize;
    void* m_blockBaseAddr;
    BitArray m_BitArray;
    
    bool Contains(const void* ptr) const;

    bool IsAllocated(const void* ptr) const;

    void* Alloc();
    
    bool Free(void* ptr);

    void Destroy() const;
};

FixedSizeAllocator* CreateFixedSizeAllocator(size_t blockSize, size_t blockNum, void* heapBaseAddr);
