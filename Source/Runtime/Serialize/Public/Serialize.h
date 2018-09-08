#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_SERIALIZE
#   define PPE_SERIALIZE_API DLL_EXPORT
#else
#   define PPE_SERIALIZE_API DLL_IMPORT
#endif

#include "Allocator/PoolAllocatorTag.h"

namespace PPE {
namespace Serialize {
POOL_TAG_DECL(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FSerializeModule is the entry and exit point encapsulating every call to PPE::Serialize::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FSerializeModule : public FModule {
public:
    FSerializeModule();
    virtual ~FSerializeModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
