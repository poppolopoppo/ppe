#include "stdafx.h"

#include "Graphics.h"

#include "GlobalVideoMemory.h"
#include "Name.h"

#include "Window/BasicWindow.h"

#include "Device/State/BlendState.h"
#include "Device/State/DepthStencilState.h"
#include "Device/State/RasterizerState.h"
#include "Device/State/SamplerState.h"
#include "Device/Texture/SurfaceFormat.h"
#include "Device/Geometry/VertexDeclaration.h"
#include "RenderDocWrapper.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#if defined(_DEBUG) && defined(PLATFORM_WINDOWS)
#   define WITH_CORE_MATHS_UNITTESTS
#endif
#ifdef WITH_CORE_MATHS_UNITTESTS
#   include "Core/Maths/MathHelpers.h"
#   include "Core/Maths/ScalarVectorHelpers.h"
#   include "Core/Maths/QuaternionHelpers.h"
#   include "Core/Maths/ScalarMatrixHelpers.h"
#   include <DirectXMath.h>
#endif

PRAGMA_INITSEG_LIB

namespace Core {
namespace Graphics {
POOL_TAG_DEF(Graphics);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef WITH_CORE_MATHS_UNITTESTS
using namespace DirectX;
//----------------------------------------------------------------------------
struct FRawMatrix {
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
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
static bool Equals_(const XMVECTOR& lhs, const TScalarVector<T, _Dim>& rhs) {
    float lhs_d[_Dim];
    memcpy(lhs_d, &lhs, sizeof(lhs_d));
    float rhs_d[_Dim];
    memcpy(rhs_d, &rhs, sizeof(rhs_d));

    for (size_t i = 0; i < _Dim; ++i)
        if (false == NearlyEquals(lhs_d[i], rhs_d[i]))
            return false;

    return true;
}

template <typename T, size_t _Width, size_t _Height>
static bool Equals_(const TScalarMatrix<T, _Width, _Height>& lhs, const TScalarMatrix<T, _Width, _Height>& rhs) {
    const ScalarMatrixData<T, _Width, _Height>& lhsData = lhs.data();
    const ScalarMatrixData<T, _Width, _Height>& rhsData = rhs.data();
    for (size_t i = 0; i < _Width*_Height; ++i)
        if (false == NearlyEquals(lhsData.raw[i], rhsData.raw[i]))
            return false;

    return true;
}

static bool Equals_(const XMMATRIX& lhs, const float4x4& rhs) {
    STATIC_ASSERT(sizeof(lhs) == sizeof(rhs));
    STATIC_ASSERT(sizeof(FRawMatrix) == sizeof(lhs));

    FRawMatrix lhs_d;
    memcpy(&lhs_d, &lhs, sizeof(lhs_d));
    FRawMatrix rhs_d;
    memcpy(&rhs_d, &rhs, sizeof(rhs_d));

    Assert(NearlyEquals(rhs_d.data._23, rhs_d.data.m[1][2]));
    Assert(NearlyEquals(rhs_d.data._23, rhs.at(1, 2)));
    Assert(NearlyEquals(rhs_d.data._23, rhs.at<1, 2>()));
    Assert(NearlyEquals(rhs_d.data._23, rhs._23()));
    Assert(NearlyEquals(rhs_d.data._23, rhs.m12()));

    Assert(NearlyEquals(rhs_d.data._12, rhs_d.data.m[0][1]));
    Assert(NearlyEquals(rhs_d.data._12, rhs._12()));
    Assert(NearlyEquals(rhs_d.data._12, rhs.m01()));

    Assert(NearlyEquals(rhs_d.data._34, rhs_d.data.m[2][3]));
    Assert(NearlyEquals(rhs_d.data._34, rhs._34()));
    Assert(NearlyEquals(rhs_d.data._34, rhs.m23()));

    for (size_t i = 0; i < 16; ++i)
        if (false == NearlyEquals(lhs_d.data.raw[i], rhs_d.data.raw[i]))
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
    const float4x4 c_rx = Make3DRotationMatrixAroundX(F_PIOver3).OneExtend();
    AssertRelease(Equals_(x_rx, c_rx));

    const XMMATRIX x_ry = XMMatrixRotationY(F_PIOver3);
    const float4x4 c_ry = Make3DRotationMatrixAroundY(F_PIOver3).OneExtend();
    AssertRelease(Equals_(x_ry, c_ry));

    const XMMATRIX x_rz = XMMatrixRotationZ(F_PIOver3);
    const float4x4 c_rz = Make3DRotationMatrixAroundZ(F_PIOver3).OneExtend();
    AssertRelease(Equals_(x_rz, c_rz));

    const float3 axis = Normalize3(float3(1,2,3));
    const FXMVECTOR x_axis = {axis.x(),axis.y(),axis.z(),0};

    const XMMATRIX x_ra = XMMatrixRotationAxis(x_axis, F_PIOver3);
    const float4x4 c_ra = Make3DRotationMatrix(axis, F_PIOver3).OneExtend();
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
    FQuaternion rot = FQuaternion::Identity;
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
    Assert(Equals_(x_det, TScalarVector<float, 1>(c_det)));
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

    const float m_view_raw[16] = {
        0.720537543f,
        -0.607772529f,
        -0.333823651f,
        0.000000000f,
        -5.96046448e-008f,
        0.481419027f,
        -0.876490593f,
        0.000000000f,
        0.693415940f,
        0.631544352f,
        0.346880436f,
        0.000000000f,
        4.16049576f,
        2.34500909f,
        4.71075439f,
        1.00000000f,
    };

    XMMATRIX x_view;
    memcpy(&x_view, m_view_raw, sizeof(m_view_raw));
    float4x4 c_view;
    memcpy(&c_view, m_view_raw, sizeof(m_view_raw));
    AssertRelease(Equals_(x_view, c_view));

    const float m_proj_raw[16] = {
        0.974278688f,
        0.000000000f,
        0.000000000f,
        0.000000000f,
        0.000000000f,
        1.73205090f,
        0.000000000f,
        0.000000000f,
        -0.000000000f,
        -0.000000000f,
        1.00001001f,
        1.00000000f,
        0.000000000f,
        0.000000000f,
        0.0100001004f,
        0.000000000f,
    };

    XMMATRIX x_proj;
    memcpy(&x_proj, m_proj_raw, sizeof(m_proj_raw));
    float4x4 c_proj;
    memcpy(&c_proj, m_proj_raw, sizeof(m_proj_raw));
    AssertRelease(Equals_(x_proj, c_proj));

    XMMATRIX x_viewproj = x_view * x_proj;
    float4x4 c_viewproj = c_view.Multiply(c_proj);
    AssertRelease(Equals_(x_viewproj, c_viewproj));

    const float v_wpos_raw[4] = {12,26,8,1};

    XMVECTOR x_wpos;
    memcpy(&x_wpos, v_wpos_raw, sizeof(v_wpos_raw));
    float4 c_wpos;
    memcpy(&c_wpos, v_wpos_raw, sizeof(v_wpos_raw));
    AssertRelease(Equals_(x_wpos, c_wpos));

    XMVECTOR x_hpos = XMVector4Transform(x_wpos, x_viewproj);
    float4 c_hpos = Transform4(c_viewproj, c_wpos);
    AssertRelease(Equals_(x_hpos, c_hpos));
    float4 c_hpos2 = Transform3_OneExtend(c_viewproj, c_wpos.xyz());
    AssertRelease(Equals_(x_hpos, c_hpos2));
    float4 c_hpos3 = c_viewproj.Multiply(c_wpos);
    AssertRelease(Equals_(x_hpos, c_hpos3));
    float4 c_hpos4 = c_viewproj.Multiply_OneExtend(c_wpos.xyz());
    AssertRelease(Equals_(x_hpos, c_hpos4));
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
void FGraphicsModule::Start() {
    CORE_MODULE_START(Graphics);

#ifdef WITH_CORE_MATHS_UNITTESTS
    MathUnitTests_();
#endif

    // 0 - pool allocator tag
    POOL_TAG(Graphics)::Start();
    // 1 - Graphics Names
    FName::Start(256);
    // 2 - Global video memory
    FGlobalVideoMemory::Create();
    // 3 - Register basic window class
    FBasicWindow::Start();
    // 4 - Surface format builtin-types
    FSurfaceFormat::Start();
    // 5 - Vertex declaration builtin-types
    FVertexDeclaration::Start();
    // 6 - Device states
    FBlendState::Start();
    FDepthStencilState::Start();
    FRasterizerState::Start();
    FSamplerState::Start();
    // 7 - render doc wrapper
#ifdef WITH_CORE_RENDERDOC
    FRenderDocWrapper::Create();
#endif
}
//----------------------------------------------------------------------------
void FGraphicsModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Graphics);

