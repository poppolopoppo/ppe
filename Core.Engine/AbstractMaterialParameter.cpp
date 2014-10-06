#include "stdafx.h"

#include "AbstractMaterialParameter.h"

#include "MaterialContext.h"
#include "MaterialDatabase.h"

#include "MaterialParameterCamera.h"
#include "MaterialParameterLighting.h"
#include "MaterialParameterMath.h"
#include "MaterialParameterRandom.h"
#include "MaterialParameterTexture.h"
#include "MaterialParameterTime.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AbstractMaterialParameter::AbstractMaterialParameter(Graphics::ConstantFieldType fieldType, MaterialVariability variability)
:   _data(0) {
    bitchanged_type::InplaceFalse(_data);
    bitdirty_type::InplaceTrue(_data);

    bitvariability_type::InplaceSet(_data, static_cast<size_t>(variability));
    bitfield_type::InplaceSet(_data, static_cast<size_t>(fieldType));
}
//----------------------------------------------------------------------------
AbstractMaterialParameter::~AbstractMaterialParameter() {}
//----------------------------------------------------------------------------
void AbstractMaterialParameter::Eval(const MaterialContext& context, bool *changed) {
    const bool ret = EvalIFN_ReturnIfChanged_(context);
    if (changed)
        *changed |= ret;

    bitchanged_type::InplaceSet(_data, ret);
    bitdirty_type::InplaceFalse(_data);
}
//----------------------------------------------------------------------------
void AbstractMaterialParameter::CopyTo(void *dst, size_t sizeInBytes) const {
    Assert(dst);
    Assert(!Dirty());

    CopyTo_AssumeEvaluated_(dst, sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterDefaultMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    RegisterCameraMaterialParameters(database);
    RegisterLightingMaterialParameters(database);
    RegisterMathMaterialParameters(database);
    RegisterRandomMaterialParameters(database);
    RegisterTextureMaterialParameters(database);
    RegisterTimeMaterialParameters(database);
}
//----------------------------------------------------------------------------
bool TryCreateDefaultMaterialParameter(
    AbstractMaterialParameter **param,
    const Material *material,
    const Scene *scene,
    const Graphics::BindName& name ) {
    Assert(param);
    Assert(!name.empty());

    return (    TryCreateMathMaterialParameter(param, material, scene, name)
            ||  TryCreateTextureMaterialParameter(param, material, scene, name) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
