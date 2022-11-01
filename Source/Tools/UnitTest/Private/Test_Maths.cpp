#include "stdafx.h"


#include "Maths/Range.h"
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
        CONSTEXPR int2 x{1, 0};
        CONSTEXPR int2 y{0, 1};
        CONSTEXPR int2 ad = ((x + y) * (x - y) * 10) / 5;
        STATIC_ASSERT(ad == int2(2,-2));
        CONSTEXPR int2 mb = Blend(x, y, GreaterMask(x, y));
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
        AssertRelease(a == int3::One);
    }
    {
        int3 x = int3::X;
        bool3 m = GreaterMask(x, int3(0));
        AssertRelease(Any( m));
        AssertRelease(not All( m));
        int3 select = Blend(int3::MinusOne, int3::One, m);
        int broadcast = select.HSum();
        AssertRelease(broadcast == 1);
    }
    {
        int4 v;
        v.xy = int2{ 1,2 };
        AssertRelease(v.x == 1);
        AssertRelease(v.y == 2);
        const int2 xy{ 3,4 };
        auto& zw = v.zw;
        zw = xy;
        AssertRelease(v.x == 1);
        AssertRelease(v.y == 2);
        AssertRelease(v.z == 3);
        AssertRelease(v.w == 4);
        v.yw = { 5, 6 };
        AssertRelease(v.x == 1);
        AssertRelease(v.y == 5);
        AssertRelease(v.z == 3);
        AssertRelease(v.w == 6);
    }
    {
        int4 v{ int2{ 1,2 }, int2{ 3,4 } };
        v.xy = v.xx;
        AssertRelease(v.x == 1);
        AssertRelease(v.y == 1);
        v.zw = v.zz;
        //v.zz = { 3,4 }; forbidden, should not compile !
        AssertRelease(v.x == 1);
        AssertRelease(v.y == 1);
        AssertRelease(v.z == 3);
        AssertRelease(v.w == 3);
    }
y}
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
        FAabb2i box2D = box.Shuffle<0, 1>();
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
static NO_INLINE void Test_Range_() {
    CONSTEXPR FRange32u a{ 4,48 };
    CONSTEXPR FRange32u b{ 32,73 };
    CONSTEXPR FRange32u c{ 17,26 };

    u32 countA = 0;
    for (u32 x : a) {
        countA++;
        Unused(x);
    }
    AssertRelease(countA == 44);
    AssertRelease(countA == a.Extent());

    u32 countB = 0;
    for (u32 x : b) {
        countB++;
        Unused(x);
    }
    AssertRelease(countB == 41);
    AssertRelease(countB == b.Extent());

    u32 countC = 0;
    for (u32 x : c) {
        countC++;
        Unused(x);
    }
    AssertRelease(countC == 9);
    AssertRelease(countC == c.Extent());

    const u32 sumA = std::accumulate(a.begin(), a.end(), 0);
    AssertRelease(sumA == 1122);
    const u32 sumB = std::accumulate(b.begin(), b.end(), 0);
    AssertRelease(sumB == 2132);
    const u32 sumC = std::accumulate(c.begin(), c.end(), 0);
    AssertRelease(sumC == 189);

    STATIC_ASSERT(a.Contains(a));
    STATIC_ASSERT(not a.Contains(b));
    STATIC_ASSERT(a.Contains(c));
    STATIC_ASSERT(a.Overlaps(b));
    STATIC_ASSERT(a.Overlaps(c));

    AssertRelease(not b.Contains(a));
    AssertRelease(not b.Contains(c));
    AssertRelease(b.Overlaps(a));
    AssertRelease(not b.Overlaps(c));

    AssertRelease(not c.Contains(a));
    AssertRelease(not c.Contains(b));
    AssertRelease(c.Overlaps(a));
    AssertRelease(not c.Overlaps(b));

    const FRange32u intersectAB = a.Intersect(b);
    AssertRelease(intersectAB == FRange32u(32, 48));
    AssertRelease(a.Contains(intersectAB));
    AssertRelease(b.Contains(intersectAB));
    AssertRelease(not c.Contains(intersectAB));
    AssertRelease(a.Overlaps(intersectAB));
    AssertRelease(b.Overlaps(intersectAB));
    AssertRelease(not c.Overlaps(intersectAB));

    const FRange32u intersectBC = b.Intersect(c);
    AssertRelease(intersectBC.Empty());
    AssertRelease(a.Contains(intersectBC));
    AssertRelease(b.Contains(intersectBC));
    AssertRelease(c.Contains(intersectBC));
    AssertRelease(not a.Overlaps(intersectBC));
    AssertRelease(not b.Overlaps(intersectBC));
    AssertRelease(not c.Overlaps(intersectBC));

    CONSTEXPR FRange32u intersectAC = a.Intersect(c);
    STATIC_ASSERT(intersectAC == c);
    STATIC_ASSERT(a.Contains(intersectAC));
    STATIC_ASSERT(not b.Contains(intersectAC));
    STATIC_ASSERT(c.Contains(intersectAC));
    STATIC_ASSERT(a.Overlaps(intersectAC));
    STATIC_ASSERT(not b.Overlaps(intersectAC));
    STATIC_ASSERT(c.Overlaps(intersectAC));

    const FRange32u unionAB = a.Union(b);
    AssertRelease(unionAB == FRange32u(4, 73));
    AssertRelease(unionAB.Contains(a));
    AssertRelease(unionAB.Contains(b));
    AssertRelease(unionAB.Contains(c));
    AssertRelease(a.Overlaps(unionAB));
    AssertRelease(b.Overlaps(unionAB));
    AssertRelease(c.Overlaps(unionAB));

#if 0 // union of non overlapping ranges is not allowed
    const FRange32u unionBC = b.Union(c);
    AssertRelease(unionBC == FRange32u(17, 73));
    AssertRelease(not unionBC.Contains(a));
    AssertRelease(unionBC.Contains(b));
    AssertRelease(unionBC.Contains(c));
    AssertRelease(a.Overlaps(unionBC));
    AssertRelease(b.Overlaps(unionBC));
    AssertRelease(c.Overlaps(unionBC));
#endif

    CONSTEXPR FRange32u unionAC = a.Union(c);
    STATIC_ASSERT(unionAC == a);
    STATIC_ASSERT(unionAC.Contains(a));
    STATIC_ASSERT(not unionAC.Contains(b));
    STATIC_ASSERT(unionAC.Contains(c));
    STATIC_ASSERT(a.Overlaps(unionAC));
    STATIC_ASSERT(b.Overlaps(unionAC));
    STATIC_ASSERT(c.Overlaps(unionAC));
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
    Test_Range_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
