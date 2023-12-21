#include "BitArray.h"

#include <intrin0.inl.h>

BitArray::BitArray() = default;

BitArray::~BitArray()
= default;

bool BitArray::FindFirstSetBit(size_t& o_firstSetBitIndex) const
{
    return findBit(true, o_firstSetBitIndex);
}

bool BitArray::FindFirstClearBit(size_t& o_firstClearBitIndex) const
{
    return findBit(false, o_firstClearBitIndex);
}

void BitArray::ClearAll() const {
    memset(m_pBits, 0, sizeof(t_BitData) * m_elementCount);
}

void BitArray::SetAll() const {
    memset(m_pBits, 0xFF, sizeof(t_BitData) * m_elementCount);
}

bool BitArray::AreAllBitsClear() const 
{
    for (size_t i = 0; i < m_elementCount; ++i) {
        if (m_pBits[i] != 0) {
            return false;
        }
    }
    return true;
}

bool BitArray::AreAllBitsSet() const 
{
    for (size_t i = 0; i < m_elementCount; ++i) {
        if (m_pBits[i] != ~static_cast<t_BitData>(0)) {
            return false;
        }
    }
    return true;
}

bool BitArray::IsBitSet(size_t i_bitNumber) const 
{
    const size_t byteIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    return (m_pBits[byteIndex] & (static_cast<t_BitData>(1) << bitIndex)) != 0;
}

bool BitArray::IsBitClear(size_t i_bitNumber) const 
{
    const size_t byteIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    return (m_pBits[byteIndex] & (static_cast<t_BitData>(1) << bitIndex)) == 0;
}

t_BitData* BitArray::FindElementPtr(size_t idx) const
{
    return &m_pBits[idx];
}

void BitArray::SetBit(size_t i_bitNumber) const
{
    const size_t elementIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    m_pBits[elementIndex] |= (static_cast<t_BitData>(1) << bitIndex);
}

void BitArray::ClearBit(size_t i_bitNumber) const
{
    const size_t elementIndex = i_bitNumber / bitsPerElement;
    const size_t bitIndex = i_bitNumber % bitsPerElement;
    m_pBits[elementIndex] &= ~(static_cast<t_BitData>(1) << bitIndex);
}

bool BitArray::operator[](size_t i_bitIndex) const
{
    return IsBitSet(i_bitIndex);
}


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

#if _WIN32
    _BitScanForward(&bitIndex, Bits);
#else
    _BitScanForward64(&bitIndex, Bits);
#endif

    o_bitIndex = elementIndex * (sizeof(t_BitData) * 8) + bitIndex;
    return true;
}
