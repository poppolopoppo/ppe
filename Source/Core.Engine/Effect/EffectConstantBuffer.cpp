#include "stdafx.h"

#include "EffectConstantBuffer.h"

#include "MaterialEffect.h"

#include "Material/Material.h"
#include "Material/MaterialContext.h"
#include "Material/MaterialDatabase.h"
#include "Material/Parameters/AbstractMaterialParameter.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/DialogBox.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static AbstractMaterialParameter *TryGetParameter_(
    const Graphics::BindName& name, 
    MaterialEffect *materialEffect, 
    const Scene *scene ) {

    PAbstractMaterialParameter param;
    if (materialEffect->Material()->Parameters().TryGet(name, &param)) {
        Assert(param);
        return param.get();
    }
    if (materialEffect->Parameters().TryGet(name, &param)) {
        Assert(param);
        return param.get();
    }
    if (scene->MaterialDatabase()->TryGetParameter(name, param)) {
        Assert(param);
        return param.get();
    }

    return nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(EffectConstantBuffer, );
//----------------------------------------------------------------------------
EffectConstantBuffer::EffectConstantBuffer(
    const Graphics::BindName& name,
    const Graphics::ConstantBufferLayout *layout )
:   Graphics::ConstantBuffer(layout)
,   _name(name) {
    Assert(!name.empty());

    _variability.Seed.Value = VariabilitySeed::Invalid;

    SetResourceName(name.cstr());
    Freeze();
}
//----------------------------------------------------------------------------
EffectConstantBuffer::~EffectConstantBuffer() {}
//----------------------------------------------------------------------------
void EffectConstantBuffer::Prepare(
    Graphics::IDeviceAPIEncapsulator *device,
    MaterialEffect *materialEffect,
    MaterialDatabase *materialDatabase,
    const Scene *scene ) {
    const Material *material = materialEffect->Material();
    const auto materialParameters = material->Parameters();
    Assert(materialDatabase == scene->MaterialDatabase());

    const Graphics::ConstantBufferLayout *layout = this->Layout();

    const size_t count = layout->Count();
    const auto names = layout->Names();
    const auto fields = layout->Fields();

    _parameters.resize(count);
    _variability.Seed.Value = VariabilitySeed::Invalid;
    _variability.Variability = MaterialVariability::Once;

    for (size_t i = 0; i < count; ++i) {
        const Graphics::BindName& name = names[i];
        const Graphics::ConstantField& field = fields[i];

        AbstractMaterialParameter *param = TryGetParameter_(name, materialEffect, scene);

        if (!param && !TryCreateDefaultMaterialParameter(&param, materialEffect, materialDatabase, scene, name, field)) {
            DialogBox::Show(L"Material parameter not found", DialogBox::Type::Ok, DialogBox::Icon::Aterisk,
                L"Failed to retrieve parameter '{0}' in constant buffer '{1}' from material effect <{2}> !",
                name.cstr(), _name.cstr(), material->Name().cstr() );
            AssertNotImplemented(); // TODO : throw an exception to retry
        }

        Assert(param);

        if (SameOrMoreVariability(param->Variability(), _variability.Variability))
            _variability.Variability = param->Variability();

        _parameters[i] = param;
    }
}
//----------------------------------------------------------------------------
void EffectConstantBuffer::Eval(Graphics::IDeviceAPIEncapsulator *device,  const MaterialContext& context) {
    bool needUpdate = false;
    for (const PAbstractMaterialParameter& param : _parameters)
        param->Eval(context, &needUpdate);

    if (_variability.Seed != context.Seeds[size_t(_variability.Variability)]) {
        needUpdate = true;
        _variability.Seed = context.Seeds[size_t(_variability.Variability)];
    }

    if (!needUpdate)
        return;

    const Graphics::ConstantBufferLayout *layout = this->Layout();
    const auto rawData = MALLOCA_VIEW(u8, layout->SizeInBytes());

    const size_t count = layout->Count();
    const auto fields = layout->Fields();
    Assert(count == _parameters.size());

    for (size_t i = 0; i < count; ++i) {
        const Graphics::ConstantField& field = fields[i];
        const PAbstractMaterialParameter& param = _parameters[i];

        u8 *const storage = &rawData.at(field.Offset());
        const size_t sizeInBytes = Graphics::ConstantFieldTypeSizeInBytes(field.Type());

        param->CopyTo(storage, sizeInBytes);
    }

    this->SetData(device, rawData.Cast<const u8>() );
}
//----------------------------------------------------------------------------
bool EffectConstantBuffer::Match(
    const Graphics::BindName& name,
    const Graphics::ConstantBufferLayout& layout) const {
    return (_name == name && Layout()->Equals(layout) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
