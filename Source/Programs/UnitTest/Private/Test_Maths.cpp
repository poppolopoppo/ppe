// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.


#include "Maths/Plane.h"
#include "Maths/Quaternion.h"
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
        a += a;
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
    {
       int4 v;
       v.xy = int2{ 1,2 };
       PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
       PPE_LOG_CHECKVOID(Test_Maths, v.y == 2);
       const int2 xy{ 3,4 };
       auto& zw = v.zw;
       zw = xy;
       PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
       PPE_LOG_CHECKVOID(Test_Maths, v.y == 2);
       PPE_LOG_CHECKVOID(Test_Maths, v.z == 3);
       PPE_LOG_CHECKVOID(Test_Maths, v.w == 4);
       v.yw = int2{ 5, 6 };
       PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
       PPE_LOG_CHECKVOID(Test_Maths, v.y == 5);
       PPE_LOG_CHECKVOID(Test_Maths, v.z == 3);
       PPE_LOG_CHECKVOID(Test_Maths, v.w == 6);
    }
    {
       int4 v{ int2{ 1,2 }, int2{ 3,4 } };
       v.xy = v.xx;
       PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
       PPE_LOG_CHECKVOID(Test_Maths, v.y == 1);
       v.zw = v.zz;
       //v.zz = { 3,4 }; forbidden, should not compile !
       PPE_LOG_CHECKVOID(Test_Maths, v.x == 1);
       PPE_LOG_CHECKVOID(Test_Maths, v.y == 1);
       PPE_LOG_CHECKVOID(Test_Maths, v.z == 3);
       PPE_LOG_CHECKVOID(Test_Maths, v.w == 3);
    }
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
        float3x3 m1{ float3::X, float3::Y, float3::Z };
        PPE_LOG_CHECKVOID(Test_Maths, m1.Diagonal() == float3::One);
        float3x3 m2 = float3x3::Identity;
        PPE_LOG_CHECKVOID(Test_Maths, m1 == m2);
        float3x3 m3 = float3(1,2,3);
        PPE_LOG_CHECKVOID(Test_Maths, m3.Diagonal() == float3(1,2,3));
        float3x3 m4 = m3.transposed;
        PPE_LOG_CHECKVOID(Test_Maths, m4.Diagonal() == m3.Diagonal());
    }
    {
        int4x3 m = int4x3::Identity;
        int3x4 t = m.transposed;
        int4x3 n = t.transposed;
        PPE_LOG_CHECKVOID(Test_Maths, m == n);
        int4 c0 = n.Column<0>();
        int3 c1 = t.Column<0>();
        PPE_LOG_CHECKVOID(Test_Maths, c0.xyz == c1);
    }
    {
        float4x4 m = MakeTranslationMatrix(float3(1));
        PPE_LOG_CHECKVOID(Test_Maths, m.Column<0>() == float3::X.Extend(0));
        PPE_LOG_CHECKVOID(Test_Maths, m.Column<1>() == float3::Y.Extend(0));
        PPE_LOG_CHECKVOID(Test_Maths, m.Column<2>() == float3::Z.Extend(0));
        PPE_LOG_CHECKVOID(Test_Maths, m.Column<3>() == float4::One);
        float3x4 p{
            m.rows[0],
            m.rows[1],
            m.rows[2],
        };
        PPE_LOG_CHECKVOID(Test_Maths, p.rows[0].xyz == float3::X);
        PPE_LOG_CHECKVOID(Test_Maths, p.rows[1].xyz == float3::Y);
        PPE_LOG_CHECKVOID(Test_Maths, p.rows[2].xyz == float3::Z);
        float3x4 q = PackHomogeneousMatrix(m);
        float4x4 r = UnpackHomogeneousMatrix(q);
        PPE_LOG_CHECKVOID(Test_Maths, p == q);
        PPE_LOG_CHECKVOID(Test_Maths, r == m);
        float4x4 id = float4x4::Identity;
        PPE_LOG_CHECKVOID(Test_Maths, id.rows[0] == float4::X);
        PPE_LOG_CHECKVOID(Test_Maths, id.rows[1] == float4::Y);
        PPE_LOG_CHECKVOID(Test_Maths, id.rows[2] == float4::Z);
        PPE_LOG_CHECKVOID(Test_Maths, id.rows[3] == float4::W);
        float4x4 n = m.Multiply(id);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column<0>(), float4::X));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column<1>(), float4::Y));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column<2>(), float4::Z));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(n.Column<3>(), float4::One));
    }
    {
        float3x3 m{
            {1, 2, 3},
            {2, 7, 6},
            {3, 6, 9}
        };
        PPE_LOG_CHECKVOID(Test_Maths, IsSymmetric(m));
        float2x3 p = PackSymmetricMatrix(m);
        float3x3 r = UnpackSymmetricMatrix(p);
        PPE_LOG_CHECKVOID(Test_Maths, r == m);
    }
    {
        float3x3 m = float3x3::Identity;
        float3 v = {1,2,3};
        float3 r0 = m * v;
        float3 r1 = v * m;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, v));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r1, v));
    }
    {
        float4x4 m = float4x4::Identity;
        float4x4 n = float4x4::Identity;
        float4x4 r = m * n;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r, m));
        PPE_LOG_CHECKVOID(Test_Maths, IsHomogeneous(r));
    }
    {
        float3x3 m{ {1.f,2.f,3.f},
                    {4.f,5.f,6.f},
                    {7.f,8.f,9.f} };
        float3x3 p{ float3(1.f,2.f,3.f),
                    float3(4.f,5.f,6.f),
                    float3(7.f,8.f,9.f) };
        float3x3 q{ 1.f,2.f,3.f,
                    4.f,5.f,6.f,
                    7.f,8.f,9.f };
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(m, p));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(p, q));
    }
    {
        float3x3 m( {1,2,3},
                    {4,5,6},
                    {7,8,9} );
        float3x3 n{ {9,8,7},
                    {6,5,4},
                    {3,2,1} };
        float3x3 r0 = m * n;
        float3x3 r1 = n * m;
        float3x3 e0{{30, 24, 18},
                    {84, 69, 54},
                    {138, 114, 90} };
        float3x3 e1{{90, 114, 138},
                    {54, 69, 84},
                    {18, 24, 30}} ;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, e0));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r1, e1));
    }
    {
        float4x3 m = float4x3::Identity;
        float3x4 n = float3x4::Identity;
        float4x3 t = n.transposed;
        float4x4 r0 = m * n;
        float3x3 r1 = n * m;
        float4 v0 = m.Multiply(float3::One);
        float3 v1 = n.Multiply(float4::One);
        float4x4 h0 = m.OneExtend();
        float4x4 h1 = n.OneExtend();
        float4x4 h2 = r1.Homogeneous();
    }
    {
        float3x3 a = Make3DRotationMatrixAroundX(.33f);
        float3x3 t0 = a.transposed;
        float3x3 t1 = a.Transpose();
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(t0, t1));
        float3x3 b = t0.transposed;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(a, b));
    }
    {
        float4x4 a = Make3DTransformMatrix(float3(1,2,3), float3(2,3,4), Make3DRotationMatrixAroundX(.33f));
        float3x4 b = PackHomogeneousMatrix(a);
        float4x3 t0 = b.transposed;
        float4x3 t1 = b.Transpose();
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(t0, t1));
        float3x4 c = t0.transposed;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(b, c));
    }
    {
        float3x3 a = Make3DRotationMatrixAroundX(.33f);
        float3x3 b = Make3DRotationMatrixAroundY(.22f);
        float3x3 c = Make3DRotationMatrixAroundZ(.44f);
        float3x3 r0 = a * (b + c);
        float3x3 r1 = a * b + a * c;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, r1));
    }
    {
        float3x3 a = Make3DRotationMatrixAroundX(.33f);
        float3x3 b = Make3DRotationMatrixAroundY(.22f);
        float3x3 c = Make3DRotationMatrixAroundZ(.44f);
        float3x3 r0 = (b + c) * a;
        float3x3 r1 = b * a + c * a;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, r1));
    }
    {
        float3x3 a = Make3DRotationMatrixAroundX(.33f);
        float3x3 b = Make3DRotationMatrixAroundY(.22f);
        float3x3 r0 = (a * b).transposed;
        float3x3 r1 = b.transposed * a.transposed;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, r1));
    }
    {
        float4x4 a = Make3DTransformMatrix(float3(1,2,3), float3(2,3,4), Make3DRotationMatrixAroundX(.33f));
        float4x4 i = Invert(a);
        float4x4 r0 = a * i;
        float4x4 r1 = i * a;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, float4x4::Identity));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r1, float4x4::Identity));
    }
    {
        float3x3 a = Make3DRotationMatrixAroundX(.33f);
        float3x3 b = Make3DRotationMatrixAroundY(.22f);
        float3x3 r0 = Invert(a * b);
        float3x3 r1 = Invert(b) * Invert(a);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, r1));
    }
    {
        float4x4 a = Make3DTransformMatrix(float3(1,2,3), float3(2,3,4), Make3DRotationMatrixAroundX(.33f));
        float4x4 r0 = Invert(a.Transpose());
        float4x4 r1 = Invert(a).transposed;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, r1));
    }
    {
        float4x4 a = Make3DTransformMatrix(float3(1,2,3), float3(2,3,4), Make3DRotationMatrixAroundX(.33f));
        float4x4 r0 = (a * 3).Transpose();
        float4x4 r1 = a.transposed * 3;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r0, r1));
    }
    {
        float4x4 a = Make3DTransformMatrix(float3(1,2,3), float3(2,3,4), Make3DRotationMatrixAroundX(.33f));
        float4x4 i = Invert(a);
        float4x4 r = Invert(i);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r, a));
    }
    {
        float4x4 a{ {1,0,0,1},
                    {0,1,0,2},
                    {0,0,1,3},
                    {0,0,0,1}};
        float4x4 i = Invert(a);
        float4x4 n{ {1,0,0,-1},
                    {0,1,0,-2},
                    {0,0,1,-3},
                    {0,0,0,1}};
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(i, n));
        float4 p{1,2,3,1};
        float4 r = a * p;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r, float4(2,4,6,1)));
        float4 o = i * r;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(o, p));
    }
    {
        float4x4 a{ {1,0,0,1},
                    {0,1,0,2},
                    {0,0,1,3},
                    {0,0,0,1}};
        float4x4 i = Invert_AssumeHomogeneous(a);
        float4x4 n{ {1,0,0,-1},
                    {0,1,0,-2},
                    {0,0,1,-3},
                    {0,0,0,1}};
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(i, n));
        float4 p{1,2,3,1};
        float4 r = a * p;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r, float4(2,4,6,1)));
        float4 o = i * r;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(o, p));
    }
    {
        float3 o = {1,2,3};
        float4x4 t = MakeTranslationMatrix(o);
        float3 p = t.Multiply_OneExtend(float3::Zero).xyz;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(p, o));
    }
    {
        float3 s = {1,2,3};
        float4x4 t = MakeScalingMatrix(s);
        float3 p = t.Multiply_OneExtend(float3::One).xyz;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(p, s));
    }
    {
        float3 o = {1,2,3};
        float3 s = {1,2,3};
        float4x4 t = MakeScalingMatrix(s) * MakeTranslationMatrix(o);
        float3 p = t.Multiply_OneExtend(float3::One).xyz;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(p, float3{2,6,12}));
    }
    {
        float3 o = {1,2,3};
        float3 s = {1,2,3};
        float4x4 t = MakeTranslationMatrix(o) * MakeScalingMatrix(s);
        float3 p = t.Multiply_OneExtend(float3::One).xyz;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(p, float3{2,4,6}));
    }
    {
        FQuaternion q = FQuaternion::Identity;
        PPE_LOG_CHECKVOID(Test_Maths, q.IsIdentity());
        float3x3 r = Make3DRotationMatrixFromQuaternion(q);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r, float3x3::Identity));
    }
    {
        float3x3 m = float3x3::Identity;
        FQuaternion q = MakeQuaternionFromRotationMatrix(m);
        PPE_LOG_CHECKVOID(Test_Maths, q.IsIdentity());
        float3x3 r = Make3DRotationMatrixFromQuaternion(q);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r, float3x3::Identity));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(m, r));
    }
    {
        float3 u = float3::Y;
        float3x3 t = Make3DRotationMatrixAroundX(PIf);
        float3 v = t.Multiply(u);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(v, -u));
        FQuaternion q = MakeQuaternionFromRotationMatrix(t);
        float3x3 m = Make3DRotationMatrixFromQuaternion(q);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(t, m));
        float3 w = q.Transform(u);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(v, w));
    }
    {
        float3 u = float3::X;
        float3x3 t = Make3DRotationMatrixAroundY(PIf);
        float3 v = t.Multiply(u);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(v, -u));
        FQuaternion q = MakeQuaternionFromRotationMatrix(t);
        float3x3 m = Make3DRotationMatrixFromQuaternion(q);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(t, m));
        float3 w = q.Transform(u);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(v, w));
    }
    {
        float3 u = float3::X;
        float3x3 t = Make3DRotationMatrixAroundZ(PIf);
        float3 v = t.Multiply(u);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(v, -u));
        FQuaternion q = MakeQuaternionFromRotationMatrix(t);
        float3x3 m = Make3DRotationMatrixFromQuaternion(q);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(t, m));
        float3 w = q.Transform(u);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(v, w));
    }
    {
        float3x3 mrot = Make3DRotationMatrixAroundX(.33f);
        FQuaternion qrot = MakeQuaternionFromRotationMatrix(mrot);
        float3x3 test = Make3DRotationMatrixFromQuaternion(qrot);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(mrot, test));
    }
    {
        float3x3 mrot = Make3DRotationMatrixAroundY(.33f);
        FQuaternion qrot = MakeQuaternionFromRotationMatrix(mrot);
        float3x3 test = Make3DRotationMatrixFromQuaternion(qrot);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(mrot, test));
    }
    {
        float3x3 mrot = Make3DRotationMatrixAroundZ(.33f);
        FQuaternion qrot = MakeQuaternionFromRotationMatrix(mrot);
        float3x3 test = Make3DRotationMatrixFromQuaternion(qrot);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(mrot, test));
    }
    {
        float3x3 mrot = Make3DRotationMatrixAroundX(.33f) *
                        Make3DRotationMatrixAroundY(.22f) *
                        Make3DRotationMatrixAroundZ(.44f);
        FQuaternion qrot = MakeQuaternionFromRotationMatrix(mrot);
        float3x3 test = Make3DRotationMatrixFromQuaternion(qrot);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(mrot, test));
        float3 translate{ 1, 2, 3 };
        float3 scale{ 2, 3, 4 };
        float4x4 transform = Make3DTransformMatrix(translate, scale, qrot);
        float3 t, s;
        FQuaternion q;
        Decompose(transform, s, q, t);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(t, translate));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(s, scale));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(q.vec, qrot.vec));
    }
    {
        float4x4 a = Make3DTransformMatrix(float3(1,2,3), float3(2,3,4), Make3DRotationMatrixAroundX(.33f));
        float4x4 i = Invert_AssumeHomogeneous(a);
        float4x4 n = Invert(a);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(i, n));
        float4x4 r = Invert_AssumeHomogeneous(i);
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(r, a));
    }
    {
        const float left = -7.f;
        const float right = 9.f;
        const float top = 11.f;
        const float bottom = -13.f;
        const float znear = -3.f;
        const float zfar = 31.f;
        float4x4 projection = MakeOrthoProjectionMatrix(left, right, bottom, top, znear, zfar);
        {
            const float3 viewPos{left,bottom,znear};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{-1,-1,-1}));
        }
        {
            const float3 viewPos{right,bottom,znear};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{1,-1,-1}));
        }
        {
            const float3 viewPos{left,top,znear};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{-1,1,-1}));
        }
        {
            const float3 viewPos{right,top,znear};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{1,1,-1}));
        }
        {
            const float3 viewPos{left,bottom,zfar};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{-1,-1,1}));
        }
        {
            const float3 viewPos{right,bottom,zfar};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{1,-1,1}));
        }
        {
            const float3 viewPos{left,top,zfar};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{-1,1,1}));
        }
        {
            const float3 viewPos{right,top,zfar};
            float4 clipPos = projection * viewPos.Extend(1);
            clipPos.xyz /= clipPos.w;
            PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(clipPos.xyz, float3{1,1,1}));
        }
    }
    {
        float4 viewPos{ float3::Z, 1 };
        float4x4 projection = MakePerspectiveProjectionMatrix(Radians(75.f), 1.f, Epsilon_v<float>, 2.f);
        float4 screenPos = projection * viewPos;
        screenPos.xyz /= screenPos.w;
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(screenPos.xy, float2::Zero));
        PPE_LOG_CHECKVOID(Test_Maths, NearlyEquals(screenPos.z, 0.f));
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
