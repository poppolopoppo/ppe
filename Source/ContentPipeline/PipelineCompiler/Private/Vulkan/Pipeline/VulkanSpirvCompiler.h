#pragma once

#include "PipelineCompiler_fwd.h"

#include "RHIVulkan_fwd.h"

#include "RHI/PipelineCompiler.h"
#include "RHI/PipelineDesc.h"
#include "RHI/ShaderEnums.h"

#include "Container/Vector.h"
#include "IO/Dirpath.h"
#include "IO/StringBuilder.h"
#include "Misc/Function.h"
#include "Misc/Opaque.h"

#include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_core.h"

#include "glslang-external.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/MachineIndependent/localintermediate.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVulkanSpirvCompiler : Meta::FNonCopyable {
public:
    enum EShaderAnnotation : u32 {
        // ex: // @<annotation> (, @<annotation> ...) ]
        Set             = 1<<0, // @set <index> <name>
        WriteDiscard    = 1<<1, // @discard
        DynamicOffset   = 1<<2, // @dynamic-offset
        Unknown         = 0,
    };
    ENUM_FLAGS_FRIEND(EShaderAnnotation);

    struct FShaderReflection {
        using FShader = FPipelineDesc::FShader;
        using FPipelineLayout = FPipelineDesc::FPipelineLayout;
        using FFragmentOutputs = FPipelineDesc::FFragmentOutputs;
        using FSpecializationConstants = FPipelineDesc::FSpecializationConstants;
        using FTopologyBits = FGraphicsPipelineDesc::FTopologyBits;
        using FVertexAttributes = FGraphicsPipelineDesc::FVertexAttributes;

        FShader Shader{};
        FPipelineLayout Layout{};
        FSpecializationConstants Specializations{};

        struct {
            FTopologyBits SupportedTopology{};
            FVertexAttributes VertexAttributes{};
        }   Vertex;

        struct {
            u32 PatchControlPoints{};
        }   Tesselation;

        struct {
            FFragmentOutputs FragmentOutputs{};
            bool EarlyFragmentTests{ false };
        }   Fragment;

        struct {
            uint3 LocalGroupSize{};
            uint3 LocalGroupSpecialization{};
        }   Compute;

        struct {
            uint3 TaskGroupSize{};
            uint3 TaskGroupSpecialization{};

            uint3 MeshGroupSize{};
            uint3 MeshGroupSpecialization{};

            EPrimitiveTopology Topology{};
            u32 MaxPrimitives{ 0 };
            u32 MaxIndices{ 0 };
            u32 MaxVertices{ 0 };
        }   Mesh;
    };

    using FDirectories = VECTORINSITU(PipelineCompiler, FDirpath, 3);

    explicit FVulkanSpirvCompiler(const FDirectories& directories);
    ~FVulkanSpirvCompiler();

    EShaderCompilationFlags CompilationFlags() const {
        return _compilationFlags;
    }

    void SetCompilationFlags(EShaderCompilationFlags flags);
    void SetShaderClockFeatures(bool shaderSubgroupClock, bool shaderDeviceClock);
    void SetShaderFeatures(bool vertexPipelineStoresAndAtomics, bool fragmentStoresAndAtomic);

    void SetDefaultResourceLimits();
    void SetCurrentResourceLimits(const FVulkanDevice& device);

#if USE_PPE_RHIDEBUG
    void SetShaderDebugFlags(EShaderLangFormat flags);
#endif

    using FCompilationLogger = IPipelineCompiler::FLogger;

    NODISCARD bool Compile(
        FPipelineDesc::FShader* outShader,
        FShaderReflection* outReflection,
        const FCompilationLogger& log,
        EShaderType shaderType,
        EShaderLangFormat srcShaderFormat,
        EShaderLangFormat dstShaderFormat,
        FConstChar entry,
        const FSharedBuffer& content,
        FConstChar sourceFile ) const;

