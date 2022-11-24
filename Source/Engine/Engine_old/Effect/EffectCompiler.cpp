// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "EffectCompiler.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Maths/Units.h"
#include "Core/Time/ProcessTime.h"

#include "Effect.h"
#include "EffectDescriptor.h"
#include "SharedConstantBuffer.h"
#include "SharedConstantBufferFactory.h"
#include "Material/Material.h"
#include "MaterialEffect.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FEffectCompilerKey {
    STATIC_CONST_INTEGRAL(size_t, TagCapacity, 8);

    size_t HashValue;
    PCEffectDescriptor Descriptor;
    Graphics::FBindName Tags[TagCapacity];
    Graphics::PCVertexDeclaration FVertexDeclaration;

    bool operator ==(const FEffectCompilerKey& other) const { 
        return  this == &other || (
                HashValue == other.HashValue && 
                Descriptor == other.Descriptor && 
                FVertexDeclaration == other.VertexDeclaration &&
                std::equal(&Tags[0], &Tags[TagCapacity], other.Tags) ); // most expensive last
    }

    bool operator !=(const FEffectCompilerKey& other) const { return !operator ==(other); }
};
//----------------------------------------------------------------------------
hash_t hash_value(const FEffectCompilerKey& key) {
    return key.HashValue;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEffectCompiler::FEffectCompiler() 
:   _device(nullptr)
,   _sharedBufferFactory(nullptr) {
    Assert(_variability.Value == FVariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
FEffectCompiler::~FEffectCompiler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);
    Assert(!_sharedBufferFactory);
    Assert(_effects.empty());
    Assert(_variability.Value == FVariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
FEffect *FEffectCompiler::GetOrCreateEffect(
    const FEffectDescriptor *descriptor,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    const TMemoryView<const Graphics::FBindName>& tags )
{
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(descriptor);
    Assert(vertexDeclaration);
    Assert(_device);
    Assert(_sharedBufferFactory);
    Assert(_variability.Value != FVariabilitySeed::Invalid);

    FEffectCompilerKey key;
    key.Descriptor = descriptor;
    key.VertexDeclaration = vertexDeclaration;
    for (size_t i = 0; i < tags.size(); ++i) 
        key.Tags[i] = tags[i];

    key.HashValue = hash_value(key.Descriptor, key.VertexDeclaration, key.Tags);

    PEffect& effect = _effects[key];

    if (!effect) {
        Graphics::IDeviceAPIShaderCompiler *const compiler = _device->Encapsulator()->Compiler();

        LOG(Info, L"[FEffectCompiler] Create effect for descriptor <{0}> and vertex declaration <{1}> ...",
            descriptor->Name().c_str(), vertexDeclaration->ResourceName() );

        effect = new FEffect(descriptor, vertexDeclaration, tags);
        effect->Create(_device);
        effect->LinkReflectedData(_sharedBufferFactory, compiler);
    }

    Assert(effect);
    Assert(effect->Descriptor() == descriptor);
    Assert(effect->VertexDeclaration() == vertexDeclaration);

    return effect;
}
//----------------------------------------------------------------------------
FMaterialEffect *FEffectCompiler::CreateMaterialEffect(
    const FEffectDescriptor *descriptor,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    const FMaterial *material ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(material);

    LOG(Info, L"[FEffectCompiler] Create material effect '{0}' for material <{1}> descriptor <{2}> and vertex declaration <{3}> ...",
        material->Description(), material->Name(), descriptor->Name().c_str(), vertexDeclaration->ResourceName() );

    STACKLOCAL_POD_ARRAY(Graphics::FBindName, activeTags, descriptor->Substitutions().size());
    size_t activeTagsCount = 0;
    for (const TPair<Graphics::FBindName, FString>& substitution : descriptor->Substitutions())
        if (Contains(material->Tags(), substitution.first))
            activeTags[activeTagsCount++] = substitution.first;

    FEffect *const effect = GetOrCreateEffect(descriptor, vertexDeclaration, activeTags.SubRangeConst(0, activeTagsCount) );

    return new FMaterialEffect(effect, material);
}
//----------------------------------------------------------------------------
void FEffectCompiler::RegenerateEffects() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_variability.Value != FVariabilitySeed::Invalid);

    LOG(Info, L"[FEffectCompiler] Regenerating {0} effects ...",
        _effects.size() );

    const Units::Time::Seconds startedAt = ProcessTime::TotalSeconds();

    Graphics::IDeviceAPIShaderCompiler *const compiler = _device->Encapsulator()->Compiler();

    for (const TPair<const FEffectCompilerKey, PEffect>& effect : _effects) {
        Assert(effect.second->Available());

#ifdef USE_DEBUG_LOGGER
        LOG(Info, L"[FEffectCompiler] Regenerate effect named \"{0}\" with vertex declaration <{1}> ...",
            effect.first.Descriptor->Name().c_str(),
            effect.first.VertexDeclaration->ResourceName() );

        for (const Graphics::FBindName& tag : effect.first.Tags)
            if (!tag.empty())
                LOG(Info, L"[FEffectCompiler] - With material tag <{0}>", tag);
#endif

        effect.second->UnlinkReflectedData(_sharedBufferFactory);
        effect.second->Destroy(_device);

        effect.second->Create(_device);
        effect.second->LinkReflectedData(_sharedBufferFactory, compiler);
    }

    const Units::Time::Seconds stoppedAt = ProcessTime::TotalSeconds();
    const double totalDuration = stoppedAt.Value() - startedAt.Value();

    LOG(Info, L"[FEffectCompiler] Regenerated {0} effects in {1:f4} seconds.",
        _effects.size(), totalDuration );

    _variability.Next();
}
//----------------------------------------------------------------------------
void FEffectCompiler::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);
    Assert(_variability.Value != FVariabilitySeed::Invalid);

    for (TPair<const FEffectCompilerKey, PEffect>& effect : _effects) {
        Assert(effect.second->Available());

        effect.second->UnlinkReflectedData(_sharedBufferFactory);
        effect.second->Destroy(_device);

        RemoveRef_AssertReachZero(effect.second);
    }

    _effects.clear();
    _variability.Next();
}
//----------------------------------------------------------------------------
void FEffectCompiler::Start(Graphics::IDeviceAPIEncapsulator *device, FSharedConstantBufferFactory *sharedBufferFactory) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);
    Assert(!_sharedBufferFactory);
    Assert(_effects.empty());
    Assert(_variability.Value == FVariabilitySeed::Invalid);

    LOG(Info, L"[FEffectCompiler] Starting with device <{0}> and shared buffer factory <{1}> ...",
        device, sharedBufferFactory );

    _device = device;
    _sharedBufferFactory = sharedBufferFactory;
    _variability.Reset();
}
//----------------------------------------------------------------------------
void FEffectCompiler::Shutdown(Graphics::IDeviceAPIEncapsulator *device, FSharedConstantBufferFactory *sharedBufferFactory) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(device == _device);
    Assert(sharedBufferFactory);
    Assert(sharedBufferFactory == _sharedBufferFactory);
    Assert(_variability.Value != FVariabilitySeed::Invalid);

    LOG(Info, L"[FEffectCompiler] Shutting down with device <{0}> and shared buffer factory <{1}> ...",
        device, sharedBufferFactory );

    Clear();

    _device = nullptr;
    _sharedBufferFactory = nullptr;
    _variability.Value = size_t(FVariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
