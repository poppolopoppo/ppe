#pragma once

#include "RHI_fwd.h"

#include "RHI/BindingIndex.h"
#include "RHI/RenderStateEnums.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceState.h"
#include "RHI/ShaderEnums.h"
#include "RHI/VertexInputState.h"

#include "Container/FixedSizeHashTable.h"
#include "Container/HashMap.h"
#include "Container/Stack.h"
#include "IO/String.h"
#include "Meta/Utility.h"

#include <memory>
#include <variant>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FPipelineBaseUniform {
    const FUniformID Id;
    const FBindingIndex Index;
    const u32 ArraySize;
    const EShaderStages StageFlags;

    FPipelineBaseUniform(
        FUniformID id,
        FBindingIndex index,
        u32 arraySize,
        EShaderStages stageFlags )
    :   Id(id)
    ,   Index(index)
    ,   ArraySize(arraySize)
    ,   StageFlags(stageFlags)
    {}
};
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct TPipelineUniform : FPipelineBaseUniform {

    const T Data;

    TPipelineUniform(
        FUniformID id,
        FBindingIndex index,
        u32 arraySize,
        EShaderStages stageFlags,
        T&& rdata )
    :   FPipelineBaseUniform(id, index, arraySize, stageFlags)
    ,   Data(std::move(rdata))
    {}

    template <typename... _Args>
    TPipelineUniform(
        FUniformID id,
        FBindingIndex index,
        u32 arraySize,
        EShaderStages stageFlags,
        _Args&&... rargs )
    :   FPipelineBaseUniform(id, index, arraySize, stageFlags)
    ,   Data(std::forward<_Args>(rargs)...)
    {}
};
} //!details
//----------------------------------------------------------------------------
template <typename T>
class IShaderData : public FRefCountable {
public:
    virtual ~IShaderData() = default;

    virtual const T& Data() const = 0;
    virtual FStringView Entry() const = 0;
    virtual hash_t HashValue() const = 0;

#if USE_PPE_RHIDEBUG
    virtual FStringView DebugName() const = 0;
#endif

    friend hash_t hash_value(const IShaderData& data) {
        return data.HashValue();
    }
};
//----------------------------------------------------------------------------
struct FShaderSource {
    using union_type = std::variant<
        FString,
        FRawData,
        PShaderModule
    >;

    union_type Data;
    u128 Fingerprint;
};
//----------------------------------------------------------------------------
struct FPipelineDesc {
    STATIC_CONST_INTEGRAL(u32, StaticOffset, UMax);

    struct FTexture {
        EResourceState State{ Default };
        EImageType Type{ Default };

        bool operator ==(const FTexture& other) const { return (State == other.State && Type == other.Type); }
        bool operator !=(const FTexture& other) const { return (not operator ==(other)); }
    };

    struct FSampler {
        CONSTEXPR bool operator ==(const FSampler&) const { return true; }
        CONSTEXPR bool operator !=(const FSampler&) const { return false; }
    };

    struct FSubpassInput {
        EResourceState State{ Default };
        u32 AttachmentIndex{ UMax };
        bool IsMultiSample{ false };

        bool operator ==(const FSubpassInput& other) const { return (State == other.State && AttachmentIndex == other.AttachmentIndex && IsMultiSample == other.IsMultiSample); }
        bool operator !=(const FSubpassInput& other) const { return (not operator ==(other)); }
    };

    struct FImage {
        EResourceState State{ Default };
        EImageType Type{ Default };
        EPixelFormat Format{ Default };

        bool operator ==(const FImage& other) const { return (State == other.State && Type == other.Type && Format == other.Format); }
        bool operator !=(const FImage& other) const { return (not operator ==(other)); }
    };

    struct FUniformBuffer {
        EResourceState State{ Default };
        u32 DynamicOffsetIndex{ StaticOffset };
        size_t Size{ UMax };

        bool operator ==(const FUniformBuffer& other) const { return (State == other.State && DynamicOffsetIndex == other.DynamicOffsetIndex && Size == other.Size); }
        bool operator !=(const FUniformBuffer& other) const { return (not operator ==(other)); }
    };

