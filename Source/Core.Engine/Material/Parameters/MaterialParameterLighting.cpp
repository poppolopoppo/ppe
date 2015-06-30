#include "stdafx.h"

#include "MaterialParameterLighting.h"

#include "Lighting/LightingEnvironment.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"
#include "World/World.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterLighting {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_LIGHTING(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_LIGHTING(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterLighting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterLighting {
//----------------------------------------------------------------------------
// Sun
//----------------------------------------------------------------------------
void SunColor(const MaterialParameterContext& context, float3& dst) {
    const ColorRGBA& color = context.Scene->World()->Lighting()->Sun().Color();
    dst = ColorRGBAF(color).Data().xyz();
}
//----------------------------------------------------------------------------
void SunDirection(const MaterialParameterContext& context, float3& dst) {
    dst = context.Scene->World()->Lighting()->Sun().Direction();
}
//----------------------------------------------------------------------------
void SunIntensity(const MaterialParameterContext& context, float& dst) {
    dst = context.Scene->World()->Lighting()->Sun().Intensity();
}
//----------------------------------------------------------------------------
} //!MaterialParameterLighting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterLighting {
//----------------------------------------------------------------------------
// Tone Mapping
//----------------------------------------------------------------------------
void Exposure(const MaterialParameterContext& context, float& dst) {
    dst = context.Scene->World()->Lighting()->Exposure();
}
//----------------------------------------------------------------------------
void WhitePoint(const MaterialParameterContext& context, float& dst) {
    dst = context.Scene->World()->Lighting()->WhitePoint();
}
//----------------------------------------------------------------------------
} //!MaterialParameterLighting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
