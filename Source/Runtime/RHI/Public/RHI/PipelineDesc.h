#pragma once

#include "RHI_fwd.h"

#include "RHI/BindingIndex.h"
#include "RHI/RenderStateEnums.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceState.h"
#include "RHI/ShaderEnums.h"
#include "RHI/VertexInputState.h"

#include "Container/AssociativeVector.h"
#include "Container/FixedSizeHashTable.h"
#include "Container/HashMap.h"
#include "Container/Stack.h"
#include "IO/String.h"
#include "Meta/Utility.h"

#include <memory>
#include <variant>

#include "Container/Appendable.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FPipelineDescUniform {
    FUniformID Id;
    FBindingIndex Index;
    u32 ArraySize;
    EShaderStages StageFlags;

    FPipelineDescUniform() = default;
    FPipelineDescUniform(
        FUniformID id,
        FBindingIndex index,
        u32 arraySize,
        EShaderStages stageFlags ) NOEXCEPT
    :   Id(id)
    ,   Index(index)
    ,   ArraySize(arraySize)
    ,   StageFlags(stageFlags)
    {}
};
PPE_ASSUME_TYPE_AS_POD(FPipelineDescUniform);
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct TPipelineDescUniform : FPipelineDescUniform {
    STATIC_ASSERT(Meta::has_trivial_destructor<T>::value);
    T Data;

    TPipelineDescUniform() = default;

    TPipelineDescUniform(
        const FPipelineDescUniform& uniform,
        const T& data ) NOEXCEPT
    :   FPipelineDescUniform(uniform)
    ,   Data(data)
    {}

    TPipelineDescUniform(
        FUniformID id,
        FBindingIndex index,
        u32 arraySize,
        EShaderStages stageFlags,
        T&& rdata ) NOEXCEPT
    :   FPipelineDescUniform(id, index, arraySize, stageFlags)
    ,   Data(std::move(rdata))
    {}

    template <typename... _Args>
    TPipelineDescUniform(
        FUniformID id,
        FBindingIndex index,
        u32 arraySize,
        EShaderStages stageFlags,
        _Args&&... rargs ) NOEXCEPT
    :   FPipelineDescUniform(id, index, arraySize, stageFlags)
    ,   Data(std::forward<_Args>(rargs)...)
    {}
};
PPE_ASSUME_TEMPLATE_AS_POD(TPipelineDescUniform<T>, typename T);
} //!details
//----------------------------------------------------------------------------
template <typename T>
class IShaderData : public FRefCountable {
public:
    virtual ~IShaderData() = default;

    using FDataRef = Meta::TAddPointer<Meta::TAddConst<Meta::TRemovePointer<T>>>;

    virtual FDataRef Data() const NOEXCEPT = 0;
    virtual FStringView EntryPoint() const NOEXCEPT = 0;
    virtual hash_t HashValue() const NOEXCEPT = 0;

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT = 0;
    virtual bool ParseDebugOutput(TAppendable<FString> outp, EShaderDebugMode mode, FRawMemoryConst trace) = 0;
#endif

    friend hash_t hash_value(const IShaderData& data) {
        return data.HashValue();
    }
};
//----------------------------------------------------------------------------
using FSharedShaderString = TRefCountable<FString>;
using PSharedShaderString = TRefPtr<FSharedShaderString>;
using FShaderSource = std::variant<
    FString,
    FRawData,
    PSharedShaderString,
    PShaderModule
>;;
using PShaderSource = PShaderData< FShaderSource >;
//----------------------------------------------------------------------------
struct FPipelineDesc {
    STATIC_CONST_INTEGRAL(u32, StaticOffset, UMax);

    struct FTexture {
        EResourceState State{ Default };
        EImageSampler Type{ Default };

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
        EImageSampler Type{ Default };
        EPixelFormat Format{ Default };

        bool operator ==(const FImage& other) const { return (State == other.State && Type == other.Type && Format == other.Format); }
        bool operator !=(const FImage& other) const { return (not operator ==(other)); }
    };

    struct FUniformBuffer {
        EResourceState State{ Default };
        u32 DynamicOffsetIndex{ StaticOffset };
        u32 Size{ UMax };

        bool operator ==(const FUniformBuffer& other) const { return (State == other.State && DynamicOffsetIndex == other.DynamicOffsetIndex && Size == other.Size); }
        bool operator !=(const FUniformBuffer& other) const { return (not operator ==(other)); }
    };

    struct FStorageBuffer {
        EResourceState State{ Default };
        u32 DynamicOffsetIndex{ StaticOffset };
        u32 StaticSize{ UMax };
        u32 ArrayStride{ UMax };

        bool operator ==(const FStorageBuffer& other) const { return (State == other.State && DynamicOffsetIndex == other.DynamicOffsetIndex && StaticSize == other.StaticSize && ArrayStride == other.ArrayStride); }
        bool operator !=(const FStorageBuffer& other) const { return (not operator ==(other)); }
    };

    struct FRayTracingScene {
        EResourceState State{ Default };

        bool operator ==(const FRayTracingScene& other) const { return (State == other.State); }
        bool operator !=(const FRayTracingScene& other) const { return (not operator ==(other)); }
    };