    struct FStorageBuffer {
        EResourceState State{ Default };
        u32 DynamicOffsetIndex{ StaticOffset };
        size_t StaticSize{ UMax };
        size_t ArrayStride{ UMax };

        bool operator ==(const FStorageBuffer& other) const { return (State == other.State && DynamicOffsetIndex == other.DynamicOffsetIndex && StaticSize == other.StaticSize && ArrayStride == other.ArrayStride); }
        bool operator !=(const FStorageBuffer& other) const { return (not operator ==(other)); }
    };

    struct FRayTracingScene {
        EResourceState State{ Default };

        bool operator ==(const FRayTracingScene& other) const { return (State == other.State); }
        bool operator !=(const FRayTracingScene& other) const { return (not operator ==(other)); }
    };

    using FTextureUniform = details::TPipelineUniform<FTexture>;
    using FSamplerUniform = details::TPipelineUniform<FSampler>;
    using FSubpassInputUniform = details::TPipelineUniform<FSubpassInput>;
    using FImageUniform = details::TPipelineUniform<FImage>;
    using FUniformBufferUniform = details::TPipelineUniform<FUniformBuffer>;
    using FStorageBufferUniform = details::TPipelineUniform<FStorageBuffer>;
    using FRayTracingSceneUniform = details::TPipelineUniform<FRayTracingScene>;

    struct FPushConstant {
        FPushConstantID Id{ Default };
        EShaderStages StageFlags{ Default };
        u16 Offset{ UMax };
        u16 Size{ UMax };

        FPushConstant() = default;
        FPushConstant(FPushConstantID id, EShaderStages stageFlags, size_t offset, size_t size)
        :   Id(id)
        ,   StageFlags(stageFlags)
        ,   Offset(checked_cast<u16>(offset))
        ,   Size(checked_cast<u16>(size))
        {}
    };

    struct FSpecializationConstant {
        FSpecializationID Id{ Default };
        u32 Index{ UMax };
    };

    using FVariantResource = std::variant<
        FTexture,
        FSampler,
        FSubpassInput,
        FImage,
        FUniformBuffer,
        FStorageBuffer,
        FRayTracingScene >;

    using FVariantUniform = details::TPipelineUniform<FVariantResource>;
    using FUniformMap = TRefCountable<TFixedSizeHashMap<FUniformID, FVariantUniform, MaxUniforms>>;
    using FSharedUniformMap = TRefPtr<FUniformMap>; // #TODO : replace std::shared_ptr by something else

    struct FDescriptorSet {
        FDescriptorSetID Id{ Default };
        u32 BindingIndex = UMax;
        FSharedUniformMap Uniforms;
    };

    using FDescriptorSets = TFixedSizeStack<FDescriptorSet, MaxDescriptorSets>;
    using FPushConstants = TFixedSizeHashMap<FPushConstantID, FPushConstant, MaxPushConstantsCount>;

    struct FPipelineLayout {
        FDescriptorSet DescriptorSets;
        FPushConstants PushConstants;
    };

    using FShaderSourceMap = TFixedSizeHashMap<EShaderLangFormat, PShaderData<FShaderSource>, 2>;
    using FSpecializationConstants = TFixedSizeHashMap<FSpecializationID, u32, MaxSpecializationConstants>;

    struct FShader {
        FShaderSourceMap Sources;
        FSpecializationConstants Specializations;

        FShader() = default;

        void AddSource(EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(const FStringView& name));
        void AddSource(EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(const FStringView& name));
        void AddSource(EShaderLangFormat fmt, const PShaderModule& module);
    };

    struct FFragmentOutput {
        u32 Index{ UMax };
        EFragmentOutput Type{ Default };

        FFragmentOutput() = default;
        FFragmentOutput(u32 index, EFragmentOutput type) : Index(index), Type(type) {}

        bool operator ==(const FFragmentOutput& other) const { return (Index == other.Index && Type == other.Type); }
        bool operator !=(const FFragmentOutput& other) const { return (not operator ==(other)); }
    };

    using FTopologyBits = Meta::TStaticBitset<size_t(EPrimitiveTopology::_Count)>;
    using FShaders = TFixedSizeHashMap<EShaderType, FShader, size_t(EShaderType::_Count)>;
    using FVertexAttributes = TFixedSizeStack<FVertexAttribute, MaxVertexAttribs>;
    using FFragmentOutputs = TFixedSizeStack<FFragmentOutput, MaxColorBuffers>;

