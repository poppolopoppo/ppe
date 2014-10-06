#include "stdafx.h"

#include "Graphics.h"

#include "BindName.h"

#include "BasicWindow.h"

#include "BlendState.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "SamplerState.h"

#include "SurfaceFormat.h"

#include "VertexDeclaration.h"

#if defined(_DEBUG) && defined(OS_WINDOWS)
#   define WITH_CORE_MATHS_UNITTESTS
#endif
#ifdef WITH_CORE_MATHS_UNITTESTS
#   include "Core/ScalarMatrixHelpers.h"
#   include "Core/ScalarVectorHelpers.h"
#   include "Core/QuaternionHelpers.h"
#   include <DirectXMath.h>
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef WITH_CORE_MATHS_UNITTESTS
using namespace DirectX;
//----------------------------------------------------------------------------
struct RawMatrix {
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
        float raw[16];
    }   data;
};

static bool AlmostEqualRelative_(float A, float B, float maxRelDiff=1e-3f)
{
    // Calculate the difference.
    float diff = fabs(A - B);
    A = fabs(A);
    B = fabs(B);
    // Find the largest
    float largest = (B > A) ? B : A;

    if (diff <= largest * maxRelDiff)
        return true;
    return false;
}

template <typename T, size_t _Dim>
static bool Equals_(const XMVECTOR& lhs, const ScalarVector<T, _Dim>& rhs) {
    float lhs_d[_Dim];
    memcpy(lhs_d, &lhs, sizeof(lhs_d));
    float rhs_d[_Dim];
    memcpy(rhs_d, &rhs, sizeof(rhs_d));

    for (size_t i = 0; i < _Dim; ++i)
        if (!AlmostEqualRelative_(lhs_d[i], rhs_d[i]))
            return false;

    return true;
}

template <typename T, size_t _Width, size_t _Height>
static bool Equals_(const ScalarMatrix<T, _Width, _Height>& lhs, const ScalarMatrix<T, _Width, _Height>& rhs) {
    for (size_t i = 0; i < _Width*_Height; ++i)
        if (!AlmostEqualRelative_(lhs.data_()[i], rhs.data_()[i]))
            return false;

    return true;
}