    // 7 - render doc wrapper
#ifdef WITH_CORE_RENDERDOC
    FRenderDocWrapper::Destroy();
#endif
    // 6 - Device states
    FSamplerState::Shutdown();
    FRasterizerState::Shutdown();
    FDepthStencilState::Shutdown();
    FBlendState::Shutdown();
    // 5 - Vertex declaration builtin-types
    FVertexDeclaration::Shutdown();
    // 4 - Surface format builtin-types
    FSurfaceFormat::Shutdown();
    // 3 - Unregister basic window class
    FBasicWindow::Shutdown();
    // 2 - Global video memory
    FGlobalVideoMemory::Destroy();
    // 1 - Graphic Names
    FName::Shutdown();
    // 0 - pool allocator tag
    POOL_TAG(Graphics)::Shutdown();
}
//----------------------------------------------------------------------------
void FGraphicsModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Graphics);

    POOL_TAG(Graphics)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGraphicsModule::OnDeviceCreate(FDeviceEncapsulator *device) {
    // 1 - Surface format builtin-types
    FSurfaceFormat::OnDeviceCreate(device);
    // 2 - Vertex declaration builtin-types
    FVertexDeclaration::OnDeviceCreate(device);
    // 3 - Device states
    FBlendState::OnDeviceCreate(device);
    FDepthStencilState::OnDeviceCreate(device);
    FRasterizerState::OnDeviceCreate(device);
    FSamplerState::OnDeviceCreate(device);
}
//----------------------------------------------------------------------------
void FGraphicsModule::OnDeviceDestroy(FDeviceEncapsulator *device) {
    // 3 - Device states
    FSamplerState::OnDeviceDestroy(device);
    FRasterizerState::OnDeviceDestroy(device);
    FDepthStencilState::OnDeviceDestroy(device);
    FBlendState::OnDeviceDestroy(device);
    // 2 - Vertex declaration builtin-types
    FVertexDeclaration::OnDeviceDestroy(device);
    // 1 - Surface format builtin-types
    FSurfaceFormat::OnDeviceDestroy(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
