// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI/PipelineDesc.h"

#include "VirtualFileSystem_fwd.h"

#include "IO/Filename.h"
#include "IO/String.h"
#include "RHI/EnumHelpers.h"

#include "IO/StaticString.h"
#include "Memory/SharedBuffer.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
class TShaderDataImpl_ : public IShaderData<T> {
public:
    using typename IShaderData<T>::FDataRef;
    using typename IShaderData<T>::FFingerprint;

    TShaderDataImpl_(T&& rdata, FConstChar entryPoint, FFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName)) NOEXCEPT
    :   _data(std::move(rdata))
    ,   _entryPoint(entryPoint)
    ,   _fingerprint(fingerprint)
    ARGS_IF_RHIDEBUG(_debugName(debugName.c_str()))
    {}

    virtual FDataRef Data() const NOEXCEPT override final { return std::addressof(_data); }
    virtual FConstChar EntryPoint() const NOEXCEPT override final { return _entryPoint; }
    virtual FFingerprint Fingerprint() const NOEXCEPT override final { return _fingerprint; }

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT override final { return _debugName; }
    virtual bool ParseDebugOutput(TAppendable<FString> , EShaderDebugMode , FRawMemoryConst ) override final {
        return false;
    }
#endif

private:
    T _data;
    TStaticString<64> _entryPoint;
    FFingerprint _fingerprint;
#if USE_PPE_RHIDEBUG
    TStaticString<64> _debugName;
#endif
};
//----------------------------------------------------------------------------
template <>
class TShaderDataImpl_<IShaderSource> : public IShaderData<IShaderSource>, private IShaderSource {
public:
    using typename IShaderData<IShaderSource>::FDataRef;
    using typename IShaderData<IShaderSource>::FFingerprint;

    TShaderDataImpl_(FConstChar entryPoint, FFingerprint fingerprint ARGS_IF_RHIDEBUG(FString&& debugName)) NOEXCEPT
    :   _entryPoint(entryPoint)
    ,   _fingerprint(fingerprint)
    ARGS_IF_RHIDEBUG(_debugName(std::move(debugName)))
    {}

    virtual FDataRef Data() const NOEXCEPT override final { return this; }
    virtual FConstChar EntryPoint() const NOEXCEPT override final { return _entryPoint; }
    virtual FFingerprint Fingerprint() const NOEXCEPT override final { return _fingerprint; }

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT override final { return _debugName.c_str(); }
    virtual bool ParseDebugOutput(TAppendable<FString> , EShaderDebugMode , FRawMemoryConst ) override final {
        return false;
    }
#endif

private:
    TStaticString<64> _entryPoint;
    FFingerprint _fingerprint;
#if USE_PPE_RHIDEBUG
    FString _debugName;
#endif
};
//----------------------------------------------------------------------------
class FStringShaderSource_ final : public TShaderDataImpl_<IShaderSource> {
public:
    FString Content;

    FStringShaderSource_(FString&& content, FConstChar entryPoint, FFingerprint fingerprint ARGS_IF_RHIDEBUG(FString&& debugName)) NOEXCEPT
    :   TShaderDataImpl_(entryPoint, fingerprint ARGS_IF_RHIDEBUG(std::move(debugName)))
    ,   Content(std::move(content)) {
        Assert_NoAssume(not Content.empty());
    }

    FSharedBuffer LoadShaderSource() const NOEXCEPT override {
        return FSharedBuffer::MakeView(Content.data(), Content.SizeInBytes());
    }
};
//----------------------------------------------------------------------------
class FFileShaderSource_ final : public TShaderDataImpl_<IShaderSource> {
public:
    FFilename Filename;

    FFileShaderSource_(const FFilename& filename, FConstChar entryPoint, FFingerprint fingerprint) NOEXCEPT
    :   TShaderDataImpl_(entryPoint, fingerprint ARGS_IF_RHIDEBUG(ToString(VFS_Unalias(filename))))
    ,   Filename(filename) {
        Assert_NoAssume(not Filename.empty());
    }

