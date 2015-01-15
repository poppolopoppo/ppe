#include "stdafx.h"

#include "MaterialConstNames.h"

#include "Core/Memory/AlignedStorage.h"
#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define FOREACH_MATERIALCONSTNAMES_NAME(_Macro) \
    _Macro(AmbientColor) \
    _Macro(DiffuseColor) \
    _Macro(EmissiveColor) \
    _Macro(NormalDepth) \
    _Macro(RefractionIndex) \
    _Macro(SpecularColor) \
    _Macro(SpecularExponent) \
    \
    _Macro(Ambient) \
    _Macro(BumpMapping) \
    _Macro(CastShadows) \
    _Macro(Color) \
    _Macro(Emissive) \
    _Macro(Fresnel) \
    _Macro(Glass) \
    _Macro(Highlight) \
    _Macro(Reflection) \
    _Macro(Refraction) \
    _Macro(SeparateAlpha) \
    _Macro(Transparency) \
    \
    _Macro(AlphaMap) \
    _Macro(AmbientMap) \
    _Macro(DiffuseMap) \
    _Macro(DisplacementMap) \
    _Macro(EmissiveMap) \
    _Macro(NormalMap) \
    _Macro(ReflectionMap) \
    _Macro(SpecularColorMap) \
    _Macro(SpecularPowerMap)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_MATERIALCONSTNAMES_STORAGE(_Name) \
    static POD_STORAGE(Graphics::BindName) CONCAT(gPod_, _Name);
//----------------------------------------------------------------------------
FOREACH_MATERIALCONSTNAMES_NAME(DEF_MATERIALCONSTNAMES_STORAGE)
//----------------------------------------------------------------------------
#undef DEF_MATERIALCONSTNAMES_STORAGE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_MATERIALCONSTNAMES_ACCESSOR(_Name) \
    const Graphics::BindName& MaterialConstNames::_Name() { \
        return *reinterpret_cast<const Graphics::BindName *>(&CONCAT(gPod_, _Name)); \
    }
//----------------------------------------------------------------------------
FOREACH_MATERIALCONSTNAMES_NAME(DEF_MATERIALCONSTNAMES_ACCESSOR)
//----------------------------------------------------------------------------
#undef DEF_MATERIALCONSTNAMES_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MaterialConstNames::Start() {
#define DEF_MATERIALCONSTNAMES_STARTUP(_Name) \
    new ((void *)&CONCAT(gPod_, _Name)) Graphics::BindName(STRINGIZE(_Name));

    FOREACH_MATERIALCONSTNAMES_NAME(DEF_MATERIALCONSTNAMES_STARTUP)

#undef DEF_MATERIALCONSTNAMES_STARTUP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MaterialConstNames::Shutdown() {
#define DEF_MATERIALCONSTNAMES_SHUTDOWN(_Name) \
    reinterpret_cast<const Graphics::BindName *>(&CONCAT(gPod_, _Name))->~BindName();

    FOREACH_MATERIALCONSTNAMES_NAME(DEF_MATERIALCONSTNAMES_SHUTDOWN)

#undef DEF_MATERIALCONSTNAMES_SHUTDOWN
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
