#pragma once

inline void* PointerAdd(const void* ptr, size_t offset)
{
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) + offset);
}

inline void* PointerSub(const void* ptr, size_t offset)
{
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) - offset);
}