#pragma once

#include "Core/Core.h"

#include "Core/HAL/PlatformEndian.h"
#include "Core/Memory/HashFunctions.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFourCC {
public:
    FFourCC() : _asU32(0) {}
    explicit FFourCC(u32 value) { _asU32 = value; }

    FFourCC(const FFourCC& other) : _asU32(other._asU32) {}
    FFourCC& operator =(const FFourCC& other) { _asU32 = other._asU32; return *this; }

    FFourCC(const char c0, const char c1, const char c2, const char c3) {
        Assert('\0' != c0);
        Assert('\0' != c1);
        Assert('\0' != c2);
        Assert('\0' != c3);

        _asChars[0] = c0;
        _asChars[1] = c1;
        _asChars[2] = c2;
        _asChars[3] = c3;
    }

    FFourCC(const char (&cstr)[5]) : FFourCC(cstr[0], cstr[1], cstr[2], cstr[3]) {
        Assert('\0' == cstr[4]);
    }

    u32 AsU32() const { return _asU32; }

    TMemoryView<const char> MakeView() const { return MakeConstView(_asChars); }

    friend bool operator ==(const FFourCC& lhs, const FFourCC& rhs) { return lhs._asU32 == rhs._asU32; }
    friend bool operator !=(const FFourCC& lhs, const FFourCC& rhs) { return lhs._asU32 != rhs._asU32; }

    friend bool operator < (const FFourCC& lhs, const FFourCC& rhs) { return lhs._asU32 <  rhs._asU32; }
    friend bool operator >=(const FFourCC& lhs, const FFourCC& rhs) { return lhs._asU32 >= rhs._asU32; }

    friend hash_t hash_value(const FFourCC& value) { return hash_as_pod(value._asU32); }

private:
    union {
        u32     _asU32;
        char    _asChars[4];
    };
};
//----------------------------------------------------------------------------
inline void SwapEndiannessInPlace(FFourCC& value) {
    FPlatformEndian::SwapInPlace((u32*)&value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
