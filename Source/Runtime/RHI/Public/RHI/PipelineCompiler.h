#pragma once

#include "RHI_fwd.h"

#include "HAL/TargetRHI.h"
#include "Misc/Event.h"
#include "Misc/Function_fwd.h"
#include "Misc/Opaque_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EShaderCompilationFlags : u32
{
    Quiet                       = 1 << 8,
    //KeepSrcShaderData         = 1 << 9,   // compiler will keep incoming GLSL source and adds SPIRV or VkShaderModule

    UseCurrentDeviceLimits      = 1 << 10,  // get current device properties and use it to setup spirv compiler

    GenerateDebug               = 1 << 15,
    Optimize                    = 1 << 16,
    OptimizeSize                = 1 << 17,
    StrongOptimization          = 1 << 18, // very slow, may be usable for off-line compilation
    Validate                    = 1 << 19,

    ParseAnnotations            = 1 << 20,

    _Last,
    Unknown = 0,
};
ENUM_FLAGS(EShaderCompilationFlags);
//----------------------------------------------------------------------------
class PPE_RHI_API IPipelineCompiler : public FRefCountable {
public:
    virtual ~IPipelineCompiler() = default;

    using FLogger = TFunction<void(
        ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringView& text,
        Opaq::object_init&& object)>;

    NODISCARD virtual FStringLiteral DisplayName() const NOEXCEPT = 0;
    NODISCARD virtual const FLogger& DefaultLogger() const NOEXCEPT = 0;
    NODISCARD virtual ETargetRHI TargetRHI() const NOEXCEPT = 0;

    NODISCARD virtual EShaderCompilationFlags CompilationFlags() const NOEXCEPT = 0;
    virtual void SetCompilationFlags(EShaderCompilationFlags value) NOEXCEPT = 0;

    NODISCARD virtual bool IsSupported(const FMeshPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;
    NODISCARD virtual bool IsSupported(const FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;
    NODISCARD virtual bool IsSupported(const FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;
    NODISCARD virtual bool IsSupported(const FComputePipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;

    NODISCARD virtual bool Compile(FMeshPipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) = 0;
    NODISCARD virtual bool Compile(FRayTracingPipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) = 0;
    NODISCARD virtual bool Compile(FGraphicsPipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) = 0;
    NODISCARD virtual bool Compile(FComputePipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) = 0;

    NODISCARD inline bool Compile(FMeshPipelineDesc& desc, EShaderLangFormat fmt) { return Compile(desc, fmt, DefaultLogger()); }
    NODISCARD inline bool Compile(FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) { return Compile(desc, fmt, DefaultLogger()); }
    NODISCARD inline bool Compile(FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) { return Compile(desc, fmt, DefaultLogger()); }
    NODISCARD inline bool Compile(FComputePipelineDesc& desc, EShaderLangFormat fmt) { return Compile(desc, fmt, DefaultLogger()); }

    virtual void ReleaseUnusedMemory() = 0;

    enum ECompilationResult : u8 {
        Success = 0,
        Failed,
        NotSupported
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
