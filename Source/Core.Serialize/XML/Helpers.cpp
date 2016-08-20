#include "stdafx.h"

#include "Helpers.h"

#include "Core/Container/BitSet.h"
#include "Core/IO/StringSlice.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T, typename _Functor>
static bool ParseArray_(Array< T >& dst, const StringSlice& str, _Functor&& functor) {
    StringSlice input = Strip(str);
    StringSlice value;

    while (Split(input, ' ', value)) {
        Assert(!value.empty());

        if (not functor(dst.push_back_Default(), value)) {
            dst.pop_back();
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Functor>
static bool ParseArray_(const MemoryView< T >& dst, const StringSlice& str, _Functor&& functor) {
    StringSlice input = Strip(str);
    StringSlice value;

    size_t count = 0;
    while (Split(input, ' ', value)) {
        Assert(!value.empty());

        if (count == dst.size())
            return false;

        if (not functor(dst[count++], value))
            return false;
    }

    return (dst.size() == count);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool ParseEnumArray(Array<StringSlice>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](StringSlice& dst, const StringSlice& src) {
        dst = src;
        return true;
    });
}
//----------------------------------------------------------------------------
bool ParseEnumArray(MemoryView<StringSlice>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](StringSlice& dst, const StringSlice& src) {
        dst = src;
        return true;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(bool* dst, const StringSlice& str) {
    Assert(dst);

    const StringSlice input = Strip(str);

    if (EqualsI(input, "true")) {
        *dst = true;
        return true;
    }
    else if (EqualsI(input, "false")) {
        *dst = false;
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool Parse(BitSet& dst, const StringSlice& str) {
    Assert(dst.size());

    StringSlice input = Strip(str); // special case for booleans and bitset
    StringSlice value;

    size_t count = 0;
    while (Split(input, ' ', value)) {
        Assert(!value.empty());
        if (count == dst.size())
            return false;

        bool b = false;
        if (not Parse(&b, value))
            return false;

        dst.Set(count++, b);
    }

    return (dst.size() == count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(i32* dst, const StringSlice& str) {
    return Atoi32(dst, Strip(str), 10);
}
//----------------------------------------------------------------------------
bool Parse(Array<i32>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](i32& dst, const StringSlice& src) {
        return Atoi32(&dst, src, 10);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<i32>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](i32& dst, const StringSlice& src) {
        return Atoi32(&dst, src, 10);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float* dst, const StringSlice& str) {
    return Atof(dst, Strip(str));
}
//----------------------------------------------------------------------------
bool Parse(Array<float>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](float& dst, const StringSlice& src) {
        return Atof(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<float>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](float& dst, const StringSlice& src) {
        return Atof(&dst, src);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(word2* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(float2* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(Array<word2>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](word2& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(Array<float2>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](float2& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<word2>& dst, const StringSlice& str) {
    return Parse(dst.Cast<i32>(), str);
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<float2>& dst, const StringSlice& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(word3* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(float3* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(Array<word3>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](word3& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(Array<float3>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](float3& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<word3>& dst, const StringSlice& str) {
    return Parse(dst.Cast<i32>(), str);
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<float3>& dst, const StringSlice& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(word4* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(float4* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(Array<word4>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](word4& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(Array<float4>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](float4& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<word4>& dst, const StringSlice& str) {
    return Parse(dst.Cast<i32>(), str);
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<float4>& dst, const StringSlice& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float2x2* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(Array<float2x2>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](float2x2& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<float2x2>& dst, const StringSlice& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float3x3* dst, const StringSlice& str)  {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(Array<float3x3>& dst, const StringSlice& str)  {
    return ParseArray_(dst, str, [](float3x3& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<float3x3>& dst, const StringSlice& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float4x4* dst, const StringSlice& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(Array<float4x4>& dst, const StringSlice& str) {
    return ParseArray_(dst, str, [](float4x4& dst, const StringSlice& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const MemoryView<float4x4>& dst, const StringSlice& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
