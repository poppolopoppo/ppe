#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.RTTI/RTTIMacros.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentImporter);
FWD_INTERFACE_REFPTR(ContentProcessor);
FWD_INTERFACE_REFPTR(ContentSerializer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentToolchain : public RTTI::MetaObject {
public:
    ContentToolchain();
    ContentToolchain(   const IContentImporter* importer,
                        const IContentProcessor* processor,
                        const IContentSerializer* serializer );
    virtual ~ContentToolchain();

    ContentToolchain(ContentToolchain& ) = delete;
    ContentToolchain& operator =(ContentToolchain& ) = delete;

    const IContentImporter* Importer() const { return _importer.get(); }
    const IContentProcessor* Processor() const { return _processor.get(); }
    const IContentSerializer* Serializer() const { return _serializer.get(); }

    u128 FingerPrint() const;

    SINGLETON_POOL_ALLOCATED_DECL();
    RTTI_CLASS_HEADER(ContentToolchain, RTTI::MetaObject);

protected:
#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const override;
#endif
private:
    PCContentImporter _importer;
    PCContentProcessor _processor;
    PCContentSerializer _serializer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
