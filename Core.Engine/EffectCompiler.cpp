#include "stdafx.h"

#include "EffectCompiler.h"

#include "Core.Graphics/VertexDeclaration.h"

#include "Core/Logger.h"
#include "Core/ProcessTime.h"
#include "Core/Units.h"

#include "Effect.h"
#include "EffectDescriptor.h"
#include "Material.h"
#include "MaterialEffect.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EffectCompiler::EffectCompiler() : _device(nullptr) {
    Assert(_variability.Value == VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
EffectCompiler::~EffectCompiler() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);
    Assert(_effects.empty());
    Assert(_variability.Value == VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
Effect *EffectCompiler::GetOrCreateEffect(
    const EffectDescriptor *descriptor,
    const Graphics::VertexDeclaration *vertexDeclaration)
{
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(descriptor);
    Assert(vertexDeclaration);
    Assert(_device);
    Assert(_variability.Value != VariabilitySeed::Invalid);

    const EffectKey key = { descriptor, vertexDeclaration };
    PEffect& effect = _effects[key];

    if (!effect) {
        LOG(Information, L"[EffectCompiler] Create effect for descriptor <{0}> and vertex declaration <{1}> ...",
            descriptor->Name().c_str(), vertexDeclaration->ResourceName() );

        effect = new Effect(descriptor, vertexDeclaration);
        effect->Create(_device);
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

    LOG(Information, L"[EffectCompiler] Create material effect for material <{0}> descriptor <{1}> and vertex declaration <{2}> ...",
        material->Name(), descriptor->Name().c_str(), vertexDeclaration->ResourceName() );

    Effect *const effect = GetOrCreateEffect(descriptor, vertexDeclaration);

    return new MaterialEffect(effect, material);
}
//----------------------------------------------------------------------------
void EffectCompiler::RegenerateEffects() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_variability.Value != VariabilitySeed::Invalid);

    LOG(Information, L"[EffectCompiler] Regenerating {0} effects ...",
        _effects.size() );

    const Units::Time::Seconds startedAt = ProcessTime::TotalSeconds();

    for (const Pair<const EffectKey, PEffect>& effect : _effects) {
        Assert(effect.second->Available());

        LOG(Information, L"[EffectCompiler] Regenerate effect named \"{0}\" with vertex declaration <{1}> ...",
            effect.first.Descriptor->Name().c_str(),
            effect.first.VertexDeclaration->ResourceName() );

        effect.second->Destroy(_device);
        effect.second->Create(_device);
    }

    const Units::Time::Seconds stoppedAt = ProcessTime::TotalSeconds();
    const double totalDuration = stoppedAt.Value() - startedAt.Value();

    LOG(Information, L"[EffectCompiler] Regenerated {0} effects in {1:f4} seconds.",
        _effects.size(), totalDuration );

    _variability.Next();
}
//----------------------------------------------------------------------------
void EffectCompiler::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);
    Assert(_variability.Value != VariabilitySeed::Invalid);

    for (Pair<const EffectKey, PEffect>& effect : _effects) {
        Assert(effect.second->Available());

        effect.second->Destroy(_device);
        RemoveRef_AssertReachZero(effect.second);
    }

    _effects.clear();
    _variability.Next();
}
//----------------------------------------------------------------------------
void EffectCompiler::Start(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);
    Assert(_effects.empty());
    Assert(_variability.Value == VariabilitySeed::Invalid);

    LOG(Information, L"[EffectCompiler] Starting with device <{0}> ...",
        device );

    _device = device;
    _variability.Reset();
}
//----------------------------------------------------------------------------
void EffectCompiler::Shutdown(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(device == _device);
    Assert(_variability.Value != VariabilitySeed::Invalid);

    LOG(Information, L"[EffectCompiler] Shutting down with device <{0}> ...",
        device );

    for (auto it : _effects) {
        Assert(it.first.Descriptor);
        Assert(it.first.VertexDeclaration);
        Assert(it.second);
        Assert(it.second->Descriptor() == it.first.Descriptor);
        Assert(it.second->VertexDeclaration() == it.first.VertexDeclaration);

        LOG(Information, L"[EffectCompiler] Destroy effect for descriptor <{0}> and vertex declaration <{1}> ...",
            it.first.Descriptor->Name().c_str(), it.first.VertexDeclaration->ResourceName() );

        it.second->Destroy(_device);
        RemoveRef_AssertReachZero(it.second);
    }

    _device = nullptr;
    _variability.Value = size_t(VariabilitySeed::Invalid);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