    FPipelineLayout PipelineLayout;

protected:
    FPipelineDesc() = default;

    void AddDescriptorSet_(
        const FDescriptorSetID& id,
        u32 index,
        TMemoryView<const FTextureUniform> textures,
        TMemoryView<const FSamplerUniform> samplers,
        TMemoryView<const FSubpassInputUniform> subpassInputs,
        TMemoryView<const FImageUniform> images,
        TMemoryView<const FUniformBufferUniform> uniformBuffers,
        TMemoryView<const FStorageBufferUniform> storageBuffers,
        TMemoryView<const FRayTracingSceneUniform> rayTracingScenes );

    void SetPushConstants_(TMemoryView<const FPushConstant> values);

};
//----------------------------------------------------------------------------
// Graphics
//----------------------------------------------------------------------------
struct FGraphicsPipelineDesc final : FPipelineDesc {

    FShaders Shaders;
    FTopologyBits SupportedTopology;
    FFragmentOutputs FragmentOutputs;
    FVertexAttributes VertexAttributes;
    u32 PatchControlPoints{ 0 };
    bool EarlyFragmentTests{ true };

    FGraphicsPipelineDesc() = default;

    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);

    FGraphicsPipelineDesc& AddTopology(EPrimitiveTopology topology) { SupportedTopology.set(size_t(topology)); return (*this); }

    FGraphicsPipelineDesc& AddDescriptorSet(
        const FDescriptorSetID& id,
        u32 index,
        TMemoryView<const FTextureUniform> textures,
        TMemoryView<const FSamplerUniform> samplers,
        TMemoryView<const FSubpassInputUniform> subpassInputs,
        TMemoryView<const FImageUniform> images,
        TMemoryView<const FUniformBufferUniform> uniformBuffers,
        TMemoryView<const FStorageBufferUniform> storageBuffers ) {
        AddDescriptorSet_(id, index, textures, samplers, subpassInputs, images, uniformBuffers, storageBuffers, Default);
        return (*this);
    }

    FGraphicsPipelineDesc& SetFragmentOutputs(TMemoryView<const FFragmentOutput> outputs) { FragmentOutputs.Assign(outputs); return (*this); }
    FGraphicsPipelineDesc& SetVertexAttribytes(TMemoryView<const FVertexAttribute> attributes) { VertexAttributes.Assign(attributes); return (*this); }
    FGraphicsPipelineDesc& SetEarlyFragmentTests(bool enabled) { EarlyFragmentTests = enabled; return (*this); }
    PPE_RHI_API FGraphicsPipelineDesc& SetPushConstants(TMemoryView<const FPushConstant> values);

};
//----------------------------------------------------------------------------
// Compute
//----------------------------------------------------------------------------
struct FComputePipelineDesc final : FPipelineDesc {

    STATIC_CONST_INTEGRAL(u32, UndefinedSpecialization, UMax);

    FShader Shader;
    uint3 DefaultLocalGroupSize{ 0 };
    uint3 LocalSizeSpec{ 0 };

    FComputePipelineDesc() = default;

    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);

    FComputePipelineDesc& AddDescriptorSet(
        const FDescriptorSetID& id,
        u32 index,
        TMemoryView<const FTextureUniform> textures,
        TMemoryView<const FSamplerUniform> samplers,
        TMemoryView<const FSubpassInputUniform> subpassInputs,
        TMemoryView<const FImageUniform> images,
        TMemoryView<const FUniformBufferUniform> uniformBuffers,
        TMemoryView<const FStorageBufferUniform> storageBuffers ) {
        AddDescriptorSet_(id, index, textures, samplers, subpassInputs, images, uniformBuffers, storageBuffers, Default);
        return (*this);
    }

    FComputePipelineDesc& SetLocalGroupSize(u32 x, u32 y, u32 z) { DefaultLocalGroupSize = {x,y,z}; return (*this); }
    FComputePipelineDesc& SetLocalGroupSpecialization(u32 x = UndefinedSpecialization, u32 y = UndefinedSpecialization, u32 z = UndefinedSpecialization) { LocalSizeSpec = {x,y,z}; return (*this); }
    FComputePipelineDesc& SetPushConstants(TMemoryView<const FPushConstant> values) { SetPushConstants_(values); return (*this); }
    PPE_RHI_API FComputePipelineDesc& SetSpecializationConstants(TMemoryView<const FSpecializationConstant> values);

};
//----------------------------------------------------------------------------
// Mesh
//----------------------------------------------------------------------------
struct FMeshPipelineDesc final : FPipelineDesc {

