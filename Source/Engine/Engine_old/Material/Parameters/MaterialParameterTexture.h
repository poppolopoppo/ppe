#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
class FMaterialDatabase;

#define EACH_MATERIALPARAMETER_TEXTURE(_Macro) \
    _Macro(EMaterialVariability::FMaterial, float4, RenderTargetDuDvDimensions)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_TEXTURE(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
struct FDuDv {
    Graphics::FBindName TextureName;
    FDuDv(const Graphics::FBindName& textureName);

    typedef float2 value_type;
    EMaterialVariability Variability() const { return EMaterialVariability::Batch; }
    void TypedEval(const FMaterialParameterContext& context, float2& dst);
};
typedef TMaterialParameterMemoizer<FDuDv> Memoizer_DuDv;
extern template class TMaterialParameterMemoizer<FDuDv>;
//----------------------------------------------------------------------------
struct FDuDvDimensions {
    Graphics::FBindName TextureName;
    FDuDvDimensions(const Graphics::FBindName& textureName);

    typedef float4 value_type;
    EMaterialVariability Variability() const { return EMaterialVariability::Batch; }
    void TypedEval(const FMaterialParameterContext& context, float4& dst);
};
typedef TMaterialParameterMemoizer<FDuDvDimensions> Memoizer_DuDvDimensions;
extern template class TMaterialParameterMemoizer<FDuDvDimensions>;
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field );
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