    using FTextureUniform = details::TPipelineDescUniform<FTexture>;
    using FSamplerUniform = details::TPipelineDescUniform<FSampler>;
    using FSubpassInputUniform = details::TPipelineDescUniform<FSubpassInput>;
    using FImageUniform = details::TPipelineDescUniform<FImage>;
    using FUniformBufferUniform = details::TPipelineDescUniform<FUniformBuffer>;
    using FStorageBufferUniform = details::TPipelineDescUniform<FStorageBuffer>;
    using FRayTracingSceneUniform = details::TPipelineDescUniform<FRayTracingScene>;

    struct FPushConstant {
        FPushConstantID Id{ Default };
        EShaderStages StageFlags{ Default };
        u16 Offset{ UMax };
        u16 Size{ UMax };

        FPushConstant() = default;
        FPushConstant(FPushConstantID id, EShaderStages stageFlags, u32 offset, u32 size) NOEXCEPT
        :   Id(id)
        ,   StageFlags(stageFlags)
        ,   Offset(checked_cast<u16>(offset))
        ,   Size(checked_cast<u16>(size)) {
            STATIC_ASSERT(Meta::is_pod_v<FPushConstantID>);
        }
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
    PPE_ASSUME_FRIEND_AS_POD(FVariantResource)

    using FVariantUniform = details::TPipelineDescUniform<FVariantResource>;
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
        FDescriptorSets DescriptorSets;
        FPushConstants PushConstants;
    };

    using FShaderSourceMap = ASSOCIATIVE_VECTORINSITU(RHIPipeline, EShaderLangFormat, PShaderData<FShaderSource>, 2);
    using FSpecializationConstants = TFixedSizeHashMap<FSpecializationID, u32, MaxSpecializationConstants>;

    struct FShader {
        FShaderSourceMap Sources;
        FSpecializationConstants Specializations;

        FShader() = default;

        PPE_RHI_API void AddSource(EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
        PPE_RHI_API void AddSource(EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName));
        PPE_RHI_API void AddSource(EShaderLangFormat fmt, FStringView entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName));
        PPE_RHI_API void AddSource(EShaderLangFormat fmt, const PShaderModule& module);
        PPE_RHI_API void AddSource(EShaderLangFormat fmt, PShaderModule&& rmodule);
    };

    struct FFragmentOutput {
        u32 Index{ UMax };
        EFragmentOutput Type{ Default };

        FFragmentOutput() = default;
        FFragmentOutput(u32 index, EFragmentOutput type) : Index(index), Type(type) {}

        bool operator ==(const FFragmentOutput& other) const { return (Index == other.Index && Type == other.Type); }
        bool operator !=(const FFragmentOutput& other) const { return (not operator ==(other)); }
    };

    using FTopologyBits = TFixedSizeBitMask<size_t(EPrimitiveTopology::_Count)>;
    using FShaders = ASSOCIATIVE_VECTORINSITU(RHIPipeline, EShaderType, FShader, size_t(EShaderType::_Count));
    using FVertexAttributes = TFixedSizeStack<FVertexAttribute, MaxVertexAttribs>;
    using FFragmentOutputs = TFixedSizeStack<FFragmentOutput, MaxColorBuffers>;

    FPipelineLayout PipelineLayout;

protected:
    FPipelineDesc() = default;

    PPE_RHI_API void AddDescriptorSet_(
        const FDescriptorSetID& id,
        u32 index,
        TMemoryView<const FTextureUniform> textures,
        TMemoryView<const FSamplerUniform> samplers,
        TMemoryView<const FSubpassInputUniform> subpassInputs,
        TMemoryView<const FImageUniform> images,
        TMemoryView<const FUniformBufferUniform> uniformBuffers,
        TMemoryView<const FStorageBufferUniform> storageBuffers,
        TMemoryView<const FRayTracingSceneUniform> rayTracingScenes );

    PPE_RHI_API void SetPushConstants_(TMemoryView<const FPushConstant> values);

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

    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);

    FGraphicsPipelineDesc& AddTopology(EPrimitiveTopology topology) { SupportedTopology.SetTrue(static_cast<u32>(topology)); return (*this); }

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
    FGraphicsPipelineDesc& SetVertexAttributes(TMemoryView<const FVertexAttribute> attributes) { VertexAttributes.Assign(attributes); return (*this); }
    FGraphicsPipelineDesc& SetEarlyFragmentTests(bool enabled) { EarlyFragmentTests = enabled; return (*this); }
    FGraphicsPipelineDesc& SetPushConstants(TMemoryView<const FPushConstant> values) { SetPushConstants_(values); return (*this); }
    PPE_RHI_API FGraphicsPipelineDesc& SetSpecializationConstants(EShaderType type, TMemoryView<const FSpecializationConstant> values);

};
//----------------------------------------------------------------------------
// Compute
//----------------------------------------------------------------------------
struct FComputePipelineDesc final : FPipelineDesc {

    STATIC_CONST_INTEGRAL(u32, UndefinedSpecialization, UMax);

    FShader Shader;
    uint3 DefaultLocalGroupSize{ 0 };
    uint3 LocalSizeSpec{ UndefinedSpecialization };

    FComputePipelineDesc() = default;

    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, FStringView entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, const PShaderModule& module);

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
    uint3 TaskSizeSpec{ UndefinedSpecialization };
    uint3 DefaultMeshGroupSize{ 0 };
    uint3 MeshSizeSpec{ UndefinedSpecialization };
    bool EarlyFragmentTests{ true };

    FMeshPipelineDesc() = default;

    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FStringView entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName));
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

    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FStringView entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FStringView entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FStringView entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName));
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
