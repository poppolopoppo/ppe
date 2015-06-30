#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
class MaterialDatabase;

#define EACH_MATERIALPARAMETER_TEXTURE(_Macro) \
    _Macro(MaterialVariability::Material, float4, RenderTargetDuDvDimensions)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_TEXTURE(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
struct DuDv {
    Graphics::BindName TextureName;
    DuDv(const Graphics::BindName& textureName);

    typedef float2 value_type;
    MaterialVariability Variability() const { return MaterialVariability::Batch; }
    void TypedEval(const MaterialParameterContext& context, float2& dst);
};
typedef MaterialParameterMemoizer<DuDv> Memoizer_DuDv;
extern template class MaterialParameterMemoizer<DuDv>;
//----------------------------------------------------------------------------
struct DuDvDimensions {
    Graphics::BindName TextureName;
    DuDvDimensions(const Graphics::BindName& textureName);

    typedef float4 value_type;
    MaterialVariability Variability() const { return MaterialVariability::Batch; }
    void TypedEval(const MaterialParameterContext& context, float4& dst);
};
typedef MaterialParameterMemoizer<DuDvDimensions> Memoizer_DuDvDimensions;
extern template class MaterialParameterMemoizer<DuDvDimensions>;
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const MaterialParameterMutableContext& context,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