static bool Equals_(const XMMATRIX& lhs, const float4x4& rhs) {
    RawMatrix lhs_d;
    memcpy(&lhs_d, &lhs, sizeof(lhs_d));
    RawMatrix rhs_d;
    memcpy(&rhs_d, &rhs, sizeof(rhs_d));

    Assert(AlmostEqualRelative_(rhs_d.data._23, rhs_d.data.m[1][2]));
    Assert(AlmostEqualRelative_(rhs_d.data._23, rhs.at(1, 2)));
    Assert(AlmostEqualRelative_(rhs_d.data._23, rhs.at<1, 2>()));
    Assert(AlmostEqualRelative_(rhs_d.data._23, rhs._23()));
    Assert(AlmostEqualRelative_(rhs_d.data._23, rhs.m12()));

    Assert(AlmostEqualRelative_(rhs_d.data._12, rhs_d.data.m[0][1]));
    Assert(AlmostEqualRelative_(rhs_d.data._12, rhs._12()));
    Assert(AlmostEqualRelative_(rhs_d.data._12, rhs.m01()));

    Assert(AlmostEqualRelative_(rhs_d.data._34, rhs_d.data.m[2][3]));
    Assert(AlmostEqualRelative_(rhs_d.data._34, rhs._34()));
    Assert(AlmostEqualRelative_(rhs_d.data._34, rhs.m23()));

    for (size_t i = 0; i < 16; ++i)
        if (!AlmostEqualRelative_(lhs_d.data.raw[i], rhs_d.data.raw[i]))
            return false;

    return true;
}
//----------------------------------------------------------------------------
static void MatrixUnitTests_() {
    const XMMATRIX x_id = XMMatrixIdentity();
    const float4x4 c_id = float4x4::Identity();
    AssertRelease(Equals_(x_id, c_id));

    const XMMATRIX x_sc = XMMatrixScaling(2, 3, 4);
    const float4x4 c_sc = MakeScalingMatrix(float3(2, 3, 4));
    AssertRelease(Equals_(x_sc, c_sc));

    const XMMATRIX x_rx = XMMatrixRotationX(F_PIOver3);
    const float4x4 c_rx = Make3DRotationMatrixAroundX(F_PIOver3);
    AssertRelease(Equals_(x_rx, c_rx));

    const XMMATRIX x_ry = XMMatrixRotationY(F_PIOver3);
    const float4x4 c_ry = Make3DRotationMatrixAroundY(F_PIOver3);
    AssertRelease(Equals_(x_ry, c_ry));

    const XMMATRIX x_rz = XMMatrixRotationZ(F_PIOver3);
    const float4x4 c_rz = Make3DRotationMatrixAroundZ(F_PIOver3);
    AssertRelease(Equals_(x_rz, c_rz));

    const float3 axis = Normalize3(float3(1,2,3));
    const FXMVECTOR x_axis = {axis.x(),axis.y(),axis.z(),0};

    const XMMATRIX x_ra = XMMatrixRotationAxis(x_axis, F_PIOver3);
    const float4x4 c_ra = Make3DRotationMatrix(axis, F_PIOver3);
    AssertRelease(Equals_(x_ra, c_ra));

    const XMMATRIX x_tr = XMMatrixTranslation(-1, -2, -3);
    const float4x4 c_tr = MakeTranslationMatrix(float3(-1, -2, -3));
    AssertRelease(Equals_(x_tr, c_tr));

    const XMMATRIX x_af = XMMatrixMultiply(x_sc, XMMatrixMultiply(x_rx, x_tr));
    const float4x4 c_af = c_sc * c_rx * c_tr;
    AssertRelease(Equals_(x_af, c_af));

    XMVECTOR x_vsc;
    XMVECTOR x_vqt;
    XMVECTOR x_vtr;
    XMMatrixDecompose(&x_vsc, &x_vqt, &x_vtr, x_af);
    float3 scale;
    Quaternion rot = Quaternion::Identity();
    float3 translate;
    Decompose(c_af, scale, rot, translate);
    Assert(Equals_(x_vsc, scale));
    Assert(Equals_(x_vqt, rot.Value()));
    Assert(Equals_(x_vtr, translate));

    const float4x4 c_af2 = Make3DTransformMatrix(translate, scale, rot);
    Assert(Equals_(c_af, c_af2));

    const FXMVECTOR eyePos = {-1,-2,-3,0};
    const FXMVECTOR eyeFocus = {-4,-5,-6,0};
    const FXMVECTOR eyeUp = {0,1,0,0};

    XMVECTOR x_det;
    const XMMATRIX x_af_inv = XMMatrixInverse(&x_det, x_af);
    const float c_det = Det(c_af);
    Assert(Equals_(x_det, ScalarVector<float, 1>(c_det)));
    const float4x4 c_af_inv = Invert(c_af);
    Assert(Equals_(x_af_inv, c_af_inv));

    const XMMATRIX x_lklh = XMMatrixLookAtLH(eyePos, eyeFocus, eyeUp);
    const float4x4 c_lklh = MakeLookAtLHMatrix(float3(-1, -2, -3), float3(-4,-5,-6), float3(0,1,0));
    AssertRelease(Equals_(x_lklh, c_lklh));

    const XMMATRIX x_lkrh = XMMatrixLookAtRH(eyePos, eyeFocus, eyeUp);
    const float4x4 c_lkrh = MakeLookAtRHMatrix(float3(-1, -2, -3), float3(-4,-5,-6), float3(0,1,0));
    AssertRelease(Equals_(x_lkrh, c_lkrh));

    const XMMATRIX x_pslh = XMMatrixPerspectiveFovLH(F_PIOver3, 0.5f, 100.0f, 1000.0f);
    const float4x4 c_pslh = MakePerspectiveFovLHMatrix(F_PIOver3, 0.5f, 100.0f, 1000.0f);
    AssertRelease(Equals_(x_pslh, c_pslh));

    const XMMATRIX x_psrh = XMMatrixPerspectiveFovRH(F_PIOver3, 0.5f, 100.0f, 1000.0f);
    const float4x4 c_psrh = MakePerspectiveFovRHMatrix(F_PIOver3, 0.5f, 100.0f, 1000.0f);
    AssertRelease(Equals_(x_psrh, c_psrh));

    const XMMATRIX x_otlh = XMMatrixOrthographicLH(1280.0f, 720.0f, 10.0f, 100.0f);
    const float4x4 c_otlh = MakeOrthographicLHMatrix(1280.0f, 720.0f, 10.0f, 100.0f);
    AssertRelease(Equals_(x_otlh, c_otlh));

    const XMMATRIX x_otrh = XMMatrixOrthographicRH(1280.0f, 720.0f, 10.0f, 100.0f);
    const float4x4 c_otrh = MakeOrthographicRHMatrix(1280.0f, 720.0f, 10.0f, 100.0f);
    AssertRelease(Equals_(x_otrh, c_otrh));
}
//----------------------------------------------------------------------------
static void MathUnitTests_() {
    MatrixUnitTests_();
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_MATHS_UNITTESTS
//----------------------------------------------------------------------------
} // !namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void GraphicsStartup::Start() {
#ifdef WITH_CORE_MATHS_UNITTESTS
    MathUnitTests_();
#endif

    // 1 - Register basic window class
    BasicWindow::Start();
    // 2 - Surface format builtin-types
    SurfaceFormat::Start();
    // 3 - Vertex declaration builtin-types
    VertexDeclaration::Start();
    // 4 - Device states
    BlendState::Start();
    DepthStencilState::Start();
    RasterizerState::Start();
    SamplerState::Start();
    // 5 - Bind names
    BindName::Start(256);
}
//----------------------------------------------------------------------------
void GraphicsStartup::Shutdown() {
    // 1 - Bind names
    BindName::Shutdown();
    // 4 - Device states
    SamplerState::Shutdown();
    RasterizerState::Shutdown();
    DepthStencilState::Shutdown();
    BlendState::Shutdown();
    // 3 - Vertex declaration builtin-types
    VertexDeclaration::Shutdown();
    // 2 - Surface format builtin-types
    SurfaceFormat::Shutdown();
    // 1 - Unregister basic window class
    BasicWindow::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void GraphicsStartup::OnDeviceCreate(DeviceEncapsulator *device) {
    // 1 - Surface format builtin-types
    SurfaceFormat::OnDeviceCreate(device);
    // 2 - Vertex declaration builtin-types
    VertexDeclaration::OnDeviceCreate(device);
    // 3 - Device states
    BlendState::OnDeviceCreate(device);
    DepthStencilState::OnDeviceCreate(device);
    RasterizerState::OnDeviceCreate(device);
    SamplerState::OnDeviceCreate(device);
}
//----------------------------------------------------------------------------
void GraphicsStartup::OnDeviceDestroy(DeviceEncapsulator *device) {
    // 3 - Device states
    SamplerState::OnDeviceDestroy(device);
    RasterizerState::OnDeviceDestroy(device);
    DepthStencilState::OnDeviceDestroy(device);
    BlendState::OnDeviceDestroy(device);
    // 2 - Vertex declaration builtin-types
    VertexDeclaration::OnDeviceDestroy(device);
    // 1 - Surface format builtin-types
    SurfaceFormat::OnDeviceDestroy(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
