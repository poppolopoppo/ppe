#include "stdafx.h"


#include "Maths/ScalarVectorHelpers.h"
#include "Maths/ScalarBoundingBoxHelpers.h"
#include "Maths/ScalarMatrixHelpers.h"

#include "Diagnostic/Logger.h"
#include "Meta/Optional.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Maths)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static NO_INLINE void Test_Iterable_() {
    constexpr auto is_odd = [](int x) constexpr -> bool { return !!(x & 1); };
    constexpr auto is_even = [](int x) constexpr -> bool { return !(x & 1); };
    constexpr auto sqr = [](int x) constexpr -> int { return x * x; };

    static constexpr const int values[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    auto range = MakeIterable(values);

    using iterator_t = Meta::TIteratorTraits<decltype(range)::iterator>;
    STATIC_ASSERT(std::is_same_v<iterator_t::value_type, int>);
    STATIC_ASSERT(std::is_same_v<iterator_t::reference, const int&>);
    STATIC_ASSERT(std::is_same_v<iterator_t::pointer, const int*>);

    AssertRelease(lengthof(values) == std::distance(range.begin(), range.end()));

    const size_t count_all = range.size();
    const size_t count_odd = range.CountIf(is_odd);
    const size_t count_even = range.CountIf(is_even);
    AssertRelease(5 == count_odd);
    AssertRelease(4 == count_even);
    AssertRelease(count_all == count_even + count_odd);

    const size_t count_odd2 = range.FilterBy(is_odd).size();
    const size_t count_even2 = range.FilterBy(is_even).size();
    AssertRelease(count_odd2 == count_odd);
    AssertRelease(count_even2 == count_even);

    const int sum_all = range.Accumulate();
    const int sum_odd = range.FilterBy(is_odd).Accumulate();
    const int sum_even = range.FilterBy(is_even).Accumulate();
    AssertRelease(25 == sum_odd);
    AssertRelease(20 == sum_even);
    AssertRelease(sum_all == sum_odd + sum_even);

    const int sqr_all = range.MapReduce(sqr, Meta::TPlus<>{});
    const int sqr_odd = range.FilterBy(is_odd).MapReduce(sqr, Meta::TPlus<>{});
    const int sqr_even = range.FilterBy(is_even).MapReduce(sqr, Meta::TPlus<>{});
    AssertRelease(285 == sqr_all);
    AssertRelease(165 == sqr_odd);
    AssertRelease(120 == sqr_even);
    AssertRelease(sqr_all == sqr_odd + sqr_even);

    const int sqr_odd2 = range.Select([&](int x) NOEXCEPT {
        Meta::TOptional<int> ret;
        if (is_odd(x)) ret.emplace(sqr(x));
        return ret;
    }).Accumulate();
    const int sqr_even2 = range.Select([&](const int& x) {
        Meta::TOptional<int> ret;
        if (is_even(x)) ret.emplace(sqr(x));
        return ret;
    }).Accumulate();
    AssertRelease(165 == sqr_odd2);
    AssertRelease(120 == sqr_even2);
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Vector_() {
    {
        CONSTEXPR int2 x = int2::X;
        CONSTEXPR int2 y = int2::Y;
        CONSTEXPR int2 ad = ((x + y) * (x - y) * 10) / 5;
        STATIC_ASSERT(ad == int2(2,-2));
        CONSTEXPR int2 mb = Blend(x, y, x > y);
        STATIC_ASSERT(mb == int2(1,1));
        CONSTEXPR int2 ma = Max(x, y);
        CONSTEXPR int2 mi = Min(x, y);
        STATIC_ASSERT(ma == int2::One);
        STATIC_ASSERT(mi == int2::Zero);
        CONSTEXPR int norm = Dot(ma, ma);
        STATIC_ASSERT(norm == 2);
    }
    {
        float4 x = float4::X;
        float4 y = float4::Y;
        float4 z = float4::Z;
        float4 w = float4::W;
        float4 a = Max(x, y);
        AssertRelease(a == float4(1,1,0,0));
        float4 b = Max(z, w);
        AssertRelease(b == float4(0,0,1,1));
        float4 c = Max(Max(x, y), Max(z, w));
        AssertRelease(c == float4::One);
        float4 d = Max3(x, y, z);
        AssertRelease(d == float4(float3::One, 0));
        float3 e = Max3(x.xyz, y.xyz, z.xyz);
        AssertRelease(e == float3::One);
        float4 f = Max3(Max(x, y), z, w);
        AssertRelease(f == float4::One);
        float4 m = Max3(x, y, Max(z, w));
        AssertRelease(m == float4::One);
        ubyte4 n = Float01_to_UByte0255(m);
        AssertRelease(n == ubyte4(UINT8_MAX));
    }
    {
        int3 x = int3::X;
        int3 y = int3::Y;
        int3 z = int3::Z;
        int3 a = x + y + z;
        AssertRelease(a == float3::One);
    }
    {
        int3 x = int3::X;
        bool3 m = x > int3(0);
        AssertRelease(Any( m));
        AssertRelease(not All( m));
        int3 select = Blend(int3::MinusOne, int3::One, m);
        int broadcast = Dot(select);
        AssertRelease(broadcast == 1);
    }

}

//----------------------------------------------------------------------------
static NO_INLINE void Test_BoundingBox_() {
    {
        FAabb3f box;
        AssertRelease(not box.HasPositiveExtents());
        AssertRelease(not box.HasPositiveExtentsStrict());
        box.Add(float3::Zero);
        AssertRelease(box.HasPositiveExtents());
        AssertRelease(not box.HasPositiveExtentsStrict());
        box.Add(float3::One);
        AssertRelease(box.HasPositiveExtentsStrict());
    }
    {
        FAabb3i box;
        AssertRelease(not box.HasPositiveExtents());
        AssertRelease(not box.HasPositiveExtentsStrict());
        box.Add(int3::Zero);
        box.Add(int3::X);
        box.Add(int3::Y);
        AssertRelease(box.HasPositiveExtents());
        AssertRelease(not box.HasPositiveExtentsStrict());
        box.Add(int3::Z);
        AssertRelease(box.HasPositiveExtentsStrict());
        FAabb2i box2D = box.xy();
        AssertRelease(box2D.HasPositiveExtentsStrict());
        int3 ext = box.Extents();
        AssertRelease(ext == int3::One);
        AssertRelease(box.Contains(int3::Zero));
        AssertRelease(box.Contains(int3::One));
        AssertRelease(not box.Contains(int3::MinusOne));
        AssertRelease(not box.ContainsStrict(int3::Zero));
        AssertRelease(not box.ContainsStrict(int3::One));
        AssertRelease(box.ContainsMaxStrict(int3::Zero));
        AssertRelease(not box.ContainsMaxStrict(int3::One));
    }
    {
        FAabb3f unit = FAabb3f::MinusOneOneValue();
        FAabb3f boxes[] = {
            unit - float3(1.5f),
            unit + float3(1.5f),
            unit
        };
        AssertRelease(boxes[2].Intersects(boxes[0]));
        AssertRelease(boxes[2].Intersects(boxes[1]));
        AssertRelease(not boxes[0].Intersects(boxes[1]));
        FAabb3f all = MakeBoundingBox(MakeView(boxes));
        AssertRelease(all.Contains(boxes[0]));
        AssertRelease(all.Contains(boxes[1]));
        AssertRelease(all.Contains(boxes[2]));
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Matrix_() {
    {
        int4x3 m = float4x3::Identity();
        int3x4 t = m.Transpose();
        int4x3 n = t.Transpose();
        AssertRelease(m == n);
    }
    {
        float4x4 m = MakeTranslationMatrix(float3(1));
        AssertRelease(m.AxisX() == float3::X);
        AssertRelease(m.AxisY() == float3::Y);
        AssertRelease(m.AxisZ() == float3::Z);
        AssertRelease(m.AxisT() == float3::One);
        float4x3 p{
            m.AxisX(),
            m.AxisY(),
            m.AxisZ(),
            m.AxisT()
        };
        AssertRelease(p.AxisX() == float3::X);
        AssertRelease(p.AxisY() == float3::Y);
        AssertRelease(p.AxisZ() == float3::Z);
        AssertRelease(p.AxisT() == float3::One);
        float4x3 q = PackHomogeneousMatrix(m);
        AssertRelease(p == q);
        float4x4 id = float4x4::Identity();
        AssertRelease(id.Column_x() == float4::X);
        AssertRelease(id.Column_y() == float4::Y);
        AssertRelease(id.Column_z() == float4::Z);
        AssertRelease(id.Column_w() == float4::W);
        float4x4 n = m.Multiply(id);
        AssertRelease(NearlyEquals(n.Column_x(), float4::X));
        AssertRelease(NearlyEquals(n.Column_y(), float4::Y));
        AssertRelease(NearlyEquals(n.Column_z(), float4::Z));
        AssertRelease(NearlyEquals(n.Column_w(), float4::One));
    }
}
//----------------------------------------------------------------------------
} //!namedspace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Maths() {
    PPE_DEBUG_NAMEDSCOPE("Test_Maths");

    LOG(Test_Maths, Emphasis, L"starting maths tests ...");

    Test_Iterable_();
    Test_Vector_();
    Test_BoundingBox_();
    Test_Matrix_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
