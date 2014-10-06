#include "stdafx.h"

#include "MaterialParameterTime.h"

#include "MaterialContext.h"
#include "MaterialDatabase.h"
#include "Scene.h"
#include "World.h"
#include "WorldTime.h"

#include "Core.Graphics/BindName.h"

#include "Core/ProcessTime.h"
#include "Core/Timeline.h"
#include "Core/Units.h"

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
