#pragma once

#include "Core/Core.h"

#include <type_traits>

namespace Core {
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
template <typename T>
size_t hash_value(const T& value);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
size_t hash_value(T(&staticArray)[_Dim]);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
size_t hash_value(const _Arg0& arg0, const _Args&... args);
//----------------------------------------------------------------------------
template <typename T>
size_t hash_value_type(const T& value, typename std::enable_if<(sizeof(T) <= sizeof(size_t))>::type* = 0);
//----------------------------------------------------------------------------
template <typename T>
size_t hash_value_type(const T& value, typename std::enable_if<(sizeof(T) > sizeof(size_t)) && (sizeof(T) <= 2 * sizeof(size_t))>::type* = 0);
//----------------------------------------------------------------------------
template <typename _It>
size_t hash_value_seq(const _It& begin, const _It& end);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t hash_value(bool value) { return hash_value_type(value); }
inline size_t hash_value(int8_t value) { return hash_value_type(value); }
inline size_t hash_value(int16_t value) { return hash_value_type(value); }
inline size_t hash_value(int32_t value) { return hash_value_type(value); }
inline size_t hash_value(int64_t value) { return hash_value_type(value); }
inline size_t hash_value(uint8_t value) { return hash_value_type(value); }
inline size_t hash_value(uint16_t value) { return hash_value_type(value); }
inline size_t hash_value(uint32_t value) { return hash_value_type(value); }
inline size_t hash_value(uint64_t value) { return hash_value_type(value); }
inline size_t hash_value(float value) { return hash_value_type(value); }
inline size_t hash_value(double value) { return hash_value_type(value); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Hash-inl.h"
