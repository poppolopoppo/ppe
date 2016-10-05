#pragma once

#include "Core.Engine/Engine.h"

namespace Core {
namespace Graphics {
class FBindName;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMaterialConstNames {
private:
    FMaterialConstNames();

public:
    // Parameters :
    static const Graphics::FBindName& AmbientColor();
    static const Graphics::FBindName& DiffuseColor();
    static const Graphics::FBindName& EmissiveColor();
    static const Graphics::FBindName& Metallic();
    static const Graphics::FBindName& NormalDepth();
    static const Graphics::FBindName& RefractiveIndex();
    static const Graphics::FBindName& Roughness();
    static const Graphics::FBindName& SpecularColor();
    static const Graphics::FBindName& SpecularExponent();
    static const Graphics::FBindName& FWorld();

    // Tags :
    static const Graphics::FBindName& Ambient();
    static const Graphics::FBindName& BumpMapping();
    static const Graphics::FBindName& CastShadows();
    static const Graphics::FBindName& Color();
    static const Graphics::FBindName& Emissive();
    static const Graphics::FBindName& Fresnel();
    static const Graphics::FBindName& Glass();
    static const Graphics::FBindName& Highlight();
    static const Graphics::FBindName& Reflection();
    static const Graphics::FBindName& Refraction();
    static const Graphics::FBindName& SeparateAlpha();
    static const Graphics::FBindName& Transparency();

    // Textures :
    static const Graphics::FBindName& AlphaMap();
    static const Graphics::FBindName& AmbientMap();
    static const Graphics::FBindName& DiffuseMap();
    static const Graphics::FBindName& DisplacementMap();
    static const Graphics::FBindName& EmissiveMap();
    static const Graphics::FBindName& NormalMap();
    static const Graphics::FBindName& ReflectionMap();
    static const Graphics::FBindName& SpecularColorMap();
    static const Graphics::FBindName& SpecularPowerMap();

    static void Start();
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
