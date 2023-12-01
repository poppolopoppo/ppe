#pragma once

#include "IO/Archive.h"

#include "Allocator/Alloca.h"
#include "Container/Token.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void SerializeCompactSigned(FArchive& ar, TCompactInt<T>* value) {
    u8 b;
    if (ar.IsLoading()) {
        ar.Serialize(MakeRawView(b));
        const bool sign = !!(b & 0x80_u8); // extract sign bit
        T rd = checked_cast<T>(b & 0x3f_u8);
        if (!!(b & 0x40_u8)) { // has 2nd byte?
            for (T shift = 6; !!(b & 0x80_u8); shift += 7) {
                ar.Serialize(MakeRawView(b));
                rd |= checked_cast<T>(b & 0x7f_u8) << shift;
                if (not (b & 0x80_u8)) // no more byte?
                    break;
            }
        }

        value->Data = (sign ? -rd : rd);
    }
    else {
        T wr = value->Data;
        b = 0;
        if (wr < 0) {
            b |= 0x80_u8; // record sign bit
            wr = -wr;
        }
        b |= checked_cast<u8>(wr & 0x3f);
        if (wr <= 0x3f) {
            ar.Serialize(MakeRawView(b));
        }
        else {
            b |= 0x40_u8; // has 2nd byte
            wr >>= 6;
            ar.Serialize(MakeRawView(b));
            while (wr != 0) {
                b = checked_cast<u8>(wr & 0x3f);
                wr >>= 7;
                if (wr != 0)
                    b |= 0x80_u8; // has more bytes
                ar.Serialize(MakeRawView(b));
            }
        }
    }
}
//----------------------------------------------------------------------------
template <typename T>
void SerializeCompactUnsigned(FArchive& ar, TCompactInt<T>* value) {
    u8 b;
    if (ar.IsLoading()) {
        ar.Serialize(MakeRawView(b));
        T rd = checked_cast<T>(b & 0x7f_u8);
        for (T shift = 7; !!(b & 0x80); shift += 7) {
            ar.Serialize(MakeRawView(b));
            rd |= checked_cast<T>(b & 0x7f_u8) << shift;
        }
        value->Data = rd;
    }
    else {
        T wr = value->Data;
        for (;;) {
            b = checked_cast<u8>(wr & 0x7f);
            if (wr >>= 7; wr == 0) {
                ar.Serialize(MakeRawView(b));
                break;
            }
            else {
                b |= 0x80_u8; // has more bytes
                ar.Serialize(MakeRawView(b));
            }
        }
    }
}
//----------------------------------------------------------------------------
template <typename T>
void SerializeResizableArray(FArchive& ar, TResizable<T> array) {
    TCompactInt<u64> len{ checked_cast<u64>(array.Size()) };
    ar.SerializeCompactInt(&len);

    if (ar.IsLoading())
        array.Resize(checked_cast<size_t>(*len));

    using value_type = typename TResizable<T>::value_type;
    IF_CONSTEXPR(Meta::is_pod_v<value_type>) {
        ar.Serialize(array.RawView());
    } else {
        // do not care about endianness here (should be handled by <<)
        for (value_type& it : array.RawView())
            ar << &it;
    }
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
void SerializeToken(FArchive& ar, TToken<_Tag, _Char, _Sensitive, _TokenTraits>* token) {
    TCompactInt<u64> size{ checked_cast<u32>(token->size()) };
    ar.SerializeCompactInt(&size);

    if (ar.IsLoading()) {
        STACKLOCAL_POD_ARRAY(_Char, transient, *size);
        ar.Serialize(MakeRawView(transient));
        *token = MakeStringView(transient);
    }
    else {
        ar.Serialize(RemoveConstView(MakeRawView(token->MakeView())));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
