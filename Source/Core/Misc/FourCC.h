#pragma once

#include "Core/Core.h"

#include "Core/Memory/HashFunctions.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FourCC {
public:
    FourCC() : _asU32(0) {}

    FourCC(const FourCC& other) : _asU32(other._asU32) {}
    FourCC& operator =(const FourCC& other) { _asU32 = other._asU32; return *this; }

    FourCC(const char c0, const char c1, const char c2, const char c3) {
        Assert('\0' != c0);
        Assert('\0' != c1);
        Assert('\0' != c2);
        Assert('\0' != c3);

        _asChars[0] = c0;
        _asChars[1] = c1;
        _asChars[2] = c2;
        _asChars[3] = c3;
    }
    FourCC(const char* cstr) {
        Assert('\0' != cstr[0]);
        Assert('\0' != cstr[1]);
        Assert('\0' != cstr[2]);
        Assert('\0' != cstr[3]);

        _asChars[0] = cstr[0];
        _asChars[1] = cstr[1];
        _asChars[2] = cstr[2];
        _asChars[3] = cstr[3];
    }

    u32 AsU32() const { return _asU32; }

    MemoryView<const char> MakeView() const { return MakeConstView(_asChars); }

    friend bool operator ==(const FourCC& lhs, const FourCC& rhs) { return lhs._asU32 == rhs._asU32; }
    friend bool operator !=(const FourCC& lhs, const FourCC& rhs) { return lhs._asU32 != rhs._asU32; }

    friend bool operator < (const FourCC& lhs, const FourCC& rhs) { return lhs._asU32 <  rhs._asU32; }
    friend bool operator >=(const FourCC& lhs, const FourCC& rhs) { return lhs._asU32 >= rhs._asU32; }

    friend hash_t hash_value(const FourCC& value) { return hash_as_pod(value._asU32); }

private:
    union {
        u32     _asU32;
        char    _asChars[4];
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