private:
    struct FGLSLangResult;
    class FIncludeResolver;
    struct FCompilationContext;

    NODISCARD bool ParseGLSL_(
        FCompilationContext& ctx,
        EShaderType shaderType,
        EShaderLangFormat srcShaderFormat,
        EShaderLangFormat dstShaderFormat,
        FConstChar entry,
        const FSharedBuffer& content,
        FConstChar sourceFile,
        FIncludeResolver& resolver ) const;

    NODISCARD bool CompileSPIRV_(FRawData* outSPIRV, const FCompilationContext& ctx) const;

    NODISCARD bool BuildReflection_(FCompilationContext& ctx) const;
    NODISCARD bool ParseAnnotations_(const FCompilationContext& ctx, FStringView content, FConstChar sourceFile) const;

    void OnCompilationFailed_(
        const FCompilationContext& ctx,
        const FConstChar compilerLog,
        const TMemoryView<const FConstChar> sourceFiles) const;
    NODISCARD bool CheckShaderFeatures_(EShaderType shaderType) const;

    static void GenerateDefaultResources_(TBuiltInResource* outResources) NOEXCEPT;

private: // GLSL deserializer
    NODISCARD bool ProcessExternalObjects_(const FCompilationContext& ctx, TIntermNode& node) const;
    NODISCARD bool DeserializeExternalObjects_(const FCompilationContext& ctx, TIntermNode& node) const;
    NODISCARD bool ProcessShaderInfos_(const FCompilationContext& ctx) const;
    NODISCARD bool CalculateStructSize_(u32* outStaticSize, u32* outArrayStride, u32* outMinOffset, const FCompilationContext& ctx, const glslang::TType& bufferType) const;

    void MergeWithGeometryInputPrimitive_(const FCompilationContext& ctx, FShaderReflection::FTopologyBits* inoutTopology, glslang::TLayoutGeometry geometryType) const;

    NODISCARD static FBindingIndex ToBindingIndex_(const FCompilationContext& ctx, u32 index);
    NODISCARD static FPipelineDesc::FDescriptorSet& ToDescriptorSet_(const FCompilationContext& ctx, u32 index);

    NODISCARD static FStringView ExtractNodeName_(const TIntermNode* node);
    NODISCARD static FUniformID ExtractUniformID_(const TIntermNode* node) { return FUniformID{ ExtractNodeName_(node) }; }
    NODISCARD static FVertexID ExtractVertexID_(const TIntermNode* node) { return FVertexID{ ExtractNodeName_(node) }; }
    NODISCARD static FSpecializationID ExtractSpecializationID_(const TIntermNode* node) { return FSpecializationID{ ExtractNodeName_(node) }; }

    NODISCARD static FUniformID ExtractBufferUniformID_(const glslang::TType& type);
    NODISCARD static EImageSampler ExtractImageSampler_(const FCompilationContext& ctx, const glslang::TType& type);
    NODISCARD static EVertexFormat ExtractVertexFormat_(const glslang::TType& type);
    NODISCARD static EFragmentOutput ExtractFragmentOutput_(const glslang::TType& type);
    NODISCARD static EResourceState ExtractShaderAccessType_(const glslang::TQualifier& q);

private:
    const FDirectories& _directories;
    TBuiltInResource _builtInResources{};
    FShaderDataFingerprint _glslangFingerprint;

    EShaderCompilationFlags _compilationFlags{ Default };

#if USE_PPE_RHIDEBUG
    EShaderLangFormat _debugFlags{ Default };
#endif

    struct {
        bool EnableShaderDeviceClock : 1;
        bool EnableShaderSubgroupClock : 1;
        bool EnableFragmentStoresAndAtomics : 1;
        bool EnableVertexPipelineStoresAndAtomics : 1;
    }   _features{};

};
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, FVulkanSpirvCompiler::EShaderAnnotation annotations);
FWTextWriter& operator <<(FWTextWriter& oss, FVulkanSpirvCompiler::EShaderAnnotation annotations);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
