#include "stdafx.h"

#include "MaterialParameterRandom.h"

#include "MaterialContext.h"
#include "MaterialDatabase.h"

#include "Core.Graphics/BindName.h"

#include "Core/ScalarVector.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MaterialParameterRandom_Unit::Memoize_ReturnIfChanged_(float *cached, const MaterialContext&  ) {
    *cached = _rand.NextFloat01();

    return true;
}
//----------------------------------------------------------------------------
bool MaterialParameterRandom_Unit2::Memoize_ReturnIfChanged_(float2 *cached, const MaterialContext&  ) {
    *cached = float2(   _rand.NextFloat01(),
                        _rand.NextFloat01() );

    return true;
}
//----------------------------------------------------------------------------
bool MaterialParameterRandom_Unit3::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext&  ) {
    *cached = float3(   _rand.NextFloat01(),
                        _rand.NextFloat01(),
                        _rand.NextFloat01() );

    return true;
}
//----------------------------------------------------------------------------
bool MaterialParameterRandom_Unit4::Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext&  ) {
    *cached = float4(   _rand.NextFloat01(),
                        _rand.NextFloat01(),
                        _rand.NextFloat01(),
                        _rand.NextFloat01() );

    return true;
}
//----------------------------------------------------------------------------
bool MaterialParameterRandom_Hemisphere::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext&  ) {
    *cached = Normalize3(
        float3( _rand.NextFloatM11(),
                _rand.NextFloatM11(),
                _rand.NextFloat01() ));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterRandomMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    database->BindParameter("uniRandomUnit",        new MaterialParameterRandom_Unit());
    database->BindParameter("uniRandomUnit2",       new MaterialParameterRandom_Unit2());
    database->BindParameter("uniRandomUnit3",       new MaterialParameterRandom_Unit3());
    database->BindParameter("uniRandomUnit4",       new MaterialParameterRandom_Unit4());
    database->BindParameter("uniRandomHemisphere",  new MaterialParameterRandom_Hemisphere());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
