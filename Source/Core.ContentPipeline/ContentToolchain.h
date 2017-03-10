#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentImporter.h"
#include "Core.ContentPipeline/ContentProcessor.h"
#include "Core.ContentPipeline/ContentSerializer.h"

#include "Core.RTTI/RTTI_Macros.h"
#include "Core.RTTI/MetaObject.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Input, typename _Output>
class FContentToolchain;
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(ContentToolchain);
class IContentToolchain : public RTTI::FMetaObject {
public:
    IContentToolchain();
    IContentToolchain(  const IContentImporter* importer,
                        const IContentProcessor* processor,
                        const IContentSerializer* serializer );
    virtual ~IContentToolchain();

    IContentToolchain(const IContentToolchain& ) = delete;
    IContentToolchain& operator =(const IContentToolchain& ) = delete;

    const IContentImporter* Importer() const { return _importer.get(); }
    const IContentProcessor* Processor() const { return _processor.get(); }
    const IContentSerializer* Serializer() const { return _serializer.get(); }

    virtual bool Build( FContentImporterContext& importerContext,
                        FContentProcessorContext& processContext,
                        FContentSerializerContext& serializerContext ) const = 0;

    template <typename _Asset>
    bool Load(FContentSerializerContext& serializerContext, _Asset& asset) const {
        return Serializer()->Deserialize(context, asset);
    }

    RTTI_CLASS_HEADER(IContentToolchain, RTTI::FMetaObject);

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
template <typename _Input, typename _Output>
class FContentToolchain : public IContentToolchain {
public:
    typedef TContentImporter<_Input> importer_type;
    typedef TContentProcessor<_Input, _Output> processor_type;
    typedef TContentSerializer<_Output> serializer_type;

    FContentToolchain() {}
    FContentToolchain(   const importer_type* importer,
                        const processor_type* processor,
                        const serializer_type* serializer )
        : _importer(importer)
        , _processor(processor)
        , _serializer(serializer) {}

    virtual bool Build( FContentImporterContext& importerContext,
                        FContentProcessorContext& processContext,
                        FContentSerializerContext& serializerContext ) const override {
        _Output asset;
        {
            _Input intermediate;
            if (not _importer->Import(importerContext, intermediate))
                return false;

            if (not _processor->Process(processContext, asset, intermediate))
                return false;
        }
        return _serializer->Serialize(serializerContext, asset);
    }

protected:
#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const override {
        FMetaClass::parent_type::RTTI_VerifyPredicates();
        RTTI_VerifyPredicate(dynamic_cast<const importer_type*>(Importer()));
        RTTI_VerifyPredicate(dynamic_cast<const processor_type*>(Processor()));
        RTTI_VerifyPredicate(dynamic_cast<const serializer_type*>(Serializer()));
    }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
