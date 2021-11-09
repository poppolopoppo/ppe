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
    ,   StageFlags(stageFlags) {
        Assert(id.Valid());
    }

    bool operator ==(const FPipelineDescUniform& other) const NOEXCEPT {
        return (Id == other.Id && Index == other.Index && ArraySize == other.ArraySize && StageFlags == other.StageFlags);
    }
    bool operator !=(const FPipelineDescUniform& other) const NOEXCEPT {
        return (not operator ==(other));
    }
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

    bool operator ==(const TPipelineDescUniform& other) const NOEXCEPT {
        return (static_cast<const FPipelineDescUniform&>(*this) == other && Data == other.Data);
    }
    bool operator !=(const TPipelineDescUniform& other) const NOEXCEPT {
        return (not operator ==(other));
    }
};
PPE_ASSUME_TEMPLATE_AS_POD(TPipelineDescUniform<T>, typename T);
} //!details
struct FTextureUniform;
struct FSamplerUniform;
struct FSubpassInputUniform;
struct FImageUniform;
struct FUniformBufferUniform;
struct FStorageBufferUniform;
struct FRayTracingSceneUniform;
//----------------------------------------------------------------------------
template <typename T>
class IShaderData : public FRefCountable {
public:
    virtual ~IShaderData() = default;

    using FDataRef = Meta::TAddPointer<Meta::TAddConst<Meta::TRemovePointer<T>>>;
    using FFingerprint = FShaderDataFingerprint;

    virtual FDataRef Data() const NOEXCEPT = 0;
    virtual FConstChar EntryPoint() const NOEXCEPT = 0;
    virtual FFingerprint Fingerprint() const NOEXCEPT = 0;

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT = 0;
    virtual bool ParseDebugOutput(TAppendable<FString> outp, EShaderDebugMode mode, FRawMemoryConst trace) = 0;
#endif

    friend hash_t hash_value(const IShaderData& data) NOEXCEPT {
        return hash_value(data.Fingerprint());
    }