    FSharedBuffer LoadShaderSource() const NOEXCEPT override {
        FUniqueBuffer content = VFS_ReadAll(Filename, EAccessPolicy::Binary);
        return content.MoveToShared();
    }
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
    Assert_NoAssume(not shader.Data.empty());
    Assert_NoAssume(shader.Specializations.empty());
    CopySpecializationConstants_(&shader.Specializations, values);
}
//----------------------------------------------------------------------------
void AddShaderData_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Data.end() == shader.Data.find(lang));
    shader.AddShader(lang, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
}
//----------------------------------------------------------------------------
void AddShaderData_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, FConstChar entry, const FFilename& sourceFile) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Data.end() == shader.Data.find(lang));
    shader.AddShader(lang, entry, sourceFile);
}
//----------------------------------------------------------------------------
void AddShaderData_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Data.end() == shader.Data.find(lang));
    shader.AddShader(lang, entry, std::move(rbinary), fingerprint ARGS_IF_RHIDEBUG(debugName));
}
//----------------------------------------------------------------------------
void AddShaderData_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, const PShaderModule& module ) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Data.end() == shader.Data.find(lang));
    shader.AddShader(lang, module);
}
//----------------------------------------------------------------------------
void AddShaderData_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    EShaderLangFormat lang, FShaderDataVariant&& rdata ) {
    Assert(pShaderMap);
    FPipelineDesc::FShader& shader = pShaderMap->FindOrAdd(shaderType);
    Assert_NoAssume(shader.Data.end() == shader.Data.find(lang));
    shader.AddShader(lang, std::move(rdata));
}
//----------------------------------------------------------------------------
void AddShaderData_(FPipelineDesc::FShaders* pShaderMap, EShaderType shaderType,
    const FPipelineDesc::FShader& shader ) {
    Assert(pShaderMap);
    pShaderMap->Emplace_AssertUnique(shaderType, shader);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPipelineDesc::FPipelineDesc() NOEXCEPT = default;
FPipelineDesc::~FPipelineDesc() = default;
//----------------------------------------------------------------------------
FPipelineDesc::FPipelineDesc(const FPipelineDesc&) = default;
//----------------------------------------------------------------------------
FPipelineDesc::FPipelineDesc(FPipelineDesc&&) NOEXCEPT = default;
FPipelineDesc& FPipelineDesc::operator =(FPipelineDesc&&) NOEXCEPT = default;
//----------------------------------------------------------------------------
FPipelineDesc::FShader::FShader() NOEXCEPT = default;
FPipelineDesc::FShader::~FShader() = default;
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddShader(EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(entry);
    Assert(not rsource.empty());
    const auto fingerprint = Fingerprint128(rsource.MakeView());
    PShaderSource shader = NEW_REF(RHIPipeline, FStringShaderSource_,
        std::move(rsource), entry, fingerprint ARGS_IF_RHIDEBUG(FString(debugName.MakeView())) );
    Data.GetOrAdd(fmt) = std::move(shader);
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddShader(EShaderLangFormat fmt, FConstChar entry, const FFilename& filename) {
    Assert(entry);
    Assert(not filename.empty());
    FileSystem::char_type tmp[FileSystem::MaxPathLength];
    const auto fingerprint = Fingerprint128(filename.ToWCStr(tmp)); // use filename as seed, but content is always hashed by compiler itself
    PShaderSource shader = NEW_REF(RHIPipeline, FFileShaderSource_,
        filename, entry, fingerprint );
    Data.GetOrAdd(fmt) = std::move(shader);
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddShader(EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(entry);
    Assert(not rbinary.empty());
    PShaderBinaryData shader = NEW_REF(RHIPipeline, TShaderDataImpl_<FRawData>,
        std::move(rbinary), entry, fingerprint ARGS_IF_RHIDEBUG(debugName) );
    Data.GetOrAdd(fmt) = std::move(shader);
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddShader(EShaderLangFormat fmt, PShaderModule&& rmodule) {
    Assert(rmodule);
    Data.GetOrAdd(fmt) = std::move(rmodule);
}
//----------------------------------------------------------------------------
void FPipelineDesc::FShader::AddShader(EShaderLangFormat fmt, FShaderDataVariant&& rdata) {
    Assert(not std::holds_alternative<std::monostate>(rdata));
    Data.GetOrAdd(fmt) = std::move(rdata);
}
//----------------------------------------------------------------------------
const FShaderDataVariant* FPipelineDesc::FShader::Find(EShaderLangFormat fmt) const NOEXCEPT {
    if (const auto it = Data.find(fmt); Data.end() != it)
        return std::addressof(it->second);
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FPipelineDesc::DescriptorSet(const FDescriptorSetID& id) NOEXCEPT -> FDescriptorSet* {
    for (auto& it : PipelineLayout.DescriptorSets) {
        if (it.Id == id)
            return &it;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
auto FPipelineDesc::DescriptorSet(const FDescriptorSetID& id) const NOEXCEPT -> const FDescriptorSet* {
    for ( auto& it : PipelineLayout.DescriptorSets) {
        if (it.Id == id)
            return &it;
    }
    return nullptr;
}
//---------------------------------------------------------------(-------------
void FPipelineDesc::AddDescriptorSet_(
    const FDescriptorSetID& id, u32 index,
    TMemoryView<const FTextureUniform> textures,
    TMemoryView<const FSamplerUniform> samplers,
    TMemoryView<const FSubpassInputUniform> subpassInputs,
    TMemoryView<const FImageUniform> images,
    TMemoryView<const FBufferUniform> uniformBuffers,
    TMemoryView<const FStorageBufferUniform> storageBuffers,
    TMemoryView<const FRayTracingSceneUniform> rayTracingScenes ) {
    Assert(id.Valid());
    Assert(index < MaxDescriptorSets);

#if USE_PPE_RHIDEBUG && USE_PPE_ASSERT
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
FGraphicsPipelineDesc::FGraphicsPipelineDesc() NOEXCEPT = default;
FGraphicsPipelineDesc::~FGraphicsPipelineDesc() = default;
//----------------------------------------------------------------------------
FGraphicsPipelineDesc::FGraphicsPipelineDesc(const FGraphicsPipelineDesc&) = default;
//----------------------------------------------------------------------------
FGraphicsPipelineDesc::FGraphicsPipelineDesc(FGraphicsPipelineDesc&&) NOEXCEPT = default;
FGraphicsPipelineDesc& FGraphicsPipelineDesc::operator =(FGraphicsPipelineDesc&&) NOEXCEPT = default;
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderData_(&Shaders, type, fmt, module);
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderData_(&Shaders, type, fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, const FFilename& sourceFile) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderData_(&Shaders, type, fmt, entry, sourceFile);
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderData_(&Shaders, type, fmt, entry, std::move(rbinary), fingerprint ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FShaderDataVariant&& rdata) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderData_(&Shaders, type, fmt, std::move(rdata));
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::AddShader(EShaderType type, const FShader& shader) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    AddShaderData_(&Shaders, type, shader);
    return (*this);
}
//----------------------------------------------------------------------------
FGraphicsPipelineDesc& FGraphicsPipelineDesc::SetSpecializationConstants(EShaderType type, TMemoryView<const FSpecializationConstant> values) {
    AssertRelease_NoAssume(EShaderType_IsGraphicsShader(type));
    SetSpecializationConstants_(&Shaders, type, values);
    return (*this);
}
//----------------------------------------------------------------------------
const FVertexAttribute* FGraphicsPipelineDesc::VertexInput(const FVertexID& vertexId) const NOEXCEPT {
    for (const FVertexAttribute& attribute : VertexAttributes) {
        if (attribute.Id == vertexId)
            return &attribute;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
// Compute
//----------------------------------------------------------------------------
FComputePipelineDesc::FComputePipelineDesc() NOEXCEPT = default;
FComputePipelineDesc::~FComputePipelineDesc() = default;
//----------------------------------------------------------------------------
FComputePipelineDesc::FComputePipelineDesc(const FComputePipelineDesc&) = default;
//----------------------------------------------------------------------------
FComputePipelineDesc::FComputePipelineDesc(FComputePipelineDesc&&) NOEXCEPT = default;
FComputePipelineDesc& FComputePipelineDesc::operator =(FComputePipelineDesc&&) NOEXCEPT = default;
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, const PShaderModule& module) {
    Assert_NoAssume(Shader.Data.end() == Shader.Data.find(fmt));
    Shader.AddShader(fmt, module);
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(Shader.Data.end() == Shader.Data.find(fmt));
    Shader.AddShader(fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, FConstChar entry, const FFilename& sourceFile) {
    Assert_NoAssume(Shader.Data.end() == Shader.Data.find(fmt));
    Shader.AddShader(fmt, entry, sourceFile);
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(Shader.Data.end() == Shader.Data.find(fmt));
    Shader.AddShader(fmt, entry, std::move(rbinary), fingerprint ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::AddShader(EShaderLangFormat fmt, FShaderDataVariant&& rdata) {
    Assert_NoAssume(Shader.Data.end() == Shader.Data.find(fmt));
    Shader.AddShader(fmt, std::move(rdata));
    return (*this);
}
//----------------------------------------------------------------------------
FComputePipelineDesc& FComputePipelineDesc::SetSpecializationConstants(TMemoryView<const FSpecializationConstant> values) {
    Assert_NoAssume(not Shader.Data.empty());
    Assert_NoAssume(Shader.Specializations.empty());
    CopySpecializationConstants_(&Shader.Specializations, values);
    return (*this);
}
//----------------------------------------------------------------------------
// Mesh
//----------------------------------------------------------------------------
FMeshPipelineDesc::FMeshPipelineDesc() NOEXCEPT = default;
FMeshPipelineDesc::~FMeshPipelineDesc() = default;
//----------------------------------------------------------------------------
FMeshPipelineDesc::FMeshPipelineDesc(const FMeshPipelineDesc&) = default;
//----------------------------------------------------------------------------
FMeshPipelineDesc::FMeshPipelineDesc(FMeshPipelineDesc&&) NOEXCEPT = default;
FMeshPipelineDesc& FMeshPipelineDesc::operator =(FMeshPipelineDesc&&) NOEXCEPT = default;
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, const PShaderModule& module) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderData_(&Shaders, type, fmt, module);
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderData_(&Shaders, type, fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, const FFilename& sourceFile) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderData_(&Shaders, type, fmt, entry, sourceFile);
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderData_(&Shaders, type, fmt, entry, std::move(rbinary), fingerprint ARGS_IF_RHIDEBUG(debugName));
    return (*this);
}
//----------------------------------------------------------------------------
FMeshPipelineDesc& FMeshPipelineDesc::AddShader(EShaderType type, EShaderLangFormat fmt, FShaderDataVariant&& rdata) {
    AssertRelease_NoAssume(EShaderType_IsMeshProcessingShader(type));
    AddShaderData_(&Shaders, type, fmt, std::move(rdata));
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
FRayTracingPipelineDesc::FRayTracingPipelineDesc() NOEXCEPT = default;
FRayTracingPipelineDesc::~FRayTracingPipelineDesc() = default;
//----------------------------------------------------------------------------
FRayTracingPipelineDesc::FRayTracingPipelineDesc(const FRayTracingPipelineDesc&) = default;
//----------------------------------------------------------------------------
FRayTracingPipelineDesc::FRayTracingPipelineDesc(FRayTracingPipelineDesc&&) NOEXCEPT = default;
FRayTracingPipelineDesc& FRayTracingPipelineDesc::operator =(FRayTracingPipelineDesc&&) NOEXCEPT = default;
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, const PShaderModule& module) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddShader(fmt, module);

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, FString&& rsource ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddShader(fmt, entry, std::move(rsource) ARGS_IF_RHIDEBUG(debugName));

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, const FFilename& sourceFile) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddShader(fmt, entry, sourceFile);

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FConstChar entry, FRawData&& rbinary, FShaderDataFingerprint fingerprint ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddShader(fmt, entry, std::move(rbinary), fingerprint ARGS_IF_RHIDEBUG(debugName));

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::AddShader(const FRTShaderID& id, EShaderType type, EShaderLangFormat fmt, FShaderDataVariant&& rdata) {
    Assert(id.Valid());
    AssertRelease_NoAssume(EShaderType_IsRayTracingShader(type));

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    rtShader.Type = type;
    rtShader.AddShader(fmt, std::move(rdata));

    return (*this);
}
//----------------------------------------------------------------------------
FRayTracingPipelineDesc& FRayTracingPipelineDesc::SetSpecializationConstants(const FRTShaderID& id, TMemoryView<const FSpecializationConstant> values) {
    Assert(id.Valid());

    FRTShader& rtShader = Shaders.FindOrAdd(id);
    Assert_NoAssume(not rtShader.Data.empty());
    Assert_NoAssume(not rtShader.Specializations.empty());
    CopySpecializationConstants_(&rtShader.Specializations, values);

    return (*this);
}
//----------------------------------------------------------------------------
// Uniforms
//----------------------------------------------------------------------------
FTextureUniform::FTextureUniform(const FUniformID& id, EImageSampler textureType, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT
:   base_type(id, index, arraySize, stageFlags, FPipelineDesc::FTexture{
        EResourceState_ShaderSample | EResourceShaderStages_FromShaders(stageFlags),
        textureType
    })
{}
//----------------------------------------------------------------------------
FSamplerUniform::FSamplerUniform(const FUniformID& id, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT
:   base_type(id, index, arraySize, stageFlags, FPipelineDesc::FSampler{})
{}
//----------------------------------------------------------------------------
FSubpassInputUniform::FSubpassInputUniform(const FUniformID& id, u32 attachmentIndex, bool isMultisample, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT
:   base_type(id, index, arraySize, stageFlags, FPipelineDesc::FSubpassInput{
        EResourceState_InputAttachment | EResourceShaderStages_FromShaders(stageFlags),
        attachmentIndex, isMultisample
    })
{}
//----------------------------------------------------------------------------
FImageUniform::FImageUniform(const FUniformID& id, EImageSampler imageType, EShaderAccess access, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT
:   base_type(id, index, arraySize, stageFlags, FPipelineDesc::FImage{
        EResourceState_FromShaderAccess(access) | EResourceShaderStages_FromShaders(stageFlags),
        imageType
    })
{}
//----------------------------------------------------------------------------
FBufferUniform::FBufferUniform(const FUniformID& id, u32 size, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags, u32 dynamicOffsetIndex) NOEXCEPT
:   base_type(id, index, arraySize, stageFlags, FPipelineDesc::FUniformBuffer{
        EResourceState_UniformRead | EResourceShaderStages_FromShaders(stageFlags) |
            (dynamicOffsetIndex != FPipelineDesc::StaticOffset
                ? EResourceFlags::BufferDynamicOffset
                : EResourceFlags::Unknown ),
        dynamicOffsetIndex, size
    })
{}
//----------------------------------------------------------------------------
FStorageBufferUniform::FStorageBufferUniform(const FUniformID& id, u32 staticSize, u32 arrayStride, EShaderAccess access, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags, u32 dynamicOffsetIndex) NOEXCEPT
:   base_type(id, index, arraySize, stageFlags, FPipelineDesc::FStorageBuffer{
        EResourceState_FromShaderAccess(access) | EResourceShaderStages_FromShaders(stageFlags) |
            (dynamicOffsetIndex != FPipelineDesc::StaticOffset
                ? EResourceFlags::BufferDynamicOffset
                : EResourceFlags::Unknown ),
        dynamicOffsetIndex, staticSize, arrayStride
    })
{}
//----------------------------------------------------------------------------
FRayTracingSceneUniform::FRayTracingSceneUniform(const FUniformID& id, const FBindingIndex& index, u32 arraySize, EShaderStages stageFlags) NOEXCEPT
:   base_type(id, index, arraySize, stageFlags, FPipelineDesc::FRayTracingScene{
        EResourceState_RayTracingShaderStorage
    })
{}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
