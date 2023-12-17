#pragma once
#include <cstdint>
#include <cstring>

#if _WIN32 // For 32-bit Windows platforms
typedef uint32_t t_BitData;
#else // For 64-bit platforms and non-Windows platforms
typedef uint64_t t_BitData;
#endif

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
public:
    BitArray();
    ~BitArray();

    t_BitData* m_pBits = nullptr;
    size_t bitsPerElement = sizeof(t_BitData) * 8;
    size_t m_bitLength = 0;
    size_t m_elementCount = 0;

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
    bool findBit(bool findSetBit, size_t& o_bitIndex) const;
};

inline BitArray* CreateBitArray(void* baseAddr, size_t i_numBits, bool i_bInitToZero)
{
    BitArray* pBitArray = static_cast<BitArray*>(baseAddr);
    
    pBitArray->m_bitLength = i_numBits;

    // Find the number of elements needed to store the bits
    pBitArray->m_elementCount = i_numBits + pBitArray->bitsPerElement - 1 / pBitArray->bitsPerElement;
    
    // Allocate the memory for the bits
    for (size_t i = 0; i < pBitArray->m_elementCount; ++i) {
        pBitArray->m_pBits[i] = i_bInitToZero ? 0 : ~static_cast<t_BitData>(0);
    }
    
    return pBitArray;
}



