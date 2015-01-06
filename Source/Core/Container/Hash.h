#pragma once

#include "Core/Core.h"

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE u32 float_bits(float f) { return *((u32 *)((void *)&f)); }
FORCE_INLINE u64 double_bits(double f) { return *((u64 *)((void *)&f)); }
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_value(bool value)      { return value ? 1 : 0; }
FORCE_INLINE size_t hash_value(int8_t value)    { return size_t(value); }
FORCE_INLINE size_t hash_value(int16_t value)   { return size_t(value); }
FORCE_INLINE size_t hash_value(int32_t value)   { return size_t(value); }
FORCE_INLINE size_t hash_value(int64_t value)   { return size_t(value ^ (value >> 32)); }
FORCE_INLINE size_t hash_value(uint8_t value)   { return size_t(value); }
FORCE_INLINE size_t hash_value(uint16_t value)  { return size_t(value); }
FORCE_INLINE size_t hash_value(uint32_t value)  { return size_t(value); }
FORCE_INLINE size_t hash_value(uint64_t value)  { return size_t(value ^ (value >> 32)); }
FORCE_INLINE size_t hash_value(float value)     { return hash_value(float_bits(value)); }
FORCE_INLINE size_t hash_value(double value)    { return hash_value(double_bits(value)); }
FORCE_INLINE size_t hash_value(const void *ptr) { return hash_value(size_t(ptr)); }
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_value(const std::thread::id& id) { return id.hash(); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_value_as_memory(const void *ptr, size_t sizeInBytes);
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE size_t hash_value_as_memory(const T& value) { return hash_value_as_memory(&value, sizeof(T)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE size_t hash_value(const T& value, typename std::enable_if< true == std::is_pointer<T>::value >::type * ) { return hash_value((const void *)value); }
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE size_t hash_value(const T& value, typename std::enable_if< false == std::is_pointer<T>::value >::type * ) { return hash_value_as_memory(&value, sizeof(T)); }
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Arg1, typename... _Args>
size_t hash_value(const _Arg0& arg0, const _Arg1& arg1, const _Args&... args);
//----------------------------------------------------------------------------
template <typename _It>
size_t hash_value_seq(const _It& begin, const _It& end);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE size_t hash_value(T (&staticArray)[_Dim]) { return hash_value_seq(&staticArray[0], &staticArray[_Dim]); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Enables ADL/KL in Core namespace
//----------------------------------------------------------------------------
template <typename T>
struct Hash : public std::unary_function<T, size_t> {
    size_t operator ()(const T& value) {
        return hash_value(value);
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct EqualTo : public std::equal_to<T> {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Hash-inl.h"
