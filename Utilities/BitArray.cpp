#include "BitArray.h"

#include <cstring>
#include <intrin0.inl.h>

BitArray::BitArray() = default;

BitArray::~BitArray()
{
    delete[] m_pBits;
}

bool BitArray::FindFirstSetBit(size_t& o_firstSetBitIndex) const
{
    return findBit(true, o_firstSetBitIndex);
}

bool BitArray::FindFirstClearBit(size_t& o_firstClearBitIndex) const
{
    return findBit(false, o_firstClearBitIndex);
}

void BitArray::ClearAll(void) const
{
    memset(m_pBits, 0, m_bitLength / 8);
}

void BitArray::SetAll(void) const
{
    memset(m_pBits, 1, m_bitLength / 8);
}

bool BitArray::AreAllBitsClear(void) const 
{
    for (size_t i = 0; i < m_elementCount; ++i) {
        if (m_pBits[i] != 0) {
            return false;
        }
    }
    return true;
}

bool BitArray::AreAllBitsSet(void) const 
{
    for (size_t i = 0; i < m_elementCount; ++i) {
        if (m_pBits[i] != ~static_cast<t_BitData>(0)) {
            return false;
        }
    }
    return true;
}

inline bool BitArray::IsBitSet(size_t i_bitNumber) const 
{
    const size_t byteIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    return (m_pBits[byteIndex] & (static_cast<t_BitData>(1) << bitIndex)) != 0;
}

inline bool BitArray::IsBitClear(size_t i_bitNumber) const 
{
    const size_t byteIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    return (m_pBits[byteIndex] & (static_cast<t_BitData>(1) << bitIndex)) == 0;
}

void BitArray::SetBit(size_t i_bitNumber) const
{
    const size_t byteIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    m_pBits[byteIndex] |= (static_cast<t_BitData>(1) << bitIndex);
}

void BitArray::ClearBit(size_t i_bitNumber) const
{
    const size_t byteIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    m_pBits[byteIndex] &= ~(static_cast<t_BitData>(1) << bitIndex);
}

bool BitArray::operator[](size_t i_bitIndex) const
{
    return IsBitSet(i_bitIndex);
}

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
bool BitArray::findBit(bool findSetBit, size_t& o_bitIndex) const
{
    size_t elementIndex = 0;
    const t_BitData targetValue = findSetBit ? 0 : ~static_cast<t_BitData>(0);

    while ((m_pBits[elementIndex] == targetValue) && (elementIndex < m_elementCount)) {
        elementIndex++;
    }

    if (elementIndex == m_elementCount) {
        return false;
    }

    t_BitData Bits = findSetBit ? m_pBits[elementIndex] : ~m_pBits[elementIndex];
    unsigned long bitIndex;

#if WIN32
    _BitScanForward(&bitIndex, Bits);
#else
    _BitScanForward64(&bitIndex, Bits);
#endif

    o_bitIndex = elementIndex * (sizeof(t_BitData) * 8) + bitIndex;
    return true;
}