    friend hash_t hash_value(const TRefPtr<IShaderData>& dataRef) NOEXCEPT {
        return hash_value(*dataRef);
    }
};
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

        bool operator ==(const FImage& other) const { return (State == other.State && Type == other.Type); }
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

        friend hash_t hash_value(const FPushConstant& value) NOEXCEPT {
            return hash_tuple(value.Id, value.StageFlags, value.Offset, value.Size);
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
    using PUniformMap = TRefPtr<FUniformMap>;

    struct FDescriptorSet {
        FDescriptorSetID Id{ Default };
        u32 BindingIndex = UMax;
        PUniformMap Uniforms;
    };

    using FDescriptorSets = TFixedSizeStack<FDescriptorSet, MaxDescriptorSets>;
    using FPushConstants = TFixedSizeHashMap<FPushConstantID, FPushConstant, MaxPushConstantsCount>;

    struct FPipelineLayout {
        FDescriptorSets DescriptorSets;
        FPushConstants PushConstants;
    };

    using FShaderDataMap = ASSOCIATIVE_VECTORINSITU(RHIPipeline, EShaderLangFormat, FShaderDataVariant, 2);
    using FSpecializationConstants = TFixedSizeHashMap<FSpecializationID, u32, MaxSpecializationConstants>;

    struct FShader {
        FShaderDataMap Data;
        FSpecializationConstants Specializations;

        FShader() = default;

        PPE_RHI_API void AddShader(EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
        PPE_RHI_API void AddShader(EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName));
        PPE_RHI_API void AddShader(EShaderLangFormat fmt, PShaderModule&& rmodule);
        PPE_RHI_API void AddShader(EShaderLangFormat fmt, FShaderDataVariant&& rdata);
        void AddShader(EShaderLangFormat fmt, const PShaderModule& module) { AddShader(fmt, PShaderModule{ module }); }
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
// Uniforms
//----------------------------------------------------------------------------
struct FTextureUniform : details::TPipelineDescUniform<FPipelineDesc::FTexture> {
    using base_type = details::TPipelineDescUniform<FPipelineDesc::FTexture>;
    using base_type::operator==;
    using base_type::operator!=;

    PPE_RHI_API FTextureUniform(const FUniformID& id, EImageSampler textureType, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT;
};
//----------------------------------------------------------------------------
struct FSamplerUniform : details::TPipelineDescUniform<FPipelineDesc::FSampler> {
    using base_type = details::TPipelineDescUniform<FPipelineDesc::FSampler>;
    using base_type::operator==;
    using base_type::operator!=;

    PPE_RHI_API FSamplerUniform(const FUniformID& id, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT;
};
//----------------------------------------------------------------------------
struct FSubpassInputUniform : details::TPipelineDescUniform<FPipelineDesc::FSubpassInput> {
    using base_type = details::TPipelineDescUniform<FPipelineDesc::FSubpassInput>;
    using base_type::operator==;
    using base_type::operator!=;

    PPE_RHI_API FSubpassInputUniform(const FUniformID& id, u32 attachmentIndex, bool isMultisample, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT;
};
//----------------------------------------------------------------------------
struct FImageUniform : details::TPipelineDescUniform<FPipelineDesc::FImage> {
    using base_type = details::TPipelineDescUniform<FPipelineDesc::FImage>;
    using base_type::operator==;
    using base_type::operator!=;

    PPE_RHI_API FImageUniform(const FUniformID& id, EImageSampler imageType, EShaderAccess access, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT;
};
//----------------------------------------------------------------------------
struct FUniformBufferUniform : details::TPipelineDescUniform<FPipelineDesc::FUniformBuffer> {
    using base_type = details::TPipelineDescUniform<FPipelineDesc::FUniformBuffer>;
    using base_type::operator==;
    using base_type::operator!=;

    PPE_RHI_API FUniformBufferUniform(const FUniformID& id, u32 size, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags, u32 dynamicOffsetIndex = FPipelineDesc::StaticOffset) NOEXCEPT;
};
//----------------------------------------------------------------------------
struct FStorageBufferUniform : details::TPipelineDescUniform<FPipelineDesc::FStorageBuffer> {
    using base_type = details::TPipelineDescUniform<FPipelineDesc::FStorageBuffer>;
    using base_type::operator==;
    using base_type::operator!=;

    PPE_RHI_API FStorageBufferUniform(const FUniformID& id, u32 staticSize, u32 arrayStride, EShaderAccess access, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags, u32 dynamicOffsetIndex = FPipelineDesc::StaticOffset) NOEXCEPT;
};
//----------------------------------------------------------------------------
struct FRayTracingSceneUniform : details::TPipelineDescUniform<FPipelineDesc::FRayTracingScene> {
    using base_type = details::TPipelineDescUniform<FPipelineDesc::FRayTracingScene>;
    using base_type::operator==;
    using base_type::operator!=;

    PPE_RHI_API FRayTracingSceneUniform(const FUniformID& id, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT;
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

    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);
    PPE_RHI_API FGraphicsPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FShaderDataVariant&& rdata);

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
    uint3 LocalSizeSpecialization{ UndefinedSpecialization };

    FComputePipelineDesc() = default;

    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, const PShaderModule& module);
    PPE_RHI_API FComputePipelineDesc& AddShader(EShaderLangFormat fmt, FShaderDataVariant&& rdata);

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
    FComputePipelineDesc& SetLocalGroupSpecialization(u32 x = UndefinedSpecialization, u32 y = UndefinedSpecialization, u32 z = UndefinedSpecialization) { LocalSizeSpecialization = {x,y,z}; return (*this); }
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

    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);
    PPE_RHI_API FMeshPipelineDesc& AddShader(EShaderType type, EShaderLangFormat fmt, FShaderDataVariant&& rdata);

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

    using FShaders = ASSOCIATIVE_VECTORINSITU(RHIPipeline, FRTShaderID, FRTShader, size_t(EShaderType::_Count));

    FShaders Shaders;

    FRayTracingPipelineDesc() = default;

    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName));
    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, const PShaderModule& module);
    PPE_RHI_API FRayTracingPipelineDesc& AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FShaderDataVariant&& rdata);

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
