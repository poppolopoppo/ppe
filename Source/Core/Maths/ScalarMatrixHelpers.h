#pragma once

#include "Core/Core.h"

#include "Core/Maths/Plane.h"

#include "Core/Maths/Quaternion.h"
#include "Core/Maths/QuaternionHelpers.h"
#include "Core/Maths/ScalarMatrix.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

#include "Core/Misc/Endianness.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsHomogeneous(const TScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsInversible(const TScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T>
bool IsInvertible(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsOrthogonal(const TScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsOrthonormal(const TScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsSymetrical(const TScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsINF(const TScalarMatrix<T, _Width, _Height>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsNAN(const TScalarMatrix<T, _Width, _Height>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsNANorINF(const TScalarMatrix<T, _Width, _Height>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
TScalarMatrix<T, _Width, _Height> Abs(const TScalarMatrix<T, _Width, _Height>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void Lerp(  const TScalarMatrix<T, _Width, _Height>& v0,
            const TScalarMatrix<T, _Width, _Height>& v1,
            float f,
            TScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
TScalarMatrix<T, _Width, _Height> Lerp( const TScalarMatrix<T, _Width, _Height>& v0,
                                        const TScalarMatrix<T, _Width, _Height>& v1,
                                        float f );
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void SLerp( const TScalarMatrix<T, _Width, _Height>& v0,
            const TScalarMatrix<T, _Width, _Height>& v1,
            float f,
            TScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void Smoothstep(const TScalarMatrix<T, _Width, _Height>& v0,
                const TScalarMatrix<T, _Width, _Height>& v1,
                float f,
                TScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void Smootherstep(  const TScalarMatrix<T, _Width, _Height>& v0,
                    const TScalarMatrix<T, _Width, _Height>& v1,
                    float f,
                    TScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Orthogonalize(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
TScalarMatrix<T, 4, 4> Orthonormalize(const TScalarMatrix<T, 4, 4>& m);
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
T Det(const TScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 2, 2> Invert(const TScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Invert(const TScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Invert(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Invert_AssumeHomogeneous(const TScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
TScalarMatrix<T, _Height, _Width> InvertTranspose(const TScalarMatrix<T, _Width, _Height>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// * LH <=> Left Handed
// * RH <=> Right Handed
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeBillboardMatrix(  const TScalarVector<T, 3>& objectPosition,
                                            const TScalarVector<T, 3>& cameraPosition,
                                            const TScalarVector<T, 3>& cameraUpVector,
                                            const TScalarVector<T, 3>& cemaraForward);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeLookAtLHMatrix(   const TScalarVector<T, 3>& eye,
                                            const TScalarVector<T, 3>& target,
                                            const TScalarVector<T, 3>& up );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeLookAtRHMatrix(   const TScalarVector<T, 3>& eye,
                                            const TScalarVector<T, 3>& target,
                                            const TScalarVector<T, 3>& up );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicLHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicRHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicOffCenterLHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicOffCenterRHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveLHMatrix(  T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveRHMatrix(  T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveOffCenterLHMatrix( T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveOffCenterRHMatrix( T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveFovLHMatrix(   T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveFovRHMatrix(   T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar );
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeReflectionMatrix(const FPlane& plane);
//----------------------------------------------------------------------------
// light : The light direction. If the W component is 0, the light is directional light; if the
/// W component is 1, the light is a point light.
template <typename T>
TScalarMatrix<T, 4, 4> MakeShadowMatrix(const TScalarVector<T, 4>& light, const FPlane& plane);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _N>
TScalarMatrix<T, _N + 1, _N + 1> MakeHomogeneousMatrix(const TScalarMatrix<T, _N, _N>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _N>
TScalarMatrix<T, _N + 1, _N + 1> MakeScalingMatrix(const TScalarVector<T, _N>& scale);
//----------------------------------------------------------------------------
template <typename T, size_t _N>
TScalarMatrix<T, _N + 1, _N + 1> MakeTranslationMatrix(const TScalarVector<T, _N>& translate);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 2, 2> Make2DRotationMatrix(T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 2, 2> Make2DRotationMatrix(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 2> Make2DTransformMatrix(const TScalarVector<T, 2>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 2> Make2DTransformMatrix(const TScalarVector<T, 2>& translate, const TScalarVector<T, 2>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TScalarMatrix<float, 3, 3> Make3DRotationMatrix(const FQuaternion& rotation);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis, T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> MakeYawPitchRollMatrix(T yaw, T pitch, T roll);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundX(T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundX(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundY(T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundY(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundZ(T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundZ(T fsin, T fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, T scale, const TScalarVector<T, 3>& axis, T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const TScalarVector<T, 3>& axis, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const FQuaternion& rotation);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const TScalarMatrix<T, 4, 4>& rotation);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const TScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const TScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const TScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> Transform3(const TScalarMatrix<T, 3, 3>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> TransformPosition3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> TransformVector3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform3_OneExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform3_ZeroExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform4(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 4>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
TScalarMatrix<T, _Width, _Height> SwapEndianness(const TScalarMatrix<T, _Width, _Height>& value) {
    TScalarMatrix<T, _Width, _Height> r;
    forrange(i, 0, lengthof(r._data.raw))
        r._data.raw[i] = SwapEndianness(value._data.raw[i]);
    return r;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarMatrixHelpers-inl.h"
