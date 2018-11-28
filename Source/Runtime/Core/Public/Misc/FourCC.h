#pragma once

#include "Core.h"

#include "HAL/PlatformEndian.h"
#include "Memory/HashFunctions.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFourCC {
public:
    CONSTEXPR FFourCC() : _data{ 0 } {}
    CONSTEXPR explicit FFourCC(u32 value) : _data{ value } {}

    CONSTEXPR FFourCC(const FFourCC& other) : _data{ other._data } {}
    CONSTEXPR FFourCC& operator =(const FFourCC& other) {
        _data = other._data;
        return *this;
    }

    CONSTEXPR FFourCC(const char c0, const char c1, const char c2, const char c3)
        : _data{ (u32(c0) | (u32(c1) << 8) | (u32(c2) << 16) | (u32(c3) << 24)) }
    {}

    CONSTEXPR FFourCC(const char (&cstr)[5])
        : FFourCC(cstr[0], cstr[1], cstr[2], cstr[3])
    {}

    CONSTEXPR u32 AsU32() const { return _data; }

    TMemoryView<const char> MakeView() const { return TMemoryView<const char>((const char*)&_data, sizeof(_data)); }

    CONSTEXPR friend bool operator ==(const FFourCC& lhs, const FFourCC& rhs) { return lhs._data == rhs._data; }
    CONSTEXPR friend bool operator !=(const FFourCC& lhs, const FFourCC& rhs) { return lhs._data != rhs._data; }

    CONSTEXPR friend bool operator < (const FFourCC& lhs, const FFourCC& rhs) { return lhs._data <  rhs._data; }
    CONSTEXPR friend bool operator >=(const FFourCC& lhs, const FFourCC& rhs) { return lhs._data >= rhs._data; }

    friend hash_t hash_value(const FFourCC& value) { return hash_as_pod(value._data); }

private:
    u32 _data;
};
//----------------------------------------------------------------------------
inline void SwapEndiannessInPlace(FFourCC& value) {
    FPlatformEndian::SwapInPlace((u32*)&value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
