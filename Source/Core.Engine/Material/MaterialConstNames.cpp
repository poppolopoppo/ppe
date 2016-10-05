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
    _Macro(Metallic) \
    _Macro(NormalDepth) \
    _Macro(RefractiveIndex) \
    _Macro(Roughness) \
    _Macro(SpecularColor) \
    _Macro(SpecularExponent) \
    _Macro(FWorld) \
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
namespace MaterialConstNamesID {
    enum EType : size_t {
#define DEF_MATERIALCONSTNAMES_ENUM(_Name) _Name,
        FOREACH_MATERIALCONSTNAMES_NAME(DEF_MATERIALCONSTNAMES_ENUM)
#undef DEF_MATERIALCONSTNAMES_ENUM
        _Count,
    };
} //!MaterialConstNamesID
//----------------------------------------------------------------------------
static Graphics::FBindName *gMaterialConstNamesArray = nullptr;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_MATERIALCONSTNAMES_ACCESSOR(_Name) \
    const Graphics::FBindName& FMaterialConstNames::_Name() { \
        return gMaterialConstNamesArray[MaterialConstNamesID::_Name]; \
    }
FOREACH_MATERIALCONSTNAMES_NAME(DEF_MATERIALCONSTNAMES_ACCESSOR)
#undef DEF_MATERIALCONSTNAMES_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FMaterialConstNames::Start() {
    AssertRelease(!gMaterialConstNamesArray);
    gMaterialConstNamesArray = new Graphics::FBindName[MaterialConstNamesID::_Count] {
#define DEF_MATERIALCONSTNAMES_STARTUP(_Name) STRINGIZE(_Name),
        FOREACH_MATERIALCONSTNAMES_NAME(DEF_MATERIALCONSTNAMES_STARTUP)
#undef DEF_MATERIALCONSTNAMES_STARTUP
    };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FMaterialConstNames::Shutdown() {
    AssertRelease(gMaterialConstNamesArray);

    delete[] gMaterialConstNamesArray;
    gMaterialConstNamesArray = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
