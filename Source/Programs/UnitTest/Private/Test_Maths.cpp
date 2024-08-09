// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.


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

    PPE_LOG_CHECKVOID(Test_Maths, lengthof(values) == checked_cast<u32>(std::distance(range.begin(), range.end())));

    const size_t count_all = range.size();
    const size_t count_odd = range.CountIf(is_odd);
    const size_t count_even = range.CountIf(is_even);
    PPE_LOG_CHECKVOID(Test_Maths, 5 == count_odd);
    PPE_LOG_CHECKVOID(Test_Maths, 4 == count_even);
    PPE_LOG_CHECKVOID(Test_Maths, count_all == count_even + count_odd);

    const size_t count_odd2 = range.FilterBy(is_odd).size();
    const size_t count_even2 = range.FilterBy(is_even).size();
    PPE_LOG_CHECKVOID(Test_Maths, count_odd2 == count_odd);
    PPE_LOG_CHECKVOID(Test_Maths, count_even2 == count_even);

    const int sum_all = range.Accumulate();
    const int sum_odd = range.FilterBy(is_odd).Accumulate();
    const int sum_even = range.FilterBy(is_even).Accumulate();
    PPE_LOG_CHECKVOID(Test_Maths, 25 == sum_odd);
    PPE_LOG_CHECKVOID(Test_Maths, 20 == sum_even);
    PPE_LOG_CHECKVOID(Test_Maths, sum_all == sum_odd + sum_even);

    const int sqr_all = range.MapReduce(sqr, Meta::TPlus<>{});
    const int sqr_odd = range.FilterBy(is_odd).MapReduce(sqr, Meta::TPlus<>{});
    const int sqr_even = range.FilterBy(is_even).MapReduce(sqr, Meta::TPlus<>{});
    PPE_LOG_CHECKVOID(Test_Maths, 285 == sqr_all);
    PPE_LOG_CHECKVOID(Test_Maths, 165 == sqr_odd);
    PPE_LOG_CHECKVOID(Test_Maths, 120 == sqr_even);
    PPE_LOG_CHECKVOID(Test_Maths, sqr_all == sqr_odd + sqr_even);

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
    PPE_LOG_CHECKVOID(Test_Maths, 165 == sqr_odd2);
    PPE_LOG_CHECKVOID(Test_Maths, 120 == sqr_even2);
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
        PPE_LOG_CHECKVOID(Test_Maths, a == float4(1,1,0,0));
        float4 b = Max(z, w);
        PPE_LOG_CHECKVOID(Test_Maths, b == float4(0,0,1,1));
        float4 c = Max(Max(x, y), Max(z, w));
        PPE_LOG_CHECKVOID(Test_Maths, c == float4::One);
        float4 d = Max3(x, y, z);
        PPE_LOG_CHECKVOID(Test_Maths, d == float4(float3::One, 0));
        float3 e = Max3(x.xyz, y.xyz, z.xyz);
        PPE_LOG_CHECKVOID(Test_Maths, e == float3::One);
        float4 f = Max3(Max(x, y), z, w);
        PPE_LOG_CHECKVOID(Test_Maths, f == float4::One);
        float4 m = Max3(x, y, Max(z, w));
        PPE_LOG_CHECKVOID(Test_Maths, m == float4::One);
        ubyte4 n = Float01_to_UByte0255(m);
        PPE_LOG_CHECKVOID(Test_Maths, n == ubyte4(UINT8_MAX));
    }
    {
        int3 x = int3::X;
        int3 y = int3::Y;
        int3 z = int3::Z;
        int3 a = x + y + z;
        PPE_LOG_CHECKVOID(Test_Maths, a == int3::One);
    }
    {
        int3 x = int3::X;
        bool3 m = GreaterMask(x, int3(0));
        PPE_LOG_CHECKVOID(Test_Maths, Any( m));
        PPE_LOG_CHECKVOID(Test_Maths, not All( m));
        int3 select = Blend(int3::MinusOne, int3::One, m);
        int broadcast = select.HSum();
        PPE_LOG_CHECKVOID(Test_Maths, broadcast == 1);
    }
    //{
    //    int4 v;
    //    v.xy = int2{ 1,2 };
    //    PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.y == 2);
    //    const int2 xy{ 3,4 };
    //    auto& zw = v.zw;
    //    zw = xy;
    //    PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.y == 2);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.z == 3);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.w == 4);
    //    v.yw = { 5, 6 };
    //    PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.y == 5);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.z == 3);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.w == 6);
    //}
    //{
    //    int4 v{ int2{ 1,2 }, int2{ 3,4 } };
    //    v.xy = v.xx;
    //    PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.y == 1);
    //    v.zw = v.zz;
    //    //v.zz = { 3,4 }; forbidden, should not compile !
    //    PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.y == 1);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.z == 3);
    //    PPE_LOG_CHECKVOID(Test_Maths, v.w == 3);
    //} %NOCOMMIT%
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_BoundingBox_() {
    {
        FAabb3f box;
        PPE_LOG_CHECKVOID(Test_Maths, not box.HasPositiveExtents());
        PPE_LOG_CHECKVOID(Test_Maths, not box.HasPositiveExtentsStrict());
        box.Add(float3::Zero);
        PPE_LOG_CHECKVOID(Test_Maths, box.HasPositiveExtents());
        PPE_LOG_CHECKVOID(Test_Maths, not box.HasPositiveExtentsStrict());
        box.Add(float3::One);
        PPE_LOG_CHECKVOID(Test_Maths, box.HasPositiveExtentsStrict());
    }
    {
        FAabb3i box;
        PPE_LOG_CHECKVOID(Test_Maths, not box.HasPositiveExtents());
        PPE_LOG_CHECKVOID(Test_Maths, not box.HasPositiveExtentsStrict());
        box.Add(int3::Zero);
        box.Add(int3::X);
        box.Add(int3::Y);
        PPE_LOG_CHECKVOID(Test_Maths, box.HasPositiveExtents());
        PPE_LOG_CHECKVOID(Test_Maths, not box.HasPositiveExtentsStrict());
        box.Add(int3::Z);
        PPE_LOG_CHECKVOID(Test_Maths, box.HasPositiveExtentsStrict());
        FAabb2i box2D = box.Shuffle<0, 1>();
        PPE_LOG_CHECKVOID(Test_Maths, box2D.HasPositiveExtentsStrict());
        int3 ext = box.Extents();
        PPE_LOG_CHECKVOID(Test_Maths, ext == int3::One);
        PPE_LOG_CHECKVOID(Test_Maths, box.Contains(int3::Zero));
        PPE_LOG_CHECKVOID(Test_Maths, box.Contains(int3::One));
        PPE_LOG_CHECKVOID(Test_Maths, not box.Contains(int3::MinusOne));
        PPE_LOG_CHECKVOID(Test_Maths, not box.ContainsStrict(int3::Zero));
        PPE_LOG_CHECKVOID(Test_Maths, not box.ContainsStrict(int3::One));
        PPE_LOG_CHECKVOID(Test_Maths, box.ContainsMaxStrict(int3::Zero));
        PPE_LOG_CHECKVOID(Test_Maths, not box.ContainsMaxStrict(int3::One));
    }
    {
        FAabb3f unit = FAabb3f::MinusOneOneValue();
        FAabb3f boxes[] = {
            unit - float3(1.5f),
            unit + float3(1.5f),
            unit
        };
        PPE_LOG_CHECKVOID(Test_Maths, boxes[2].Intersects(boxes[0]));
        PPE_LOG_CHECKVOID(Test_Maths, boxes[2].Intersects(boxes[1]));
        PPE_LOG_CHECKVOID(Test_Maths, not boxes[0].Intersects(boxes[1]));
        FAabb3f all = MakeBoundingBox(MakeView(boxes));
        PPE_LOG_CHECKVOID(Test_Maths, all.Contains(boxes[0]));
        PPE_LOG_CHECKVOID(Test_Maths, all.Contains(boxes[1]));
        PPE_LOG_CHECKVOID(Test_Maths, all.Contains(boxes[2]));
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Matrix_() {
    {
        int4x3 m = float4x3::Identity();
        int3x4 t = m.Transpose();
        int4x3 n = t.Transpose();
        PPE_LOG_CHECKVOID(Test_Maths, m == n);
    }
    {
        float4x4 m = MakeTranslationMatrix(float3(1));
        PPE_LOG_CHECKVOID(Test_Maths, m.AxisX() == float3::X);
        PPE_LOG_CHECKVOID(Test_Maths, m.AxisY() == float3::Y);
        PPE_LOG_CHECKVOID(Test_Maths, m.AxisZ() == float3::Z);
        PPE_LOG_CHECKVOID(Test_Maths, m.AxisT() == float3::One);
        float4x3 p{
            m.AxisX(),
            m.AxisY(),
            m.AxisZ(),
            m.AxisT()
        };
        PPE_LOG_CHECKVOID(Test_Maths, p.AxisX() == float3::X);
        PPE_LOG_CHECKVOID(Test_Maths, p.AxisY() == float3::Y);
        PPE_LOG_CHECKVOID(Test_Maths, p.AxisZ() == float3::Z);
        PPE_LOG_CHECKVOID(Test_Maths, p.AxisT() == float3::One);
        float4x3 q = PackHomogeneousMatrix(m);
        PPE_LOG_CHECKVOID(Test_Maths, p == q);
        float4x4 id = float4x4::Identity();
        PPE_LOG_CHECKVOID(Test_Maths, id.Column_x() == float4::X);
        PPE_LOG_CHECKVOID(Test_Maths, id.Column_y() == float4::Y);
        PPE_LOG_CHECKVOID(Test_Maths, id.Column_z() == float4::Z);
        PPE_LOG_CHECKVOID(Test_Maths, id.Column_w() == float4::W);
        float4x4 n = m.Multiply(id);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column_x(), float4::X));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column_y(), float4::Y));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column_z(), float4::Z));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column_w(), float4::One));
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
    PPE_LOG_CHECKVOID(Test_Maths, countA == 44);
    PPE_LOG_CHECKVOID(Test_Maths, countA == a.Extent());

    u32 countB = 0;
    for (u32 x : b) {
        countB++;
        Unused(x);
    }
    PPE_LOG_CHECKVOID(Test_Maths, countB == 41);
    PPE_LOG_CHECKVOID(Test_Maths, countB == b.Extent());

    u32 countC = 0;
    for (u32 x : c) {
        countC++;
        Unused(x);
    }
    PPE_LOG_CHECKVOID(Test_Maths, countC == 9);
    PPE_LOG_CHECKVOID(Test_Maths, countC == c.Extent());

    const u32 sumA = std::accumulate(a.begin(), a.end(), 0);
    PPE_LOG_CHECKVOID(Test_Maths, sumA == 1122);
    const u32 sumB = std::accumulate(b.begin(), b.end(), 0);
    PPE_LOG_CHECKVOID(Test_Maths, sumB == 2132);
    const u32 sumC = std::accumulate(c.begin(), c.end(), 0);
    PPE_LOG_CHECKVOID(Test_Maths, sumC == 189);

    STATIC_ASSERT(a.Contains(a));
    STATIC_ASSERT(not a.Contains(b));
    STATIC_ASSERT(a.Contains(c));
    STATIC_ASSERT(a.Overlaps(b));
    STATIC_ASSERT(a.Overlaps(c));

    PPE_LOG_CHECKVOID(Test_Maths, not b.Contains(a));
    PPE_LOG_CHECKVOID(Test_Maths, not b.Contains(c));
    PPE_LOG_CHECKVOID(Test_Maths, b.Overlaps(a));
    PPE_LOG_CHECKVOID(Test_Maths, not b.Overlaps(c));

    PPE_LOG_CHECKVOID(Test_Maths, not c.Contains(a));
    PPE_LOG_CHECKVOID(Test_Maths, not c.Contains(b));
    PPE_LOG_CHECKVOID(Test_Maths, c.Overlaps(a));
    PPE_LOG_CHECKVOID(Test_Maths, not c.Overlaps(b));

    const FRange32u intersectAB = a.Intersect(b);
    PPE_LOG_CHECKVOID(Test_Maths, intersectAB == FRange32u(32, 48));
    PPE_LOG_CHECKVOID(Test_Maths, a.Contains(intersectAB));
    PPE_LOG_CHECKVOID(Test_Maths, b.Contains(intersectAB));
    PPE_LOG_CHECKVOID(Test_Maths, not c.Contains(intersectAB));
    PPE_LOG_CHECKVOID(Test_Maths, a.Overlaps(intersectAB));
    PPE_LOG_CHECKVOID(Test_Maths, b.Overlaps(intersectAB));
    PPE_LOG_CHECKVOID(Test_Maths, not c.Overlaps(intersectAB));

    const FRange32u intersectBC = b.Intersect(c);
    PPE_LOG_CHECKVOID(Test_Maths, intersectBC.Empty());
    PPE_LOG_CHECKVOID(Test_Maths, a.Contains(intersectBC));
    PPE_LOG_CHECKVOID(Test_Maths, b.Contains(intersectBC));
    PPE_LOG_CHECKVOID(Test_Maths, c.Contains(intersectBC));
    PPE_LOG_CHECKVOID(Test_Maths, not a.Overlaps(intersectBC));
    PPE_LOG_CHECKVOID(Test_Maths, not b.Overlaps(intersectBC));
    PPE_LOG_CHECKVOID(Test_Maths, not c.Overlaps(intersectBC));

    CONSTEXPR FRange32u intersectAC = a.Intersect(c);
    STATIC_ASSERT(intersectAC == c);
    STATIC_ASSERT(a.Contains(intersectAC));
    STATIC_ASSERT(not b.Contains(intersectAC));
    STATIC_ASSERT(c.Contains(intersectAC));
    STATIC_ASSERT(a.Overlaps(intersectAC));
    STATIC_ASSERT(not b.Overlaps(intersectAC));
    STATIC_ASSERT(c.Overlaps(intersectAC));

    const FRange32u unionAB = a.Union(b);
    PPE_LOG_CHECKVOID(Test_Maths, unionAB == FRange32u(4, 73));
    PPE_LOG_CHECKVOID(Test_Maths, unionAB.Contains(a));
    PPE_LOG_CHECKVOID(Test_Maths, unionAB.Contains(b));
    PPE_LOG_CHECKVOID(Test_Maths, unionAB.Contains(c));
    PPE_LOG_CHECKVOID(Test_Maths, a.Overlaps(unionAB));
    PPE_LOG_CHECKVOID(Test_Maths, b.Overlaps(unionAB));
    PPE_LOG_CHECKVOID(Test_Maths, c.Overlaps(unionAB));

#if 0 // union of non overlapping ranges is not allowed
    const FRange32u unionBC = b.Union(c);
    PPE_LOG_CHECKVOID(Test_Maths, unionBC == FRange32u(17, 73));
    PPE_LOG_CHECKVOID(Test_Maths, not unionBC.Contains(a));
    PPE_LOG_CHECKVOID(Test_Maths, unionBC.Contains(b));
    PPE_LOG_CHECKVOID(Test_Maths, unionBC.Contains(c));
    PPE_LOG_CHECKVOID(Test_Maths, a.Overlaps(unionBC));
    PPE_LOG_CHECKVOID(Test_Maths, b.Overlaps(unionBC));
    PPE_LOG_CHECKVOID(Test_Maths, c.Overlaps(unionBC));
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

    PPE_LOG(Test_Maths, Emphasis, "starting maths tests ...");

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
