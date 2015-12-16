#include "stdafx.h"

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
struct EffectCompilerKey {
    STATIC_CONST_INTEGRAL(size_t, TagCapacity, 8);

    size_t HashValue;
    PCEffectDescriptor Descriptor;
    Graphics::BindName Tags[TagCapacity];
    Graphics::PCVertexDeclaration VertexDeclaration;

    bool operator ==(const EffectCompilerKey& other) const { 
        return  this == &other || (
                HashValue == other.HashValue && 
                Descriptor == other.Descriptor && 
                VertexDeclaration == other.VertexDeclaration &&
                std::equal(&Tags[0], &Tags[TagCapacity], other.Tags) ); // most expensive last
    }

    bool operator !=(const EffectCompilerKey& other) const { return !operator ==(other); }
};
//----------------------------------------------------------------------------
hash_t hash_value(const EffectCompilerKey& key) {
    return key.HashValue;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EffectCompiler::EffectCompiler() 
:   _device(nullptr)
,   _sharedBufferFactory(nullptr) {
    Assert(_variability.Value == VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
EffectCompiler::~EffectCompiler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);
    Assert(!_sharedBufferFactory);
    Assert(_effects.empty());
    Assert(_variability.Value == VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
Effect *EffectCompiler::GetOrCreateEffect(
    const EffectDescriptor *descriptor,
    const Graphics::VertexDeclaration *vertexDeclaration,
    const MemoryView<const Graphics::BindName>& tags )
{
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(descriptor);
    Assert(vertexDeclaration);
    Assert(_device);
    Assert(_sharedBufferFactory);
    Assert(_variability.Value != VariabilitySeed::Invalid);

    EffectCompilerKey key;
    key.Descriptor = descriptor;
    key.VertexDeclaration = vertexDeclaration;
    for (size_t i = 0; i < tags.size(); ++i) 
        key.Tags[i] = tags[i];

    key.HashValue = hash_value(key.Descriptor, key.VertexDeclaration, key.Tags);

    PEffect& effect = _effects[key];

    if (!effect) {
        Graphics::IDeviceAPIShaderCompiler *const compiler = _device->Encapsulator()->Compiler();

        LOG(Info, L"[EffectCompiler] Create effect for descriptor <{0}> and vertex declaration <{1}> ...",
            descriptor->Name().c_str(), vertexDeclaration->ResourceName() );

        effect = new Effect(descriptor, vertexDeclaration, tags);
        effect->Create(_device);
        effect->LinkReflectedData(_sharedBufferFactory, compiler);
    }

    Assert(effect);
    Assert(effect->Descriptor() == descriptor);
    Assert(effect->VertexDeclaration() == vertexDeclaration);

    return effect;
}
//----------------------------------------------------------------------------
MaterialEffect *EffectCompiler::CreateMaterialEffect(
    const EffectDescriptor *descriptor,
    const Graphics::VertexDeclaration *vertexDeclaration,
    const Material *material ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(material);

    LOG(Info, L"[EffectCompiler] Create material effect '{0}' for material <{1}> descriptor <{2}> and vertex declaration <{3}> ...",
        material->Description(), material->Name(), descriptor->Name().c_str(), vertexDeclaration->ResourceName() );

    STACKLOCAL_POD_ARRAY(Graphics::BindName, activeTags, descriptor->Substitutions().size());
    size_t activeTagsCount = 0;
    for (const Pair<Graphics::BindName, String>& substitution : descriptor->Substitutions())
        if (Contains(material->Tags(), substitution.first))
            activeTags[activeTagsCount++] = substitution.first;

    Effect *const effect = GetOrCreateEffect(descriptor, vertexDeclaration, activeTags.SubRangeConst(0, activeTagsCount) );

    return new MaterialEffect(effect, material);
}
//----------------------------------------------------------------------------
void EffectCompiler::RegenerateEffects() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_variability.Value != VariabilitySeed::Invalid);

    LOG(Info, L"[EffectCompiler] Regenerating {0} effects ...",
        _effects.size() );

    const Units::Time::Seconds startedAt = ProcessTime::TotalSeconds();

    Graphics::IDeviceAPIShaderCompiler *const compiler = _device->Encapsulator()->Compiler();

    for (const Pair<const EffectCompilerKey, PEffect>& effect : _effects) {
        Assert(effect.second->Available());

#ifdef USE_DEBUG_LOGGER
        LOG(Info, L"[EffectCompiler] Regenerate effect named \"{0}\" with vertex declaration <{1}> ...",
            effect.first.Descriptor->Name().c_str(),
            effect.first.VertexDeclaration->ResourceName() );

        for (const Graphics::BindName& tag : effect.first.Tags)
            if (!tag.empty())
                LOG(Info, L"[EffectCompiler] - With material tag <{0}>", tag);
#endif

        effect.second->UnlinkReflectedData(_sharedBufferFactory);
        effect.second->Destroy(_device);

        effect.second->Create(_device);
        effect.second->LinkReflectedData(_sharedBufferFactory, compiler);
    }

    const Units::Time::Seconds stoppedAt = ProcessTime::TotalSeconds();
    const double totalDuration = stoppedAt.Value() - startedAt.Value();

    LOG(Info, L"[EffectCompiler] Regenerated {0} effects in {1:f4} seconds.",
        _effects.size(), totalDuration );

    _variability.Next();
}
//----------------------------------------------------------------------------
void EffectCompiler::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);
    Assert(_variability.Value != VariabilitySeed::Invalid);

    for (Pair<const EffectCompilerKey, PEffect>& effect : _effects) {
        Assert(effect.second->Available());

        effect.second->UnlinkReflectedData(_sharedBufferFactory);
        effect.second->Destroy(_device);

        RemoveRef_AssertReachZero(effect.second);
    }

    _effects.clear();
    _variability.Next();
}
//----------------------------------------------------------------------------
void EffectCompiler::Start(Graphics::IDeviceAPIEncapsulator *device, SharedConstantBufferFactory *sharedBufferFactory) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);
    Assert(!_sharedBufferFactory);
    Assert(_effects.empty());
    Assert(_variability.Value == VariabilitySeed::Invalid);

    LOG(Info, L"[EffectCompiler] Starting with device <{0}> and shared buffer factory <{1}> ...",
        device, sharedBufferFactory );

    _device = device;
    _sharedBufferFactory = sharedBufferFactory;
    _variability.Reset();
}
//----------------------------------------------------------------------------
void EffectCompiler::Shutdown(Graphics::IDeviceAPIEncapsulator *device, SharedConstantBufferFactory *sharedBufferFactory) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(device == _device);
    Assert(sharedBufferFactory);
    Assert(sharedBufferFactory == _sharedBufferFactory);
    Assert(_variability.Value != VariabilitySeed::Invalid);

    LOG(Info, L"[EffectCompiler] Shutting down with device <{0}> and shared buffer factory <{1}> ...",
        device, sharedBufferFactory );

    Clear();

    _device = nullptr;
    _sharedBufferFactory = nullptr;
    _variability.Value = size_t(VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
