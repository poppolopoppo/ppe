#include "stdafx.h"

#include "ContentToolchain.h"

#include "ContentImporter.h"
#include "ContentProcessor.h"
#include "ContentSerializer.h"

#include "RTTI_Macros-impl.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Memory/HashFunctions.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, FAbstractContentToolchain, Abstract)
RTTI_PROPERTY_PRIVATE_FIELD(_importer)
RTTI_PROPERTY_PRIVATE_FIELD(_processor)
RTTI_PROPERTY_PRIVATE_FIELD(_serializer)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FAbstractContentToolchain::FAbstractContentToolchain() {}
//----------------------------------------------------------------------------
FAbstractContentToolchain::FAbstractContentToolchain(
    const IContentImporter* importer,
    const IContentProcessor* processor,
    const IContentSerializer* serializer )
    : _importer(importer)
    , _processor(processor)
    , _serializer(serializer) {}
//----------------------------------------------------------------------------
FAbstractContentToolchain::~FAbstractContentToolchain() {}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FAbstractContentToolchain::RTTI_VerifyPredicates() const {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(nullptr != _importer);
    RTTI_VerifyPredicate(nullptr != _processor);
    RTTI_VerifyPredicate(nullptr != _serializer);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
