#pragma once

#include "Core_fwd.h"

#include "Maths/MathHelpers.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

#include "HAL/PlatformEndian.h"

namespace PPE {
class FPlane;
class FQuaternion;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Unary functions
//----------------------------------------------------------------------------
PPE_SCALARMATRIX_UNARYOP_FUNC(Abs)
PPE_SCALARMATRIX_UNARYOP_FUNC(Frac)
PPE_SCALARMATRIX_UNARYOP_FUNC(Fractional)
PPE_SCALARMATRIX_UNARYOP_FUNC(Saturate)
PPE_SCALARMATRIX_UNARYOP_FUNC(Sign)
PPE_SCALARMATRIX_UNARYOP_FUNC(SignNotZero)
PPE_SCALARMATRIX_UNARYOP_FUNC(Rcp)
PPE_SCALARMATRIX_UNARYOP_FUNC(RSqrt)
PPE_SCALARMATRIX_UNARYOP_FUNC(RSqrt_Low)
PPE_SCALARMATRIX_UNARYOP_FUNC(Sqr)
PPE_SCALARMATRIX_UNARYOP_FUNC(Sqrt)
//----------------------------------------------------------------------------
// Binary functions
//----------------------------------------------------------------------------
PPE_SCALARMATRIX_BINARYOP_FUNC(FMod)
PPE_SCALARMATRIX_BINARYOP_FUNC(GridSnap)
PPE_SCALARMATRIX_BINARYOP_FUNC(Hypot)
PPE_SCALARMATRIX_BINARYOP_FUNC(IntDivCeil)
PPE_SCALARMATRIX_BINARYOP_FUNC(IntDivFloor)
PPE_SCALARMATRIX_BINARYOP_FUNC(IntDivRound)
PPE_SCALARMATRIX_BINARYOP_FUNC(Min)
PPE_SCALARMATRIX_BINARYOP_FUNC(Max)
PPE_SCALARMATRIX_BINARYOP_FUNC(Pow)
PPE_SCALARMATRIX_BINARYOP_FUNC(Step)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD bool IsHomogeneous(const TScalarMatrix<T, _Rows, _Cols>& m, float epsilon = Epsilon);
//----------------------------------------------------------------------------
template <typename T, u32 N>
NODISCARD bool IsIdentity(const TScalarMatrix<T, N, N>& m, float epsilon = Epsilon);
//----------------------------------------------------------------------------
template <typename T, u32 N>
NODISCARD bool IsInversible(const TScalarMatrix<T, N, N>& m, float epsilon = Epsilon);
//----------------------------------------------------------------------------
template <typename T, u32 N>
NODISCARD bool IsOrthogonal(const TScalarMatrix<T, N, N>& m, float epsilon = Epsilon);
//----------------------------------------------------------------------------
template <typename T, u32 N>
NODISCARD bool IsOrthonormal(const TScalarMatrix<T, N, N>& m, float epsilon = Epsilon);
//----------------------------------------------------------------------------
template <typename T, u32 N>
NODISCARD bool IsSymmetric(const TScalarMatrix<T, N, N>& m, float epsilon = Epsilon);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD bool IsINF(const TScalarMatrix<T, _Rows, _Cols>& m);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD bool IsNAN(const TScalarMatrix<T, _Rows, _Cols>& m);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD bool IsNANorINF(const TScalarMatrix<T, _Rows, _Cols>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs>
NODISCARD bool NearlyEquals(
    const details::TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs,
    const details::TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>& rhs,
    float maxRelDiff = Epsilon);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs>
NODISCARD
TScalarMatrix<T, _Rows, _Cols> OuterProduct(const details::TScalarVectorExpr<T, _Rows, _Lhs>& u,
                                            const details::TScalarVectorExpr<T, _Cols, _Rhs>& v);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD
void Lerp(  const TScalarMatrix<T, _Rows, _Cols>& v0,
            const TScalarMatrix<T, _Rows, _Cols>& v1,
            float f,
            TScalarMatrix<T, _Rows, _Cols>& result);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD
TScalarMatrix<T, _Rows, _Cols> Lerp( const TScalarMatrix<T, _Rows, _Cols>& v0,
                                        const TScalarMatrix<T, _Rows, _Cols>& v1,
                                        float f );
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD
void SLerp( const TScalarMatrix<T, _Rows, _Cols>& v0,
            const TScalarMatrix<T, _Rows, _Cols>& v1,
            float f,
            TScalarMatrix<T, _Rows, _Cols>& result);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD
void Smoothstep(const TScalarMatrix<T, _Rows, _Cols>& v0,
                const TScalarMatrix<T, _Rows, _Cols>& v1,
                float f,
                TScalarMatrix<T, _Rows, _Cols>& result);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD
void Smootherstep(  const TScalarMatrix<T, _Rows, _Cols>& v0,
                    const TScalarMatrix<T, _Rows, _Cols>& v1,
                    float f,
                    TScalarMatrix<T, _Rows, _Cols>& result);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Orthogonalize(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD TScalarMatrix<T, 4, 4> Orthonormalize(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void DecomposeQR(const TScalarMatrix<T, 4, 4>& m, TScalarMatrix<T, 4, 4>& q, TScalarMatrix<T, 4, 4>& r);
//----------------------------------------------------------------------------
template <typename T>
void DecomposeLQ(const TScalarMatrix<T, 4, 4>& m, TScalarMatrix<T, 4, 4>& l, TScalarMatrix<T, 4, 4>& q);
//----------------------------------------------------------------------------
template <typename T>
void Decompose( const TScalarMatrix<T, 4, 4>& transform,
                TScalarVector<T, 3>& scale,
                FQuaternion& rotation,
                TScalarVector<T, 3>& translation );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det(const TScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det(const TScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det2x2(const TScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det2x2(const TScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det2x2(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det3x3(const TScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD T Det3x3(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 2, 2> Invert(const TScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Invert(const TScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Invert(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Invert_AssumeHomogeneous(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
NODISCARD TScalarMatrix<T, _Cols, _Rows> InvertTranspose(const TScalarMatrix<T, _Rows, _Cols>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float2x2 Make2DRotationMatrix(float fsin, float fcos) NOEXCEPT;
PPE_CORE_API NODISCARD float2x2 Make2DRotationMatrix(float radians) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float2x3 Make2DTransformMatrix(const float2& translate, float scale, float radians) NOEXCEPT;
PPE_CORE_API NODISCARD float2x3 Make2DTransformMatrix(const float2& translate, const float2& scale, float fsin, float fcos) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 Make3DTransformMatrix(const float3& translate, float scale, const float3& axis, float radians) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 Make3DTransformMatrix(const float3& translate, float scale, const float3& axis, float fsin, float fcos) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 Make3DTransformMatrix(const float3& translate, const float3& scale, const FQuaternion& rotation) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 Make3DTransformMatrix(const float3& translate, const float3& scale, const float3x3& rotation) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 Make3DTransformMatrix(const float3& translate, const float3& scale, const float4x4& rotation) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundX(float radians) NOEXCEPT;
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundX(float fsin, float fcos) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundY(float radians) NOEXCEPT;
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundY(float fsin, float fcos) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundZ(float radians) NOEXCEPT;
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundZ(float fsin, float fcos) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundAxis(const float3& axis, float radians) NOEXCEPT;
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixAroundAxis(const float3& axis, float fsin, float fcos) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float3x3 Make3DRotationMatrixFromQuaternion(const FQuaternion& q) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD FQuaternion MakeQuaternionFromRotationMatrix(const float3x3& m) NOEXCEPT;
PPE_CORE_API NODISCARD FQuaternion MakeQuaternionFromRotationMatrix(const float4x4& m) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 MakeTranslationMatrix(const float3& t) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 MakeInvertTranslationMatrix(const float3& t) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 MakeScalingMatrix(const float3& s) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 MakeInvertScalingMatrix(const float3& s) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 MakeLootAtMatrix(const float3& eye, const float3& target, const float3& up) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 MakeInvertLookAtMatrix(const float3& eye, const float3& target, const float3& up) NOEXCEPT;
//----------------------------------------------------------------------------
// from right-handed to left-handed coordinates (ex: OGL/VK is RH, whereas DX is LH)
PPE_CORE_API NODISCARD float4x4 MakeRightToLeftMatrix() NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 MakeInvertRightToLeftMatrix() NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 MakeProjectionMatrix(float znear, float zfar) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 MakeInvertProjectionMatrix(float znear, float zfar) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 MakeOrthoProjectionMatrix(float left, float right, float bottom, float top, float znear, float zfar) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 MakeInvertOrthoProjectionMatrix(float left, float right, float bottom, float top, float znear, float zfar) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 MakePerspectiveProjectionMatrix(float fov, float aspect, float znear, float zfar) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 MakeInvertPerspectiveProjectionMatrix(float fov, float aspect, float znear, float zfar) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float4x4 MakeBillboardMatrix(const float3& obj, const float3& eye, const float3& up, const float3& fwd) NOEXCEPT;
//----------------------------------------------------------------------------
// light : The light direction. If the W component is 0, the light is directional light; if the
// W component is 1, the light is a point light.
PPE_CORE_API NODISCARD float4x4 MakeShadowMatrix(const float4& light, const FPlane& plane) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float2x3 PackSymmetricMatrix(const float3x3& m) NOEXCEPT;
PPE_CORE_API NODISCARD float3x3 UnpackSymmetricMatrix(const float2x3& m) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API NODISCARD float3x4 PackHomogeneousMatrix(const float4x4& m) NOEXCEPT;
PPE_CORE_API NODISCARD float4x4 UnpackHomogeneousMatrix(const float3x4& m) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// * LH <=> Left Handed
// * RH <=> Right Handed
//----------------------------------------------------------------------------
#if 0
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakeBillboardMatrix( const TScalarVector<T, 3>& objectPosition,
                                            const TScalarVector<T, 3>& cameraPosition,
                                            const TScalarVector<T, 3>& cameraUpVector,
                                            const TScalarVector<T, 3>& cemaraForward);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakeLookAtLHMatrix(   const TScalarVector<T, 3>& eye,
                                            const TScalarVector<T, 3>& target,
                                            const TScalarVector<T, 3>& up );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakeLookAtRHMatrix(   const TScalarVector<T, 3>& eye,
                                            const TScalarVector<T, 3>& target,
                                            const TScalarVector<T, 3>& up );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakeOrthographicLHMatrix(T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakeOrthographicRHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakeOrthographicOffCenterLHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakeOrthographicOffCenterRHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakePerspectiveLHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakePerspectiveRHMatrix(  T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakePerspectiveOffCenterLHMatrix( T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakePerspectiveOffCenterRHMatrix( T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakePerspectiveFovLHMatrix(  T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD
TScalarMatrix<T, 4, 4> MakePerspectiveFovRHMatrix(  T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar );
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> MakeReflectionMatrix(const FPlane& plane);
//----------------------------------------------------------------------------
// light : The light direction. If the W component is 0, the light is directional light; if the
/// W component is 1, the light is a point light.
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> MakeShadowMatrix(const TScalarVector<T, 4>& light, const FPlane& plane);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _N>
NODISCARD TScalarMatrix<T, _N, _N> MakeDiagonalMatrix(const TScalarVector<T, _N>& diagonal);
//----------------------------------------------------------------------------
template <typename T, u32 _N>
NODISCARD TScalarMatrix<T, _N + 1, _N + 1> MakeScalingMatrix(const TScalarVector<T, _N>& scale);
//----------------------------------------------------------------------------
template <typename T, u32 _N>
NODISCARD TScalarMatrix<T, _N + 1, _N + 1> MakeTranslationMatrix(const TScalarVector<T, _N>& translate);
//----------------------------------------------------------------------------
template <typename T, u32 _N>
NODISCARD TScalarMatrix<T, _N - 1, _N> PackHomogeneousMatrix(const TScalarMatrix<T, _N, _N>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 2, 2> Make2DRotationMatrix(T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 2, 2> Make2DRotationMatrix(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 2> Make2DTransformMatrix(const TScalarVector<T, 2>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 2> Make2DTransformMatrix(const TScalarVector<T, 2>& translate, const TScalarVector<T, 2>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD TScalarMatrix<float, 3, 3> Make3DRotationMatrix(const FQuaternion& rotation);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis, T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> MakeYawPitchRollMatrix(T yaw, T pitch, T roll);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundX(T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundX(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundY(T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundY(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundZ(T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundZ(T fsin, T fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, T scale, const TScalarVector<T, 3>& axis, T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, T scale, const FQuaternion& rotation);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const TScalarVector<T, 3>& axis, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const FQuaternion& rotation);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const TScalarMatrix<T, 3, 3>& rotation);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const TScalarMatrix<T, 4, 4>& rotation);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const TScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const TScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const TScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos);
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 3> Transform3(const TScalarMatrix<T, 3, 3>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 3> Transform3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 3> TransformPosition3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 3> TransformNormal3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 3> TransformNormal3(const TScalarMatrix<T, 3, 3>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 4> Transform3_OneExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 4> Transform3_ZeroExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
NODISCARD TScalarVector<T, 4> Transform4(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 4>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
void SwapEndiannessInPlace(TScalarMatrix<T, _Rows, _Cols>* value) NOEXCEPT {
    forrange(i, 0, lengthof(value->data))
        FPlatformEndian::SwapInPlace(&value->data[i]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarMatrixHelpers-inl.h"
