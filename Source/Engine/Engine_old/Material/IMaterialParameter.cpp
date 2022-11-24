// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IMaterialParameter.h"

#include "Effect/MaterialEffect.h"

#include "Material/IMaterialParameter.h"
#include "Material/Material.h"
#include "Material/MaterialDatabase.h"

#include "Parameters/MaterialParameterConstant.h"
#include "Parameters/MaterialParameterMemoizer.h"

#include "Parameters/MaterialParameterBool.h"
#include "Parameters/MaterialParameterCamera.h"
#include "Parameters/MaterialParameterLighting.h"
#include "Parameters/MaterialParameterMath.h"
#include "Parameters/MaterialParameterMouse.h"
#include "Parameters/MaterialParameterRandom.h"
#include "Parameters/MaterialParameterTexture.h"
#include "Parameters/MaterialParameterTime.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterDefaultMaterialParameters(FMaterialDatabase *database) {
    Assert(database);

    MaterialParameterBool::RegisterMaterialParameters(database);
    MaterialParameterCamera::RegisterMaterialParameters(database);
    MaterialParameterLighting::RegisterMaterialParameters(database);
    MaterialParameterMath::RegisterMaterialParameters(database);
    MaterialParameterMouse::RegisterMaterialParameters(database);
    MaterialParameterRandom::RegisterMaterialParameters(database);
    MaterialParameterTexture::RegisterMaterialParameters(database);
    MaterialParameterTime::RegisterMaterialParameters(database);
}
//----------------------------------------------------------------------------
bool TryCreateDefaultMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field ) {
    Assert(param);
    Assert(!name.empty());

    return (    MaterialParameterBool::TryCreateMaterialParameter(param, context, name, field)
            ||  MaterialParameterMath::TryCreateMaterialParameter(param, context, name, field)
            ||  MaterialParameterTexture::TryCreateMaterialParameter(param, context, name, field)
            ||  TryCreateOptionalMaterialParameter(param, context, name, field) );
}
//----------------------------------------------------------------------------
bool GetOrCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field) {
    Assert(param);
    Assert(!name.empty());

    // Local source parameter search :
    if (context.MaterialEffect->Material()->Parameters().TryGet(name, param) ||
        context.MaterialEffect->Parameters().TryGet(name, param) ) {
        Assert(*param);
        return true;
    }
    // Global source parameter search :
    else if (context.Database->TryGetParameter(name, *param)) {
        Assert(*param);
        return true;
    }
    // If name does not exists try to create one (may exists with a modifier, like uniInvert_*) :
    else if (TryCreateDefaultMaterialParameter(param, context, name, field)) {
        Assert(*param);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
