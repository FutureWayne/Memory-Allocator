#pragma once
#include <algorithm>
#include <cstdint>


/**
 * @class BitArray
 *
 * @brief Represents a dynamic array of bits.
 *
 * The BitArray class provides methods to manipulate individual bits within the array.
 * It supports operations like setting all bits, clearing all bits, checking if all bits are clear or set,
 * setting or clearing a specific bit, and finding the index of the first set or clear bit.
 */
class BitArray
{
#if defined(_WIN64) // For 64-bit Windows platforms
    typedef uint64_t t_BitData;
#else // For 32-bit platforms and non-Windows platforms
    typedef uint32_t t_BitData;
#endif

public:
    BitArray(size_t i_numBits, bool i_bInitToZero);
    ~BitArray();
    //static BitArray * Create(size_t i_numBits, bool i_startClear = true, HeapManager * i_pAllocator);

    void ClearAll(void) const;

    void SetAll(void) const;

    bool AreAllBitsClear(void) const;

    bool AreAllBitsSet(void) const;
    
    inline bool IsBitSet(size_t i_bitNumber) const;

    inline bool IsBitClear(size_t i_bitNumber) const;
    
    void SetBit(size_t i_bitNumber) const;

    void ClearBit(size_t i_bitNumber) const;
    
    bool FindFirstSetBit(size_t& o_firstSetBitIndex) const;
    
    bool FindFirstClearBit(size_t& o_firstClearBitIndex) const;

    bool operator[](size_t i_bitIndex) const;

private:
    t_BitData* m_pBits;
    size_t bitsPerElement = sizeof(t_BitData) * 8;
    size_t m_numBytes;

    bool findBit(bool findSetBit, size_t& o_bitIndex) const;
};


