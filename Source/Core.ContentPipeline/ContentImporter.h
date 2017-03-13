#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"
#include "Core.ContentPipeline/ContentPipelineNode.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/String.h"

#include "Core.RTTI/RTTI_Macros.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentImporter);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FContentImporterContext {
public:
    explicit FContentImporterContext(const FFilename& filename) : _source(filename) {}
    virtual ~FContentImporterContext() {}

    FContentImporterContext(const FContentImporterContext&) = delete;
    FContentImporterContext& operator=(const FContentImporterContext&) = delete;

    FContentIdentity& Source() { return _source; }
    const FContentIdentity& Source() const { return _source; }

    virtual ILogger* Logger() const = 0;

    virtual const FDirname& IntermediateDir() const = 0;
    virtual const FDirname& OutputDir() const = 0;

    virtual void AddDependency(const FFilename& filename) = 0;

private:
    FContentIdentity _source;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Import>
class TContentImporter;
//----------------------------------------------------------------------------
class IContentImporter : public FContentPipelineNode, public Meta::TDynamicCastable<TContentImporter> {
public:
    virtual ~IContentImporter() {}

    template <typename _Import>
    bool Import(FContentImporterContext& ctx, _Import& dst) const {
        const TContentImporter<_Import>* const importer = As<_Import>();
        if (nullptr == importer)
            throw FContentImporterException("invalid importer type", ctx.Identity(), this);
        else
            return importer->Import(ctx, dst);
    }

    RTTI_CLASS_HEADER(IContentImporter, FContentPipelineNode);
};
//----------------------------------------------------------------------------
template <typename _Import>
class TContentImporter : public IContentImporter {
public:
    typedef _Import import_type;

    virtual ~TContentImporter() {}

    virtual bool Import(FContentImporterContext& ctx, import_type& dst) const = 0;

    META_DYNAMIC_CASTABLE_IMPL(TContentImporter);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
