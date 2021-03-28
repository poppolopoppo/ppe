#pragma once

#include "ResourceEnums.h"
#include "ResourceState.h"
#include "RHI_fwd.h"

#include "RHI/ShaderEnums.h"
#include "RHI/VertexEnums.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Indices
//----------------------------------------------------------------------------
inline CONSTEXPR u32 EIndexFormat_SizeOf(EIndexFormat fmt) {
    switch (fmt) {
    case EIndexFormat::UShort: return sizeof(u16);
    case EIndexFormat::UInt: return sizeof(u32);
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
// Images
//----------------------------------------------------------------------------
CONSTEXPR bool EImageType_IsArray(EImageType value) {
    switch (value) {
    case EImageType::Tex1DArray:
    case EImageType::Tex2DArray:
    case EImageType::Tex2DMSArray:
    case EImageType::TexCubeArray: return true;
    default: return false;
    }
}
//----------------------------------------------------------------------------
CONSTEXPR bool EImageType_IsCube(EImageType value) {
    return (value == EImageType::TexCube || value == EImageType::TexCubeArray);
}
//----------------------------------------------------------------------------
CONSTEXPR bool EImageType_IsMultiSampled(EImageType value) {
    return (value == EImageType::Tex2DMS || value == EImageType::Tex2DMSArray);
}
//----------------------------------------------------------------------------
// Resources
//----------------------------------------------------------------------------
inline bool CONSTEXPR EResourceState_IsReadable(EResourceState value) {
    return (value & EResourceState::_Read);
}
//----------------------------------------------------------------------------
inline bool CONSTEXPR EResourceState_IsWritable(EResourceState value) {
    return (value & EResourceState::_Write);
}
//----------------------------------------------------------------------------
inline EResourceState CONSTEXPR EResourceState_FromShaders(EShaderStages stages) {
    EResourceState result =  EResourceState::Unknown;

    for (EShaderStages st = EShaderStages(1 << 0); st < EShaderStages::_Last; st = EShaderStages(u32(st) << 1u)) {
        if (not (stages & st))
            continue;

        switch (st) {
        case EShaderStages::Vertex: result |= EResourceState::_VertexShader; break;
        case EShaderStages::TessControl: result |= EResourceState::_TessControlShader; break;
        case EShaderStages::TessEvaluation: result |= EResourceState::_TessEvaluationShader; break;
        case EShaderStages::Geometry: result |= EResourceState::_GeometryShader; break;
        case EShaderStages::Fragment: result |= EResourceState::_FragmentShader; break;
        case EShaderStages::Compute: result |= EResourceState::_ComputeShader; break;
        case EShaderStages::MeshTask: result |= EResourceState::_MeshTaskShader; break;
        case EShaderStages::Mesh: result |= EResourceState::_MeshShader; break;
        case EShaderStages::RayGen:
        case EShaderStages::RayAnyHit:
        case EShaderStages::RayClosestHit:
        case EShaderStages::RayMiss:
        case EShaderStages::RayIntersection:
        case EShaderStages::RayCallable: result |= EResourceState::_RayTracingShader; break;

        case EShaderStages::_Last:
        case EShaderStages::All:
        case EShaderStages::AllGraphics:
        case EShaderStages::AllRayTracing:
        case EShaderStages::Unknown:
        default: AssertNotImplemented();
        }
    }

    return result;
}
//----------------------------------------------------------------------------
CONSTEXPR EShaderAccess EResourceState_ToShaderAccess(EResourceState state) {
    if (state & EResourceState::ShaderReadWrite)
        return EShaderAccess::ReadWrite;
    if (state & (EResourceState::ShaderWrite + EResourceState::InvalidateBefore))
        return EShaderAccess::WriteDiscard;
    if (state & EResourceState::ShaderWrite)
        return EShaderAccess::WriteOnly;
    if (state & EResourceState::ShaderRead)
        return EShaderAccess::ReadOnly;
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
CONSTEXPR EResourceState EResourceState_FromShaderAccess(EShaderAccess access) {
    switch (access) {
    case EShaderAccess::ReadOnly: return EResourceState::ShaderRead;
    case EShaderAccess::WriteOnly: return EResourceState::ShaderWrite;
    case EShaderAccess::WriteDiscard: return EResourceState::ShaderWrite | EResourceState::InvalidateBefore;
    case EShaderAccess::ReadWrite: return EResourceState::ShaderReadWrite;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
// Shaders
//----------------------------------------------------------------------------
inline CONSTEXPR EShaderStages EShaderStages_FromShader(EShaderType shader) {
    switch (shader) {
    case EShaderType::Vertex: return EShaderStages::Vertex;
    case EShaderType::TessControl: return EShaderStages::TessControl;
    case EShaderType::TessEvaluation: return EShaderStages::TessEvaluation;
    case EShaderType::Geometry: return EShaderStages::Geometry;
    case EShaderType::Fragment: return EShaderStages::Fragment;
    case EShaderType::Compute: return EShaderStages::Compute;
    case EShaderType::MeshTask: return EShaderStages::MeshTask;
    case EShaderType::Mesh: return EShaderStages::Mesh;
    case EShaderType::RayGen: return EShaderStages::RayGen;
    case EShaderType::RayAnyHit: return EShaderStages::RayAnyHit;
    case EShaderType::RayClosestHit: return EShaderStages::RayClosestHit;
    case EShaderType::RayMiss: return EShaderStages::RayMiss;
    case EShaderType::RayIntersection: return EShaderStages::RayIntersection;
    case EShaderType::RayCallable: return EShaderStages::RayCallable;
    case EShaderType::_Count:
    case EShaderType::Unknown:
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
inline CONSTEXPR EShaderDebugMode EShaderDebugMode_From(EShaderLangFormat lang) {
    const EShaderLangFormat debugMode = BitAnd(lang, EShaderLangFormat::_DebugModeMask);
    switch (debugMode) {
    case EShaderLangFormat::Unknown: return EShaderDebugMode::None;
    case EShaderLangFormat::EnableProfiling: return EShaderDebugMode::Profiling;
    case EShaderLangFormat::EnableTimeMap: return EShaderDebugMode::Timemap;
    case EShaderLangFormat::EnableDebugTrace: return EShaderDebugMode::Trace;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
// Vertices
//----------------------------------------------------------------------------
inline CONSTEXPR u32 EVertexFormat_SizeOf(EVertexFormat fmt) {
    const EVertexFormat scalar = BitAnd(fmt, EVertexFormat::_TypeMask);
    const u32 arity = u32(BitAnd(fmt, EVertexFormat::_VecMask)) >> u32(EVertexFormat::_VecOffset);

    switch (scalar) {
    case EVertexFormat::_Byte:
    case EVertexFormat::_UByte: return sizeof(u8) * arity;
    case EVertexFormat::_Short:
    case EVertexFormat::_UShort: return sizeof(u16) * arity;
    case EVertexFormat::_Int:
    case EVertexFormat::_UInt: return sizeof(u32) * arity;
    case EVertexFormat::_Long:
    case EVertexFormat::_ULong: return sizeof(u64) * arity;
    case EVertexFormat::_Half: return sizeof(u16) * arity;
    case EVertexFormat::_Float: return sizeof(float) * arity;
    case EVertexFormat::_Double: return sizeof(double) * arity;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
