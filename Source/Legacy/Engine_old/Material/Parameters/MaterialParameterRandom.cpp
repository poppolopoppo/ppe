// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MaterialParameterRandom.h"

#include "Material/MaterialDatabase.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Maths/ScalarVectorHelpers.h"
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
void RegisterMaterialParameters(FMaterialDatabase *database) {
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
namespace {
//----------------------------------------------------------------------------
static FRandomGenerator& Rnd_() {
    ONE_TIME_INITIALIZE(FRandomGenerator, sRng, FRandomGenerator::RandomSeedTag());
    return sRng;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterRandom {
//----------------------------------------------------------------------------
void UnitRand(const FMaterialParameterContext& context, float& dst) {
    dst = Rnd_().NextFloat01();
}
//----------------------------------------------------------------------------
void UnitRand2(const FMaterialParameterContext& context, float2& dst) {
    dst.x() = Rnd_().NextFloat01();
    dst.y() = Rnd_().NextFloat01();
}
//----------------------------------------------------------------------------
void UnitRand3(const FMaterialParameterContext& context, float3& dst) {
    dst.x() = Rnd_().NextFloat01();
    dst.y() = Rnd_().NextFloat01();
    dst.z() = Rnd_().NextFloat01();
}
//----------------------------------------------------------------------------
void UnitRand4(const FMaterialParameterContext& context, float4& dst) {
    dst.x() = Rnd_().NextFloat01();
    dst.y() = Rnd_().NextFloat01();
    dst.z() = Rnd_().NextFloat01();
    dst.w() = Rnd_().NextFloat01();
}
//----------------------------------------------------------------------------
void HemisphereRand(const FMaterialParameterContext& context, float3& dst) {
    dst.x() = Rnd_().NextFloatM11();
    dst.y() = Rnd_().NextFloatM11();
    dst.z() = Rnd_().NextFloat01();
    dst = Normalize3(dst);
}
//----------------------------------------------------------------------------
} //!MaterialParameterRandom
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
