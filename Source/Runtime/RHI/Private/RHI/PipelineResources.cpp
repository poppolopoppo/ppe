#include "stdafx.h"

#include "RHI/PipelineResources.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
bool UniformDataEquals_(
    const FPipelineResources::FUniform& lhsUni, const void* lhsData,
    const FPipelineResources::FUniform& rhsUni, const void* rhsData) {
    return (lhsUni.Get<T>(lhsData) == rhsUni.Get<T>(rhsData));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
hash_t FPipelineResources::FDynamicData::HashValue() const {
    hash_t h{ PPE_HASH_VALUE_SEED };
    EachUniform([&h](const FUniformID& id, auto& res) {
        hash_combine(h, id, res);
    });
    return h;
}
//----------------------------------------------------------------------------
bool FPipelineResources::FDynamicData::Equals(const FDynamicData& other) const {
    if (LayoutId != other.LayoutId || UniformsCount != other.UniformsCount)
        return false;

    forrange(i, 0, UniformsCount) {
        auto& lhs = Uniforms()[i];
        auto& rhs = other.Uniforms()[i];
        if (lhs.Id != rhs.Id || lhs.Type != rhs.Type)
            return false;

        bool equals = true;
        switch (lhs.Type) {
        case EDescriptorType::Unknown: break;
        case EDescriptorType::Buffer: equals = UniformDataEquals_<FBuffer>(lhs, this, rhs, &other); break;
        case EDescriptorType::TexelBuffer: equals = UniformDataEquals_<FTexelBuffer>(lhs, this, rhs, &other); break;
        case EDescriptorType::SubpassInput:
        case EDescriptorType::Image: equals = UniformDataEquals_<FImage>(lhs, this, rhs, &other); break;
        case EDescriptorType::Texture: equals = UniformDataEquals_<FTexture>(lhs, this, rhs, &other); break;
        case EDescriptorType::Sampler: equals = UniformDataEquals_<FSampler>(lhs, this, rhs, &other); break;
        case EDescriptorType::RayTracingScene: equals = UniformDataEquals_<FRayTracingScene>(lhs, this, rhs, &other); break;
        }

        if (not equals)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/* #TODO
FPipelineResources::~FPipelineResources() {}
//----------------------------------------------------------------------------
FPipelineResources::FPipelineResources(const FPipelineResources& other) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::operator=(const FPipelineResources& other) {}
//----------------------------------------------------------------------------
bool FPipelineResources::HasImage(const FUniformID& id) const {}
//----------------------------------------------------------------------------
bool FPipelineResources::HasSampler(const FUniformID& id) const {}
//----------------------------------------------------------------------------
bool FPipelineResources::HasTexture(const FUniformID& id) const {}
//----------------------------------------------------------------------------
bool FPipelineResources::HasBuffer(const FUniformID& id) const {}
//----------------------------------------------------------------------------
bool FPipelineResources::HasTexelBuffer(const FUniformID& id) const {}
//----------------------------------------------------------------------------
bool FPipelineResources::HasRayTracingScene(const FUniformID& id) const {}
//----------------------------------------------------------------------------
void FPipelineResources::Reset(FUniformID uniform) {}
//----------------------------------------------------------------------------
void FPipelineResources::ResetAll() {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImage(const FUniformID& id, FRawImageID image, u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImage(const FUniformID& id, FRawImageID image, const FImageViewDesc& desc,
    u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImages(const FUniformID& id, TMemoryView<const FImageID> images) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImages(const FUniformID& id, TMemoryView<const FRawImageID> images) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTexture(const FUniformID& id, FRawImageID image, FRawSamplerID sampler,
    u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTexture(const FUniformID& id, FRawImageID image, FRawSamplerID sampler,
    const FImageViewDesc& desc, u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTextures(const FUniformID& id, TMemoryView<const FImageID> images,
    FRawSamplerID sampler) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTextures(const FUniformID& id, TMemoryView<const FRawImageID> images,
    FRawSamplerID sampler) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindSampler(const FUniformID& id, FRawSamplerID sampler, u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindSamplers(const FUniformID& id, TMemoryView<const FSamplerID> samplers) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindSamplers(const FUniformID& id, TMemoryView<const FRawSamplerID> samplers) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffer(const FUniformID& id, FRawBufferID buffer, u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffer(const FUniformID& id, FRawBufferID buffer, u32 offset, u32 size,
    u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffers(const FUniformID& id, TMemoryView<const FBufferID> buffers) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffers(const FUniformID& id, TMemoryView<const FRawBufferID> buffers) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::SetBufferBase(const FUniformID& id, u32 offset, u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTexelBuffer(const FUniformID& name, FRawBufferID buffer,
    const FBufferViewDesc& desc, u32 elementIndex) {}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindRayTracingScene(const FUniformID& id, FRawRTSceneID scene, u32 elementIndex) {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPipelineResources::UDynamicData FPipelineResources::CreateDynamicData(const FPipelineDesc::FUniformMap& uniforms,
    u32 resourceCount, u32 arrayElemCount, u32 bufferDynamicOffsetCount) {}
//----------------------------------------------------------------------------
FPipelineResources::UDynamicData FPipelineResources::CloneDynamicData(const FPipelineResources& res) {}
//----------------------------------------------------------------------------
FPipelineResources::UDynamicData FPipelineResources::RemoveDynamicData(FPipelineResources* pres) {}
//----------------------------------------------------------------------------
bool FPipelineResources::Initialize(FPipelineResources* pres, FRawDescriptorSetLayoutID layoutId,
    const UDynamicData& data) {}
//----------------------------------------------------------------------------
template <typename T>
T* FPipelineResources::Resource_(const FUniformID& id) {

}
//----------------------------------------------------------------------------
template <typename T>
bool FPipelineResources::HasResource_(const FUniformID& id) const {

}
*/
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
