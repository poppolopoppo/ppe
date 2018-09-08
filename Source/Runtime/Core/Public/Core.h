#pragma once

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <type_traits>

#ifdef EXPORT_PPE_CORE
#   define PPE_CORE_API DLL_EXPORT
#else
#   define PPE_CORE_API DLL_IMPORT
#endif

#include "Meta/Config.h"

#include "Meta/Aliases.h"
#include "Meta/Alignment.h"
#include "Meta/Assert.h"
#include "Meta/Cast.h"
#include "Meta/Delete.h"
#include "Meta/Enum.h"
#include "Meta/ForRange.h"
#include "Meta/Hash_fwd.h"
#include "Meta/Iterator.h"
#include "Meta/NumericLimits.h"
#include "Meta/OneTimeInitialize.h"
#include "Meta/StronglyTyped.h"
#include "Meta/ThreadResource.h"
#include "Meta/TypeTraits.h"
#include "Meta/Warnings.h"

#include "Module.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FCoreModule is the entry and exit point encapsulating every call to PPE::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class PPE_CORE_API FCoreModule : public FModule {
public:
    FCoreModule();
    virtual ~FCoreModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;

    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
