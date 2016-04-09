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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, ContentToolchain, );
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, ContentToolchain, Default)
RTTI_PROPERTY_PRIVATE_FIELD(_importer)
RTTI_PROPERTY_PRIVATE_FIELD(_processor)
RTTI_PROPERTY_PRIVATE_FIELD(_serializer)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
ContentToolchain::ContentToolchain() {}
//----------------------------------------------------------------------------
ContentToolchain::ContentToolchain(
    const IContentImporter* importer,
    const IContentProcessor* processor,
    const IContentSerializer* serializer )
    : _importer(importer)
    , _processor(processor)
    , _serializer(serializer) {}
//----------------------------------------------------------------------------
ContentToolchain::~ContentToolchain() {}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void ContentToolchain::RTTI_VerifyPredicates() const {
    MetaClass::parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(nullptr != _importer);
    RTTI_VerifyPredicate(nullptr != _processor);
    RTTI_VerifyPredicate(nullptr != _serializer);
}
#endif
//----------------------------------------------------------------------------
u128 ContentToolchain::FingerPrint() const {
    const u128 signature[3] = {
        _importer->FingerPrint(),
        _processor->FingerPrint(),
        _serializer->FingerPrint()
    };
    return Fingerprint128(MakeView(signature));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core