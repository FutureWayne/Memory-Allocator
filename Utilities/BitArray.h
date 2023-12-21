#pragma once
#include <cstdint>

#include "PointerMath.h"

#if _WIN32
typedef uint32_t t_BitData;
#else
typedef uint64_t t_BitData;
#endif // WIN32

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
    
    size_t bitsPerElement;
    size_t m_bitLength;
    size_t m_elementCount;
    t_BitData* m_pBits;

    void ClearAll() const;

    void SetAll() const;

    bool AreAllBitsClear() const;

    bool AreAllBitsSet() const;
    
    bool IsBitSet(size_t i_bitNumber) const;

    bool IsBitClear(size_t i_bitNumber) const;
    
    t_BitData* FindElementPtr(size_t idx) const;

    void SetBit(size_t i_bitNumber) const;

    void ClearBit(size_t i_bitNumber) const;
    
    bool FindFirstSetBit(size_t& o_firstSetBitIndex) const;
    
    bool FindFirstClearBit(size_t& o_firstClearBitIndex) const;

    bool operator[](size_t i_bitIndex) const;

private:
    /**
     * @brief Finds the specified bit in the BitArray object.
     *
     * This method searches for the specified bit in the BitArray object and returns
     * the index of the first occurrence of that bit. It also updates the output parameter
     * o_bitIndex with the found bit's index. If the bit is found, the method returns true,
     * otherwise, it returns false.
     *
     * @param findSetBit Flag indicating whether to find a set bit or a clear bit.
     *                   Pass 'true' to find set bit, 'false' to find clear bit.
     * @param o_bitIndex Output parameter that will hold the index of the found bit.
     *                   Must be passed by reference.
     *
     * @return True if the bit is found, false otherwise.
     */
    bool findBit(bool findSetBit, size_t& o_bitIndex) const;
};

inline BitArray* CreateBitArray(void* baseAddr, size_t i_numBits, bool i_bInitToZero)
{
    BitArray* pBitArray = static_cast<BitArray*>(baseAddr);

    // Initialize BitArray members
    pBitArray->m_bitLength = i_numBits;
    pBitArray->bitsPerElement = sizeof(t_BitData) * 8;
    pBitArray->m_elementCount = (i_numBits + pBitArray->bitsPerElement - 1) / pBitArray->bitsPerElement;
    
    const auto newAddress = PointerAdd(pBitArray, sizeof(BitArray));
    pBitArray->m_pBits = static_cast<t_BitData*>(newAddress);
    
    for (size_t i = 0; i < pBitArray->m_elementCount; i++)
    {
        pBitArray->m_pBits[i] = i_bInitToZero ? 0 : 1;
    }
    
    return pBitArray;
}
