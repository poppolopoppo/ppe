#include "stdafx.h"

#include "MaterialParameterTime.h"

#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"
#include "World/World.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Maths/Units.h"
#include "Core/Time/ProcessTime.h"
#include "Core/Time/Timeline.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTime {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_TIME(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_TIME(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterTime
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTime {
//----------------------------------------------------------------------------
// FWorld Time
//----------------------------------------------------------------------------
void WorldElapsedSeconds(const FMaterialParameterContext& context, float& dst) {
    const Units::Time::Seconds t = context.Scene->World()->Time().Elapsed();
    dst = static_cast<float>(t.Value());
}
//----------------------------------------------------------------------------
void WorldTotalSeconds(const FMaterialParameterContext& context, float& dst) {
    const Units::Time::Seconds t = context.Scene->World()->Time().Total();
    dst = static_cast<float>(t.Value());
}
//----------------------------------------------------------------------------
} //!MaterialParameterTime
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTime {
//----------------------------------------------------------------------------
// Process Time
//----------------------------------------------------------------------------
void ProcessTotalSeconds(const FMaterialParameterContext& , float& dst) {
    const Units::Time::Seconds t = ProcessTime::TotalSeconds();
    dst = static_cast<float>(t.Value());
}
//----------------------------------------------------------------------------
} //!MaterialParameterTime
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
