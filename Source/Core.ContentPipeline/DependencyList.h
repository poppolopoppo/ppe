#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/Container/HashSet.h"
#include "Core.RTTI/RTTIMacros.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DependencyList : public RTTI::MetaObject {
public:
    DependencyList();
    ~DependencyList();

    DependencyList(const DependencyList&) = delete;
    DependencyList& operator =(const DependencyList&) = delete;

    RTTI_CLASS_HEADER(DependencyList, RTTI::MetaObject);

private:
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