    STATIC_CONST_INTEGRAL(u32, UndefinedSpecialization, UMax);

    FShaders Shaders;
    EPrimitiveTopology Topology{ Default };
    FFragmentOutputs FragmentOutputs;
    u32 MaxVertices{ 0 };
    u32 MaxIndices{ 0 };
    uint3 DefaultTaskGroupSize{ 0 };
    uint3 TaskSizeSpec{ 0 };
    uint3 DefaultMeshGroupSize{ 0 };
    uint3 MeshSizeSpec{ 0 };
    bool EarlyFragmentTests{ true };

    FMeshPipelineDesc() = default;

    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);

    FMeshPipelineDesc& AddDescriptorSet(
        const FDescriptorSetID& id,
        u32 index,
        TMemoryView<const FTextureUniform> textures,
        TMemoryView<const FSamplerUniform> samplers,
        TMemoryView<const FSubpassInputUniform> subpassInputs,
        TMemoryView<const FImageUniform> images,
        TMemoryView<const FUniformBufferUniform> uniformBuffers,
        TMemoryView<const FStorageBufferUniform> storageBuffers ) {
        AddDescriptorSet_(id, index, textures, samplers, subpassInputs, images, uniformBuffers, storageBuffers, Default);
        return (*this);
    }

    FMeshPipelineDesc& SetTopology(EPrimitiveTopology topology) { Topology = topology; return (*this); }
    FMeshPipelineDesc& SetPushConstants(TMemoryView<const FPushConstant> values) { SetPushConstants_(values); return (*this); }
    FMeshPipelineDesc& SetFragmentOutpyts(TMemoryView<const FFragmentOutput> outputs) { FragmentOutputs.Assign(outputs); return (*this); }
    FMeshPipelineDesc& SetEarlyFragmentTests(bool value) { EarlyFragmentTests = value; return (*this); }
    PPE_RHI_API FMeshPipelineDesc& SetSpecializationConstants(EShaderType type, TMemoryView<const FSpecializationConstant> values);

};
//----------------------------------------------------------------------------
// RayTracing
//----------------------------------------------------------------------------
struct FRayTracingPipelineDesc final : FPipelineDesc {

    struct FRTShader final : FShader {
        EShaderType Type{ Default };
    };

    HASHMAP(RHIPipeline, FRTShaderID, FRTShader) Shaders;

    FRayTracingPipelineDesc() = default;

    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FStringView entry, PShaderModule shader ARGS_IF_RHIDEBUG(const FStringView& name));

    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(const FStringView& name));
    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);

    FRayTracingPipelineDesc& AddDescriptorSet(
        const FDescriptorSetID& id,
        u32 index,
        TMemoryView<const FTextureUniform> textures,
        TMemoryView<const FSamplerUniform> samplers,
        TMemoryView<const FSubpassInputUniform> subpassInputs,
        TMemoryView<const FImageUniform> images,
        TMemoryView<const FUniformBufferUniform> uniformBuffers,
        TMemoryView<const FStorageBufferUniform> storageBuffers,
        TMemoryView<const FRayTracingSceneUniform> rayTracingScenes ) {
        AddDescriptorSet_(id, index, textures, samplers, subpassInputs, images, uniformBuffers, storageBuffers, rayTracingScenes);
        return (*this);
    }

    FRayTracingPipelineDesc& SetPushConstants(TMemoryView<const FPushConstant> values) { SetPushConstants_(values); return (*this); }
    PPE_RHI_API FRayTracingPipelineDesc& SetSpecializationConstants(const FRTShaderID& id, TMemoryView<const FSpecializationConstant> values);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
