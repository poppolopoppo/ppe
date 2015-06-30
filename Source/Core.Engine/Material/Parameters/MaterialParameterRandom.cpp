#include "stdafx.h"

#include "MaterialParameterRandom.h"

#include "Material/MaterialDatabase.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/RandomGenerator.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterRandom {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_RANDOM(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_RANDOM(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterRandom
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterRandom {
//----------------------------------------------------------------------------
namespace { static RandomGenerator gRandomGenerator; }
//----------------------------------------------------------------------------
void UnitRand(const MaterialParameterContext& context, float& dst) {
    dst = gRandomGenerator.NextFloat01();
}
//----------------------------------------------------------------------------
void UnitRand2(const MaterialParameterContext& context, float2& dst) {
    dst.x() = gRandomGenerator.NextFloat01();
    dst.y() = gRandomGenerator.NextFloat01();
}
//----------------------------------------------------------------------------
void UnitRand3(const MaterialParameterContext& context, float3& dst) {
    dst.x() = gRandomGenerator.NextFloat01();
    dst.y() = gRandomGenerator.NextFloat01();
    dst.z() = gRandomGenerator.NextFloat01();
}
//----------------------------------------------------------------------------
void UnitRand4(const MaterialParameterContext& context, float4& dst) {
    dst.x() = gRandomGenerator.NextFloat01();
    dst.y() = gRandomGenerator.NextFloat01();
    dst.z() = gRandomGenerator.NextFloat01();
    dst.w() = gRandomGenerator.NextFloat01();
}
//----------------------------------------------------------------------------
void HemisphereRand(const MaterialParameterContext& context, float3& dst) {
    dst.x() = gRandomGenerator.NextFloatM11();
    dst.y() = gRandomGenerator.NextFloatM11();
    dst.z() = gRandomGenerator.NextFloat01();
    dst = Normalize3(dst);
}
//----------------------------------------------------------------------------
} //!MaterialParameterRandom
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
