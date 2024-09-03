// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/ScalarMatrixHelpers.h"

#include "Maths/Plane.h"
#include "Maths/Quaternion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Make2DRotationMatrix
//----------------------------------------------------------------------------
float2x2 Make2DRotationMatrix(float radians) NOEXCEPT {
    const auto [s, c] = SinCos(radians).data;
    return Make2DRotationMatrix(s, c);
}
//----------------------------------------------------------------------------
float2x2 Make2DRotationMatrix(float s, float c) NOEXCEPT {
    return {
        { c,-s},
        { s, c}
    };
}
//----------------------------------------------------------------------------
// Make2DTransformMatrix
//----------------------------------------------------------------------------
float2x3 Make2DTransformMatrix(const float2& translate, float scale, float radians) NOEXCEPT {
    const auto [s, c] = SinCos(radians).data;
    return Make2DTransformMatrix(translate, { scale, scale }, s, c);
}
//----------------------------------------------------------------------------
float2x3 Make2DTransformMatrix(const float2& translate, const float2& scale, float s, float c) NOEXCEPT {
    return {
        { c * scale.x,-s * scale.y, translate.x},
        { s * scale.x, c * scale.y, translate.y},
    };
}
//----------------------------------------------------------------------------
// Make3DTransformMatrix
//----------------------------------------------------------------------------
float4x4 Make3DTransformMatrix(const float3& translate, float scale, const float3& axis, float radians) NOEXCEPT {
    const auto [s, c] = SinCos(radians).data;
    return Make3DTransformMatrix(translate, scale, axis, s, c);
}
//----------------------------------------------------------------------------
float4x4 Make3DTransformMatrix(const float3& translate, float scale, const float3& axis, float fsin, float fcos) NOEXCEPT {
    return Make3DTransformMatrix(translate, float3(scale), Make3DRotationMatrixAroundAxis(axis, fsin, fcos));
}
//----------------------------------------------------------------------------
float4x4 Make3DTransformMatrix(const float3& translate, const float3& scale, const FQuaternion& rotation) NOEXCEPT {
    return Make3DTransformMatrix(translate, float3(scale), Make3DRotationMatrixFromQuaternion(rotation));
}
//----------------------------------------------------------------------------
float4x4 Make3DTransformMatrix(const float3& translate, const float3& scale, const float3x3& rotation) NOEXCEPT {
    Assert_NoAssume(AllGreater(Abs(scale), float3(Epsilon)));
    return {
        (rotation.rows[0] * scale.x).Extend(translate.x),
        (rotation.rows[1] * scale.y).Extend(translate.y),
        (rotation.rows[2] * scale.z).Extend(translate.z),
        float4::Homogeneous
    };
}
//----------------------------------------------------------------------------
float4x4 Make3DTransformMatrix(const float3& translate, const float3& scale, const float4x4& rotation) NOEXCEPT {
    Assert_NoAssume(AllGreater(Abs(scale), float3(Epsilon)));
    Assert_NoAssume(IsHomogeneous(rotation));
    return {
        (rotation.rows[0].xyz * scale.x).Extend(translate.x),
        (rotation.rows[1].xyz * scale.y).Extend(translate.y),
        (rotation.rows[2].xyz * scale.z).Extend(translate.z),
        float4::Homogeneous
    };
}
//----------------------------------------------------------------------------
// Make3DRotationMatrixAroundX
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundX(float radians) NOEXCEPT {
    const auto [s, c] = SinCos(radians).data;
    return Make3DRotationMatrixAroundX(s, c);
}
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundX(float s, float c) NOEXCEPT {
    return {
        { 1, 0, 0},
        { 0, c, s},
        { 0,-s, c},
    };
}
//----------------------------------------------------------------------------
// Make3DRotationMatrixAroundY
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundY(float radians) NOEXCEPT {
    const auto [s, c] = SinCos(radians).data;
    return Make3DRotationMatrixAroundY(s, c);
}
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundY(float s, float c) NOEXCEPT {
    return {
        { c, 0,-s},
        { 0, 1, 0},
        { s, 0, c},
    };
}
//----------------------------------------------------------------------------
// Make3DRotationMatrixAroundZ
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundZ(float radians) NOEXCEPT {
    const auto [s, c] = SinCos(radians).data;
    return Make3DRotationMatrixAroundZ(s, c);
}
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundZ(float s, float c) NOEXCEPT {
    return {
        { c, s, 0},
        {-s, c, 0},
        { 0, 0, 1},
    };
}
//----------------------------------------------------------------------------
// Make3DRotationMatrixAroundAxis
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundAxis(const float3& axis, float radians) NOEXCEPT {
    const auto [s, c] = SinCos(radians).data;
    return Make3DRotationMatrixAroundAxis(axis, s, c);
}
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixAroundAxis(const float3& axis, float s, float c) NOEXCEPT {
    Assert_NoAssume(IsNormalized(axis));
    const float omc = 1 - c;
    return {
        { axis.x*axis.x * omc + c           , axis.x*axis.y * omc - axis.z * s  , axis.x*axis.z * omc + axis.y * s  },
        { axis.y*axis.x * omc + axis.z * s  , axis.y*axis.y * omc + c           , axis.y*axis.z * omc - axis.x * s  },
        { axis.z*axis.x * omc - axis.y * s  , axis.z*axis.y * omc + axis.x * c  , axis.z*axis.z * omc + c           },
    };
}
//----------------------------------------------------------------------------
// Make3DRotationMatrixFromQuaternion
//----------------------------------------------------------------------------
float3x3 Make3DRotationMatrixFromQuaternion(const FQuaternion& q) NOEXCEPT {
    //const auto [r, x, y, z] = q.vec.data; %NOCOMMI%
    const auto [x, y, z, r] = q.vec.data;
    return {
        { 1 - 2 * (y * y + z * z),     2 * (x * y + r * z),     2 * (x * z - r * y) },
        {     2 * (x * y - r * z), 1 - 2 * (x * x + z * z),     2 * (y * z + r * x) },
        {     2 * (x * z + r * y),     2 * (y * z - r * x), 1 - 2 * (x * x + y * y) }
    };
}
//----------------------------------------------------------------------------
// MakeQuaternionFromRotationMatrix
//----------------------------------------------------------------------------
template <u32 _Rows, u32 _Cols>
static FQuaternion MakeQuaternionFromRotationMatrix_(const TScalarMatrix<float, _Rows, _Cols>& m) NOEXCEPT
    requires (_Rows >= 3 and _Cols >= 3) {
    float sqrt, half;
    float scale = m.m[0][0] + m.m[1][1] + m.m[2][2];

    float4 result;

    if (scale > 0.0f) {
        sqrt = Sqrt(scale + 1.0f);
        result.w = sqrt * 0.5f;
        sqrt = 0.5f / sqrt;

        result.x = (m.m[1][2] - m.m[2][1]) * sqrt;
        result.y = (m.m[2][0] - m.m[0][2]) * sqrt;
        result.z = (m.m[0][1] - m.m[1][0]) * sqrt;
    }
    else if ((m.m[0][0] >= m.m[1][1]) && (m.m[0][0] >= m.m[2][2])) {
        sqrt = Sqrt(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]);
        half = 0.5f / sqrt;

        result.x = 0.5f * sqrt;
        result.y = (m.m[0][1] + m.m[1][0]) * half;
        result.z = (m.m[0][2] + m.m[2][0]) * half;
        result.w = (m.m[1][2] - m.m[2][1]) * half;
    }
    else if (m.m[1][1] > m.m[2][2]) {
        sqrt = Sqrt(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]);
        half = 0.5f / sqrt;

        result.y = 0.5f * sqrt;
        result.x = (m.m[1][0] + m.m[0][1]) * half;
        result.z = (m.m[2][1] + m.m[1][2]) * half;
        result.w = (m.m[2][0] - m.m[0][2]) * half;
    }
    else {
        sqrt = Sqrt(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]);
        half = 0.5f / sqrt;

        result.z = 0.5f * sqrt;
        result.x = (m.m[2][0] + m.m[0][2]) * half;
        result.y = (m.m[2][1] + m.m[1][2]) * half;
        result.w = (m.m[0][1] - m.m[1][0]) * half;
    }

    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion MakeQuaternionFromRotationMatrix(const float3x3& m) NOEXCEPT {
    return MakeQuaternionFromRotationMatrix_(m);
}
//----------------------------------------------------------------------------
FQuaternion MakeQuaternionFromRotationMatrix(const float4x4& m) NOEXCEPT {
    return MakeQuaternionFromRotationMatrix_(m);
}
//----------------------------------------------------------------------------
// MakeTranslationMatrix
//---------------------------------------------------------------------------
float4x4 MakeTranslationMatrix(const float3& t) NOEXCEPT {
    const auto [x, y, z] = t.data;
    return {
        { 1, 0, 0, x },
        { 0, 1, 0, y },
        { 0, 0, 1, z },
        { 0, 0, 0, 1 },
    };
}
//----------------------------------------------------------------------------
float4x4 MakeInvertTranslationMatrix(const float3& t) NOEXCEPT {
    const auto [x, y, z] = t.data;
    return {
        { 1, 0, 0,-x },
        { 0, 1, 0,-y },
        { 0, 0, 1,-z },
        { 0, 0, 0, 1 },
    };
}
//----------------------------------------------------------------------------
// MakeScalingMatrix
//----------------------------------------------------------------------------
float4x4 MakeScalingMatrix(const float3& s) NOEXCEPT {
    const auto [x, y, z] = s.data;
    return {
        { x, 0, 0, 0 },
        { 0, y, 0, 0 },
        { 0, 0, z, 0 },
        { 0, 0, 0, 1 },
    };
}
//----------------------------------------------------------------------------
float4x4 MakeInvertScalingMatrix(const float3& s) NOEXCEPT {
    const auto [ox, oy, oz] = float3(Rcp(s)).data;
    return {
        {ox, 0, 0, 0 },
        { 0,oy, 0, 0 },
        { 0, 0,oz, 0 },
        { 0, 0, 0, 1 },
    };
}
//----------------------------------------------------------------------------
// MakeLootAtMatrix
//----------------------------------------------------------------------------
float4x4 MakeLootAtMatrix(const float3& eye, const float3& target, const float3& up) NOEXCEPT {
    Assert_NoAssume(IsNormalized(up));

    const float3 z = SafeNormalize(eye - target);
    const float3 x = SafeNormalize(Cross(up, z));
    const float3 y = Cross(z, x);

    return float4x4{
        x.xyz0,
        y.xyz0,
        z.xyz0,
        float4::W
    } * MakeTranslationMatrix(-eye);
}
//----------------------------------------------------------------------------
float4x4 MakeInvertLookAtMatrix(const float3& eye, const float3& target, const float3& up) NOEXCEPT {
    Assert_NoAssume(IsNormalized(up));

    const float3 z = SafeNormalize(eye - target);
    const float3 x = SafeNormalize(Cross(up, z));
    const float3 y = Cross(z, x);

    return MakeInvertTranslationMatrix(-eye) * float4x4{
        x.xyz0,
        y.xyz0,
        z.xyz0,
        float4::W
    }.transposed;
}
//----------------------------------------------------------------------------
// MakeRightToLeftMatrix
//----------------------------------------------------------------------------
float4x4 MakeRightToLeftMatrix() NOEXCEPT {
    // from right-handed to left-handed coordinates (ex: OGL/VK is RH, whereas DX is LH)
    return {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0,-1, 0 },
        { 0, 0, 0, 1 },
    };
}
//----------------------------------------------------------------------------
float4x4 MakeInvertRightToLeftMatrix() NOEXCEPT {
    // same matrix
    return MakeRightToLeftMatrix();
}
//----------------------------------------------------------------------------
// MakeProjectionMatrix
//----------------------------------------------------------------------------
float4x4 MakeProjectionMatrix(float n, float f) NOEXCEPT {
    Assert_NoAssume(n < f and n > 0);
    return {
        { n, 0, 0, 0 },
        { 0, n, 0, 0 },
        { 0, 0, n+f, -f*n },
        { 0, 0, 1, 0 }
    };
}
//----------------------------------------------------------------------------
float4x4 MakeInvertProjectionMatrix(float n, float f) NOEXCEPT {
    const float oN = Rcp(n);
    const float oFN = Rcp(f*n);
    return {
        { oN, 0, 0, 0 },
        { 0, oN, 0, 0 },
        { 0,  0, 0, 1 },
        { 0,  0,-oFN, (n+f)*oFN }
    };
}
//----------------------------------------------------------------------------
// MakeOrthoProjectionMatrix
//----------------------------------------------------------------------------
float4x4 MakeOrthoProjectionMatrix(float l, float r, float b, float t, float n, float f) NOEXCEPT {
    return (MakeScalingMatrix({ 2/(r-l), 2/(t-b), 2/(f-n) }) *
            MakeTranslationMatrix({ -(l+r)/2, -(t+b)/2, -(f+n)/2 }));
}
//----------------------------------------------------------------------------
float4x4 MakeInvertOrthoProjectionMatrix(float l, float r, float b, float t, float n, float f) NOEXCEPT {
    return (MakeInvertTranslationMatrix({ -(l+r)/2, -(t+b)/2, -(f+n)/2 }) *
            MakeInvertScalingMatrix({ 2/(r-l), 2/(t-b), 2/(f-n) }));
}
//----------------------------------------------------------------------------
// MakePerspectiveProjectionMatrix
//----------------------------------------------------------------------------
float4x4 MakePerspectiveProjectionMatrix(float fov, float aspect, float znear, float zfar) NOEXCEPT {
    const float right = Tan(fov / 2) * znear;
    const float top = (right / aspect);
    return (MakeOrthoProjectionMatrix(-right, right, -top, top, znear, zfar) *
            MakeProjectionMatrix(znear, zfar) *
            MakeRightToLeftMatrix());
}
//----------------------------------------------------------------------------
float4x4 MakeInvertPerspectiveProjectionMatrix(float fov, float aspect, float znear, float zfar) NOEXCEPT {
    const float right = Tan(fov / 2) * znear;
    const float top = (right / aspect);
    return (MakeInvertRightToLeftMatrix() *
            MakeInvertProjectionMatrix(znear, zfar) *
            MakeInvertOrthoProjectionMatrix(-right, right, -top, top, znear, zfar));
}
//----------------------------------------------------------------------------
// MakeBillboardMatrix
//----------------------------------------------------------------------------
float4x4 MakeBillboardMatrix(const float3& obj, const float3& eye, const float3& up, const float3& fwd) NOEXCEPT {
    Assert_NoAssume(IsNormalized(up));
    Assert_NoAssume(IsNormalized(fwd));

    const float3 z = SafeNormalizeOr(obj - eye, -fwd);
    const float3 x = SafeNormalize(Cross(up, z));
    const float3 y = Cross(z, x);

    return float4x4{
        x.xyz0,
        y.xyz0,
        z.xyz0,
        obj.xyz1
    }.transposed;
}
//----------------------------------------------------------------------------
// light : The light direction. If the W component is 0, the light is directional light; if the
// W component is 1, the light is a point light.
float4x4 MakeShadowMatrix(const float4& light, const FPlane& plane) NOEXCEPT {
    const float d = Dot(light, plane.Vec());
    const float x = -plane.Normal().x;
    const float y = -plane.Normal().y;
    const float z = -plane.Normal().z;
    const float w = -plane.D();

    return float4x4{
        { (x + light.x) + d , x * light.y       , x * light.z       , x * light.w       },
        { y * light.x       , (y + light.y) + d , y * light.z       , y * light.w       },
        { z * light.x       , z * light.y       , (z + light.z) + d , z * light.w       },
        { w * light.x       , w * light.y       , w * light.z       , (w + light.w) + d }
    }.transposed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Symmetric matrix packing
//----------------------------------------------------------------------------
float2x3 PackSymmetricMatrix(const float3x3& m) NOEXCEPT {
    Assert_NoAssume(IsSymmetric(m));
    return {
        m.rows[0],
        { m.rows[1].yz, m.rows[2].z }
    };
}
//----------------------------------------------------------------------------
float3x3 UnpackSymmetricMatrix(const float2x3& m) NOEXCEPT {
    return {
        m.rows[0],
        { m.rows[0][1], m.rows[1].xy },
        { m.rows[0][2], m.rows[1][1], m.rows[1][2] }
    };
}
//----------------------------------------------------------------------------
// Homogeneous transform matrix packing
//----------------------------------------------------------------------------
float3x4 PackHomogeneousMatrix(const float4x4& m) NOEXCEPT {
    Assert_NoAssume(IsHomogeneous(m));
    return {
        m.rows[0],
        m.rows[1],
        m.rows[2]
    };
}
//----------------------------------------------------------------------------
float4x4 UnpackHomogeneousMatrix(const float3x4& m) NOEXCEPT {
    return m.Homogeneous();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
