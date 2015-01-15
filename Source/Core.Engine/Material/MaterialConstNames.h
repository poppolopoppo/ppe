#pragma once

#include "Core.Engine/Engine.h"

namespace Core {
namespace Graphics {
class BindName;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialConstNames {
private:
    MaterialConstNames();

public:
    // Parameters :
    static const Graphics::BindName& AmbientColor();
    static const Graphics::BindName& DiffuseColor();
    static const Graphics::BindName& EmissiveColor();
    static const Graphics::BindName& NormalDepth();
    static const Graphics::BindName& RefractionIndex();
    static const Graphics::BindName& SpecularColor();
    static const Graphics::BindName& SpecularExponent();

    // Tags :
    static const Graphics::BindName& Ambient();
    static const Graphics::BindName& BumpMapping();
    static const Graphics::BindName& CastShadows();
    static const Graphics::BindName& Color();
    static const Graphics::BindName& Emissive();
    static const Graphics::BindName& Fresnel();
    static const Graphics::BindName& Glass();
    static const Graphics::BindName& Highlight();
    static const Graphics::BindName& Reflection();
    static const Graphics::BindName& Refraction();
    static const Graphics::BindName& SeparateAlpha();
    static const Graphics::BindName& Transparency();

    // Textures :
    static const Graphics::BindName& AlphaMap();
    static const Graphics::BindName& AmbientMap();
    static const Graphics::BindName& DiffuseMap();
    static const Graphics::BindName& DisplacementMap();
    static const Graphics::BindName& EmissiveMap();
    static const Graphics::BindName& NormalMap();
    static const Graphics::BindName& ReflectionMap();
    static const Graphics::BindName& SpecularColorMap();
    static const Graphics::BindName& SpecularPowerMap();

    static void Start();
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
