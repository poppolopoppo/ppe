#include "stdafx.h"

#include "EffectConstantBuffer.h"

#include "MaterialEffect.h"
#include "SharedConstantBuffer.h"

#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Material/IMaterialParameter.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/Dialog.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool TryGetParameter_(
    PMaterialParameter *param,
    const Graphics::BindName& name, 
    MaterialEffect *materialEffect, 
    const Scene *scene ) {
    Assert(param);

    if (materialEffect->Material()->Parameters().TryGet(name, param)) {
        Assert(*param);
        return true;
    }
    if (materialEffect->Parameters().TryGet(name, param)) {
        Assert(*param);
        return true;
    }
    if (scene->MaterialDatabase()->TryGetParameter(name, *param)) {
        Assert(*param);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, EffectConstantBuffer, );
//----------------------------------------------------------------------------
EffectConstantBuffer::EffectConstantBuffer(SharedConstantBuffer *sharedBuffer)
:   _sharedBuffer(sharedBuffer)
,   _headerHashValue(0)
,   _dataHashValue(0) {
    Assert(!_sharedBuffer);

    _variability.Variability = MaterialVariability::Once;
    _variability.Seed.Value = VariabilitySeed::Invalid;
}
//----------------------------------------------------------------------------
EffectConstantBuffer::~EffectConstantBuffer() {}
//----------------------------------------------------------------------------
void EffectConstantBuffer::Prepare(const MaterialParameterMutableContext& context) {

    const Material *material = context.MaterialEffect->Material();
    const auto materialParameters = material->Parameters();
    Assert(context.Database == context.Scene->MaterialDatabase());

    const SharedConstantBufferKey sharedKey = _sharedBuffer->SharedKey();

    const size_t count = sharedKey.Layout->Count();
    const auto names = sharedKey.Layout->Names();
    const auto fields = sharedKey.Layout->Fields();

    _parameters.resize(count);
    _variability.Seed.Value = VariabilitySeed::Invalid;
    _variability.Variability = MaterialVariability::Once;

    for (size_t i = 0; i < count; ++i) {
        const Graphics::BindName& name = names[i];
        const Graphics::ConstantField& field = fields[i];

        PMaterialParameter param;

        if (!TryGetParameter_(&param, name, context.MaterialEffect, context.Scene) &&
            !TryCreateDefaultMaterialParameter(&param, context, name, field)) {

            Dialog::Show(L"Material parameter not found", Dialog::Type::Ok, Dialog::Icon::Aterisk,
                L"Failed to retrieve parameter '{0}' in constant buffer '{1}' from material effect <{2}> !",
                name.c_str(), sharedKey.Name.c_str(), material->Name().c_str() );

            AssertNotImplemented(); // TODO : throw an exception to retry
        }

        Assert(param);
        const MaterialParameterInfo paramInfo = param->Info();

        if (SameOrMoreVariability(paramInfo.Variability, _variability.Variability))
            _variability.Variability = paramInfo.Variability;

        _parameters[i] = param;
    }

    _headerHashValue = hash_range(_parameters.begin(), _parameters.end());
}
//----------------------------------------------------------------------------
void EffectConstantBuffer::Eval(const MaterialParameterContext& context, const VariabilitySeeds& seeds ) {

    if (_sharedBuffer->HeaderHashValue() == _headerHashValue &&
        seeds[size_t(_variability.Variability)] == _variability.Seed) {
        _dataHashValue = 0;
        return;
    }

    _variability.Seed = seeds[size_t(_variability.Variability)];

    const Graphics::ConstantBufferLayout *layout = _sharedBuffer->Layout();

    _rawData.Resize_DiscardData(layout->SizeInBytes());
    memset(_rawData.Pointer(), 0xDE, _rawData.SizeInBytes());

    const size_t count = layout->Count();
    const auto fields = layout->Fields();
    Assert(count == _parameters.size());

    for (size_t i = 0; i < count; ++i) {
        const Graphics::ConstantField& field = fields[i];
        void *const storage = &_rawData.at(field.Offset());
        const size_t sizeInBytes = Graphics::ConstantFieldTypeSizeInBytes(field.Type());
        _parameters[i]->Eval(context, storage, sizeInBytes);
    }

    _dataHashValue = hash_value_as_memory(_rawData.Pointer(), _rawData.SizeInBytes());
}
//----------------------------------------------------------------------------
void EffectConstantBuffer::SetDataIFN(Graphics::IDeviceAPIEncapsulator *device) const {

    if (0 == _dataHashValue)
        return;

    Assert(0 != _headerHashValue);
    
    _sharedBuffer->SetData_OnlyIfChanged(device, _headerHashValue, _dataHashValue, _rawData.MakeConstView());
}
//----------------------------------------------------------------------------
void EffectConstantBuffer::Clear() {

    _headerHashValue = 0;
    _dataHashValue = 0;

    _variability.Variability = MaterialVariability::Once;
    _variability.Seed.Value = VariabilitySeed::Invalid;

    _parameters.clear();
    _parameters.shrink_to_fit();

    _rawData.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
