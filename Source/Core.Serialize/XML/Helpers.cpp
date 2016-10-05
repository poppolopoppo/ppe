#include "stdafx.h"

#include "Helpers.h"

#include "Core/Container/BitSet.h"
#include "Core/IO/StringView.h"
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
static bool ParseArray_(TArray< T >& dst, const FStringView& str, _Functor&& functor) {
    FStringView input = Strip(str);
    FStringView value;

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
static bool ParseArray_(const TMemoryView< T >& dst, const FStringView& str, _Functor&& functor) {
    FStringView input = Strip(str);
    FStringView value;

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
bool ParseEnumArray(TArray<FStringView>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](FStringView& dst, const FStringView& src) {
        dst = src;
        return true;
    });
}
//----------------------------------------------------------------------------
bool ParseEnumArray(TMemoryView<FStringView>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](FStringView& dst, const FStringView& src) {
        dst = src;
        return true;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(bool* dst, const FStringView& str) {
    Assert(dst);

    const FStringView input = Strip(str);

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
bool Parse(FBitSet& dst, const FStringView& str) {
    Assert(dst.size());

    FStringView input = Strip(str); // special case for booleans and bitset
    FStringView value;

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
bool Parse(i32* dst, const FStringView& str) {
    return Atoi32(dst, Strip(str), 10);
}
//----------------------------------------------------------------------------
bool Parse(TArray<i32>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](i32& dst, const FStringView& src) {
        return Atoi32(&dst, src, 10);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<i32>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](i32& dst, const FStringView& src) {
        return Atoi32(&dst, src, 10);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float* dst, const FStringView& str) {
    return Atof(dst, Strip(str));
}
//----------------------------------------------------------------------------
bool Parse(TArray<float>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](float& dst, const FStringView& src) {
        return Atof(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<float>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](float& dst, const FStringView& src) {
        return Atof(&dst, src);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(word2* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(float2* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(TArray<word2>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](word2& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(TArray<float2>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](float2& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<word2>& dst, const FStringView& str) {
    return Parse(dst.Cast<i32>(), str);
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<float2>& dst, const FStringView& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(word3* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(float3* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(TArray<word3>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](word3& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(TArray<float3>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](float3& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<word3>& dst, const FStringView& str) {
    return Parse(dst.Cast<i32>(), str);
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<float3>& dst, const FStringView& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(word4* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(float4* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(TArray<word4>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](word4& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(TArray<float4>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](float4& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<word4>& dst, const FStringView& str) {
    return Parse(dst.Cast<i32>(), str);
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<float4>& dst, const FStringView& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float2x2* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(TArray<float2x2>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](float2x2& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<float2x2>& dst, const FStringView& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float3x3* dst, const FStringView& str)  {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(TArray<float3x3>& dst, const FStringView& str)  {
    return ParseArray_(dst, str, [](float3x3& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<float3x3>& dst, const FStringView& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Parse(float4x4* dst, const FStringView& str) {
    Assert(dst);
    return Parse(dst->MakeView(), str);
}
//----------------------------------------------------------------------------
bool Parse(TArray<float4x4>& dst, const FStringView& str) {
    return ParseArray_(dst, str, [](float4x4& dst, const FStringView& src) {
        return Parse(&dst, src);
    });
}
//----------------------------------------------------------------------------
bool Parse(const TMemoryView<float4x4>& dst, const FStringView& str) {
    return Parse(dst.Cast<float>(), str);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
