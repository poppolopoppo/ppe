#include "stdafx.h"

#include "MaterialParameterTime.h"

#include "Material/MaterialContext.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"
#include "World/World.h"
#include "World/WorldTime.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Maths/Units.h"
#include "Core/Time/ProcessTime.h"
#include "Core/Time/Timeline.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MaterialParameterTime_WorldElapsedSeconds::Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) {
    const Units::Time::Seconds t = context.Scene->World()->Time().Elapsed();
    const float value = static_cast<float>(t.Value());

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterTime_WorldTotalSeconds::Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) {
    const Units::Time::Seconds t = context.Scene->World()->Time().Total();
    const float value = static_cast<float>(t.Value());

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterTime_ProcessTotalSeconds::Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) {
    const Units::Time::Seconds t = ProcessTime::TotalSeconds();
    const float value = static_cast<float>(t.Value());

    *cached = value;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterTimeMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    database->BindParameter("uniWorldElapsedSeconds",   new MaterialParameterTime_WorldElapsedSeconds());
    database->BindParameter("uniWorldTotalSeconds",     new MaterialParameterTime_WorldTotalSeconds());
    database->BindParameter("uniProcessTotalSeconds",   new MaterialParameterTime_ProcessTotalSeconds());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
