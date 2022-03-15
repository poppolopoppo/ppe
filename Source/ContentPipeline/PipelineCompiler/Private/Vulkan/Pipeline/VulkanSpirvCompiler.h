#pragma once

#include "PipelineCompiler_fwd.h"

#include "RHIVulkan_fwd.h"

#include "RHI/PipelineDesc.h"
#include "RHI/ShaderEnums.h"

#include "Container/Vector.h"
#include "IO/Dirpath.h"
#include "IO/StringBuilder.h"

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

    void SetCompilationFlags(EVulkanShaderCompilationFlags flags);
    void SetShaderClockFeatures(bool shaderSubgroupClock, bool shaderDeviceClock);
    void SetShaderFeatures(bool vertexPipelineStoresAndAtomics, bool fragmentStoresAndAtomic);

    void SetDefaultResourceLimits();
    void SetCurrentResourceLimits(const FVulkanDeviceInfo& deviceInfo);

#if USE_PPE_RHIDEBUG
    void SetShaderDebugFlags(EShaderLangFormat flags);
#endif

    NODISCARD bool Compile(
        FPipelineDesc::FShader* outShader,
        FShaderReflection* outReflection,
        FWStringBuilder* outLog,
        EShaderType shaderType,
        EShaderLangFormat srcShaderFormat,
        EShaderLangFormat dstShaderFormat,
        FConstChar entry,
        FConstChar source
        ARGS_IF_RHIDEBUG(FConstChar debugName) ) const;

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
        TMemoryView<const FConstChar> source,
        FIncludeResolver& resolver ) const;

    NODISCARD bool CompileSPIRV_(FRawData* outSPIRV, const FCompilationContext& ctx) const;

    NODISCARD bool BuildReflection_(FCompilationContext& ctx) const;
    NODISCARD bool ParseAnnotations_(const FCompilationContext& ctx, FStringView source) const;

    void OnCompilationFailed_(const FCompilationContext& ctx, TMemoryView<const FConstChar> source) const;
    NODISCARD bool CheckShaderFeatures_(EShaderType shaderType) const;

    static void GenerateDefaultResources_(TBuiltInResource* outResources) NOEXCEPT;

private: // GLSL deserializer
    NODISCARD bool ProcessExternalObjects_(const FCompilationContext& ctx, TIntermNode& node) const;
    NODISCARD bool DeserializeExternalObjects_(const FCompilationContext& ctx, TIntermNode& node) const;
    NODISCARD bool ProcessShaderInfos_(const FCompilationContext& ctx) const;
    NODISCARD bool CalculateStructSize_(u32* outStaticSize, u32* outArrayStride, u32* outMinOffset, const FCompilationContext& ctx, const glslang::TType& bufferType) const;

    void MergeWithGeometryInputPrimitive_(FShaderReflection::FTopologyBits* inoutTopology, glslang::TLayoutGeometry geometryType) const;

    NODISCARD static FBindingIndex ToBindingIndex_(const FCompilationContext& ctx, u32 index);
    NODISCARD static FPipelineDesc::FDescriptorSet& ToDescriptorSet_(const FCompilationContext& ctx, u32 index);

    NODISCARD static FStringView ExtractNodeName_(const TIntermNode* node);
    NODISCARD static FUniformID ExtractUniformID_(const TIntermNode* node) { return { ExtractNodeName_(node) }; }
    NODISCARD static FVertexID ExtractVertexID_(const TIntermNode* node) { return { ExtractNodeName_(node) }; }
    NODISCARD static FSpecializationID ExtractSpecializationID_(const TIntermNode* node) { return { ExtractNodeName_(node) }; }

    NODISCARD static FUniformID ExtractBufferUniformID_(const glslang::TType& type);
    NODISCARD static EImageSampler ExtractImageSampler_(const glslang::TType& type);
    NODISCARD static EVertexFormat ExtractVertexFormat_(const glslang::TType& type);
    NODISCARD static EFragmentOutput ExtractFragmentOutput_(const glslang::TType& type);
    NODISCARD static EResourceState ExtractShaderAccessType_(const glslang::TQualifier& q);

private:
    const FDirectories& _directories;
    EVulkanShaderCompilationFlags _compilationFlags{ Default };
    TBuiltInResource _builtInResources{};

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
