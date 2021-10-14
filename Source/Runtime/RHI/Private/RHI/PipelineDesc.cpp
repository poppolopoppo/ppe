#include "stdafx.h"

#include "RHI/PipelineDesc.h"

#include "RHI/EnumHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
class TShaderDataImpl_ final : public IShaderData<T> {
public:
    using typename IShaderData<T>::FDataRef;

    TShaderDataImpl_(T&& rdata, FConstChar entryPoint ARGS_IF_RHIDEBUG(FConstChar debugName)) NOEXCEPT
    :   _data(std::move(rdata))
    ,   _entryPoint(entryPoint)
    ARGS_IF_RHIDEBUG(_debugName(debugName))
    {}

    virtual FDataRef Data() const NOEXCEPT override { return std::addressof(_data); }
    virtual FConstChar EntryPoint() const NOEXCEPT override { return _entryPoint; }
    virtual hash_t HashValue() const NOEXCEPT override { AssertNotReached(); }

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT override { return _debugName; }
    virtual bool ParseDebugOutput(TAppendable<FString> , EShaderDebugMode , FRawMemoryConst ) override {
        return false;
    }
#endif

private:
    T _data;
    FConstChar _entryPoint;
#if USE_PPE_DEBUG
    const FConstChar _debugName;
#endif
};
//----------------------------------------------------------------------------
void CopySpecializationConstants_(FPipelineDesc::FSpecializationConstants *pDst, TMemoryView<const FPipelineDesc::FSpecializationConstant> src) {
    Assert(pDst);
    for (const FPipelineDesc::FSpecializationConstant& it : src) {
        Assert_NoAssume(it.Id.Valid());
        pDst->Emplace_Overwrite(it.Id, static_cast<u32>(it.Index));
    }
}
//----------------------------------------------------------------------------
void SetSpecializationConstants_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType, TMemoryView<const FPipelineDesc::FSpecializationConstant> values) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(not shader.Sources.empty());
    Assert_NoAssume(shader.Specializations.empty());
    CopySpecializationConstants_(&shader.Specializations, values);
}
//----------------------------------------------------------------------------
void AddShaderSource_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Sources.end() == shader.Sources.find(lang));
    shader.AddSource(lang, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
}
//----------------------------------------------------------------------------
void AddShaderSource_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, FConstChar entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Sources.end() == shader.Sources.find(lang));
    shader.AddSource(lang, entry, std::move(rbinary) ARGS_IF_RHIDEBUG(debugName));
}
//----------------------------------------------------------------------------
void AddShaderSource_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, FConstChar entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(pShaderMap);
    Assert(sharedSource);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Sources.end() == shader.Sources.find(lang));
    shader.AddSource(lang, entry, sharedSource ARGS_IF_RHIDEBUG(debugName));
}
//----------------------------------------------------------------------------
void AddShaderSource_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, const PShaderModule& module ) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Sources.end() == shader.Sources.find(lang));
    shader.AddSource(lang, module);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddSource(EShaderLangFormat fmt, const PShaderModule& module) {
    Assert(module);
    Sources.GetOrAdd(fmt) = NEW_REF(RHIPipeline, TShaderDataImpl_<FShaderSource>,
        module, module->EntryPoint() ARGS_IF_RHIDEBUG(module->DebugName()) );
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddSource(EShaderLangFormat fmt, PShaderModule&& rmodule) {
    Assert(rmodule);
    const FConstChar entryPoint = rmodule->EntryPoint();
    ONLY_IF_RHIDEBUG(const FConstChar debugName = rmodule->DebugName());
    Sources.GetOrAdd(fmt) = NEW_REF(RHIPipeline, TShaderDataImpl_<FShaderSource>,
        std::move(rmodule), entryPoint ARGS_IF_RHIDEBUG(debugName) );
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddSource(EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(entry);
    Assert(not rsource.empty());
    Sources.GetOrAdd(fmt) = NEW_REF(RHIPipeline, TShaderDataImpl_<FShaderSource>,
        std::move(rsource), entry ARGS_IF_RHIDEBUG(debugName) );
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddSource(EShaderLangFormat fmt, FConstChar entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(entry);
    Assert(sharedSource);
    Sources.GetOrAdd(fmt) = NEW_REF(RHIPipeline, TShaderDataImpl_<FShaderSource>,
        sharedSource, entry ARGS_IF_RHIDEBUG(debugName) );
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddSource(EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(entry);
    Assert(not rbinary.empty());
    Sources.GetOrAdd(fmt) = NEW_REF(RHIPipeline, TShaderDataImpl_<FShaderSource>,
        std::move(rbinary), entry ARGS_IF_RHIDEBUG(debugName) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FPipelineDesc::AddDescriptorSet_(
    const FDescriptorSetID& id, u32 index,
    TMemoryView<const FTextureUniform> textures,
    TMemoryView<const FSamplerUniform> samplers,
    TMemoryView<const FSubpassInputUniform> subpassInputs,
    TMemoryView<const FImageUniform> images,
    TMemoryView<const FUniformBufferUniform> uniformBuffers,
    TMemoryView<const FStorageBufferUniform> storageBuffers,
    TMemoryView<const FRayTracingSceneUniform> rayTracingScenes ) {
    Assert(id.Valid());
    Assert(index < MaxDescriptorSets);

#if USE_PPE_RHIDEBUG
    for (const FDescriptorSet& it : PipelineLayout.DescriptorSets) {
        Assert_NoAssume(it.Id != id);
        Assert_NoAssume(it.BindingIndex != index);
    }
#endif

    FUniformMap uniforms;
    uniforms.reserve(
        textures.size() +
        samplers.size() +
        subpassInputs.size() +
        images.size() +
        uniformBuffers.size() +
        storageBuffers.size() +
        rayTracingScenes.size() );

    for (const auto& it : textures)
        uniforms.Emplace_Overwrite(FUniformID{ it.Id }, FVariantUniform{ it, FVariantResource{ it.Data } });
    for (const auto& it : samplers)
        uniforms.Emplace_Overwrite(FUniformID{ it.Id }, FVariantUniform{ it, FVariantResource{ it.Data } });
    for (const auto& it : subpassInputs)
        uniforms.Emplace_Overwrite(FUniformID{ it.Id }, FVariantUniform{ it, FVariantResource{ it.Data } });
    for (const auto& it : images)
        uniforms.Emplace_Overwrite(FUniformID{ it.Id }, FVariantUniform{ it, FVariantResource{ it.Data } });
    for (const auto& it : uniformBuffers)
        uniforms.Emplace_Overwrite(FUniformID{ it.Id }, FVariantUniform{ it, FVariantResource{ it.Data } });
    for (const auto& it : storageBuffers)
        uniforms.Emplace_Overwrite(FUniformID{ it.Id }, FVariantUniform{ it, FVariantResource{ it.Data } });
    for (const auto& it : rayTracingScenes)
        uniforms.Emplace_Overwrite(FUniformID{ it.Id }, FVariantUniform{ it, FVariantResource{ it.Data } });

    PipelineLayout.DescriptorSets.Push(id, index, NEW_REF(RHIPipeline, FUniformMap, std::move(uniforms)) );
}
//----------------------------------------------------------------------------
void FPipelineDesc::SetPushConstants_(TMemoryView<const FPushConstant> values) {
    for (const FPushConstant& it : values)
        PipelineLayout.PushConstants.Emplace_Overwrite(it.Id, it);
}
//----------------------------------------------------------------------------
// Graphics
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderSource_(&Shaders, type, fmt, module);
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderSource_(&Shaders, type, fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderSource_(&Shaders, type, fmt, entry, std::move(rbinary) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderSource_(&Shaders, type, fmt, entry, sharedSource ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::SetSpecializationConstants(EShaderType type, TMemoryView<const FSpecializationConstant> values) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    SetSpecializationConstants_(&Shaders, type, values);
    return (*this);
}
//----------------------------------------------------------------------------
// Compute
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, const PShaderModule& module) {
    Assert_NoAssume(Shader.Sources.end() == Shader.Sources.find(fmt));
    Shader.AddSource(fmt, module);
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(Shader.Sources.end() == Shader.Sources.find(fmt));
    Shader.AddSource(fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(Shader.Sources.end() == Shader.Sources.find(fmt));
    Shader.AddSource(fmt, entry, std::move(rbinary) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, FConstChar entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(Shader.Sources.end() == Shader.Sources.find(fmt));
    Shader.AddSource(fmt, entry, sharedSource ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::SetSpecializationConstants(TMemoryView<const FSpecializationConstant> values) {
    Assert_NoAssume(not Shader.Sources.empty());
    Assert_NoAssume(Shader.Specializations.empty());
    CopySpecializationConstants_(&Shader.Specializations, values);
    return (*this);
}
//----------------------------------------------------------------------------
// Mesh
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderSource_(&Shaders, type, fmt, module);
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderSource_(&Shaders, type, fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderSource_(&Shaders, type, fmt, entry, std::move(rbinary) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderSource_(&Shaders, type, fmt, entry, sharedSource ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::SetSpecializationConstants(EShaderType type, TMemoryView<const FSpecializationConstant> values) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    SetSpecializationConstants_(&Shaders, type, values);
    return (*this);
}
//----------------------------------------------------------------------------
// RayTracing
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, const PShaderModule& module) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddSource(fmt, module);

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddSource(fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddSource(fmt, entry, std::move(rbinary) ARGS_IF_RHIDEBUG(debugName));

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, const PSharedShaderString& sharedSource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddSource(fmt, entry, sharedSource ARGS_IF_RHIDEBUG(debugName));

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::SetSpecializationConstants(const FRTShaderID& id, TMemoryView<const FSpecializationConstant> values) {
    Assert(id.Valid());

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    Assert_NoAssume(not rtShader.Sources.empty());
    Assert_NoAssume(not rtShader.Specializations.empty());
    CopySpecializationConstants_(&rtShader.Specializations, values);

    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
