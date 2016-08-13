#pragma once

#include "Core/Core.h"

#include "Core/Maths/Plane.h"

#include "Core/Maths/Quaternion.h"
#include "Core/Maths/QuaternionHelpers.h"
#include "Core/Maths/ScalarMatrix.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsHomogeneous(const ScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsInversible(const ScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T>
bool IsInvertible(const ScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsOrthogonal(const ScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsOrthonormal(const ScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool IsSymetrical(const ScalarMatrix<T, _Width, _Height>& m, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height> Abs(const ScalarMatrix<T, _Width, _Height>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void Lerp(  const ScalarMatrix<T, _Width, _Height>& v0,
            const ScalarMatrix<T, _Width, _Height>& v1,
            float f,
            ScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height> Lerp(  const ScalarMatrix<T, _Width, _Height>& v0,
                                        const ScalarMatrix<T, _Width, _Height>& v1,
                                        float f );
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void SLerp( const ScalarMatrix<T, _Width, _Height>& v0,
            const ScalarMatrix<T, _Width, _Height>& v1,
            float f,
            ScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void Smoothstep(const ScalarMatrix<T, _Width, _Height>& v0,
                const ScalarMatrix<T, _Width, _Height>& v1,
                float f,
                ScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void Smootherstep(  const ScalarMatrix<T, _Width, _Height>& v0,
                    const ScalarMatrix<T, _Width, _Height>& v1,
                    float f,
                    ScalarMatrix<T, _Width, _Height>& result);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Orthogonalize(const ScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, 4, 4> Orthonormalize(const ScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void DecomposeQR(const ScalarMatrix<T, 4, 4>& m, ScalarMatrix<T, 4, 4>& q, ScalarMatrix<T, 4, 4>& r);
//----------------------------------------------------------------------------
template <typename T>
void DecomposeLQ(const ScalarMatrix<T, 4, 4>& m, ScalarMatrix<T, 4, 4>& l, ScalarMatrix<T, 4, 4>& q);
//----------------------------------------------------------------------------
template <typename T>
void Decompose( const ScalarMatrix<T, 4, 4>& transform,
                ScalarVector<T, 3>& scale,
                Quaternion& rotation,
                ScalarVector<T, 3>& translation );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det(const ScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det(const ScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det(const ScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const ScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const ScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const ScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 2, 2> Invert(const ScalarMatrix<T, 2, 2>& m);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 3, 3> Invert(const ScalarMatrix<T, 3, 3>& m);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Invert(const ScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Invert_AssumeHomogeneous(const ScalarMatrix<T, 4, 4>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Height, _Width> InvertTranspose(const ScalarMatrix<T, _Width, _Height>& m);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// * LH <=> Left Handed
// * RH <=> Right Handed
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeBillboardMatrix(  const ScalarVector<T, 3>& objectPosition,
                                            const ScalarVector<T, 3>& cameraPosition,
                                            const ScalarVector<T, 3>& cameraUpVector,
                                            const ScalarVector<T, 3>& cemaraForward);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeLookAtLHMatrix(   const ScalarVector<T, 3>& eye,
                                            const ScalarVector<T, 3>& target,
                                            const ScalarVector<T, 3>& up );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeLookAtRHMatrix(   const ScalarVector<T, 3>& eye,
                                            const ScalarVector<T, 3>& target,
                                            const ScalarVector<T, 3>& up );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeOrthographicLHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeOrthographicRHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeOrthographicOffCenterLHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeOrthographicOffCenterRHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakePerspectiveLHMatrix(  T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakePerspectiveRHMatrix(  T width,
                                                T height,
                                                T znear,
                                                T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakePerspectiveOffCenterLHMatrix( T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakePerspectiveOffCenterRHMatrix( T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakePerspectiveFovLHMatrix(   T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakePerspectiveFovRHMatrix(   T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar );
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> MakeReflectionMatrix(const Plane& plane);
//----------------------------------------------------------------------------
// light : The light direction. If the W component is 0, the light is directional light; if the
/// W component is 1, the light is a point light.
template <typename T>
ScalarMatrix<T, 4, 4> MakeShadowMatrix(const ScalarVector<T, 4>& light, const Plane& plane);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _N>
ScalarMatrix<T, _N + 1, _N + 1> MakeHomogeneousMatrix(const ScalarMatrix<T, _N, _N>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _N>
ScalarMatrix<T, _N + 1, _N + 1> MakeScalingMatrix(const ScalarVector<T, _N>& scale);
//----------------------------------------------------------------------------
template <typename T, size_t _N>
ScalarMatrix<T, _N + 1, _N + 1> MakeTranslationMatrix(const ScalarVector<T, _N>& translate);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 2, 2> Make2DRotationMatrix(T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 2, 2> Make2DRotationMatrix(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 3, 2> Make2DTransformMatrix(const ScalarVector<T, 2>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 3, 2> Make2DTransformMatrix(const ScalarVector<T, 2>& translate, const ScalarVector<T, 2>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrix(const ScalarVector<T, 3>& axis);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrix(const ScalarVector<T, 3>& axis, T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrix(const ScalarVector<T, 3>& axis, T fsin, T fcos);
//----------------------------------------------------------------------------
ScalarMatrix<float, 4, 4> Make3DRotationMatrix(const Quaternion& rotation);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DYawPitchRollMatrix(T yaw, T pitch, T roll);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrixAroundX(T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrixAroundX(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrixAroundY(T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrixAroundY(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrixAroundZ(T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DRotationMatrixAroundZ(T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrix(const ScalarVector<T, 3>& translate, T scale, const ScalarVector<T, 3>& axis, T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrix(const ScalarVector<T, 3>& translate, const ScalarVector<T, 3>& scale, const ScalarVector<T, 3>& axis, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrix(const ScalarVector<T, 3>& translate, const ScalarVector<T, 3>& scale, const Quaternion& rotation);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrix(const ScalarVector<T, 3>& translate, const ScalarVector<T, 3>& scale, const ScalarMatrix<T, 4, 4>& rotation);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const ScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const ScalarVector<T, 3>& translate, const ScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const ScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const ScalarVector<T, 3>& translate, const ScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const ScalarVector<T, 3>& translate, T scale, T radians);
//----------------------------------------------------------------------------
template <typename T>
ScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const ScalarVector<T, 3>& translate, const ScalarVector<T, 3>& scale, T fsin, T fcos);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 3> Transform3(const ScalarMatrix<T, 3, 3>& m, const ScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 4> Transform3_OneExtend(const ScalarMatrix<T, 4, 4>& m, const ScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 4> Transform3_ZeroExtend(const ScalarMatrix<T, 4, 4>& m, const ScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 4> Transform4(const ScalarMatrix<T, 4, 4>& m, const ScalarVector<T, 4>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarMatrixHelpers-inl.h"
