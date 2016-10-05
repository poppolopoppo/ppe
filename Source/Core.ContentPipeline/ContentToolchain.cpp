#include "stdafx.h"

#include "ContentToolchain.h"

#include "ContentImporter.h"
#include "ContentProcessor.h"
#include "ContentSerializer.h"

#include "Core.RTTI/RTTIMacros-impl.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Memory/HashFunctions.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, IContentToolchain, Abstract)
RTTI_PROPERTY_PRIVATE_FIELD(_importer)
RTTI_PROPERTY_PRIVATE_FIELD(_processor)
RTTI_PROPERTY_PRIVATE_FIELD(_serializer)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
IContentToolchain::IContentToolchain() {}
//----------------------------------------------------------------------------
IContentToolchain::IContentToolchain(
    const IContentImporter* importer,
    const IContentProcessor* processor,
    const IContentSerializer* serializer )
    : _importer(importer)
    , _processor(processor)
    , _serializer(serializer) {}
//----------------------------------------------------------------------------
IContentToolchain::~IContentToolchain() {}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void IContentToolchain::RTTI_VerifyPredicates() const {
    FMetaClass::parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(nullptr != _importer);
    RTTI_VerifyPredicate(nullptr != _processor);
    RTTI_VerifyPredicate(nullptr != _serializer);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
