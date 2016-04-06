#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"
#include "Core.ContentPipeline/ContentPipelineNode.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/String.h"

#include "Core.RTTI/RTTIMacros.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentImporter);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentImporterContext {
public:
    explicit ContentImporterContext(const Filename& filename) : _source(filename) {}
    virtual ~ContentImporterContext() {}

    ContentImporterContext(const ContentImporterContext&) = delete;
    ContentImporterContext& operator=(const ContentImporterContext&) = delete;

    ContentIdentity& Source() { return _source; }
    const ContentIdentity& Source() const { return _source; }

    virtual ILogger* Logger() const = 0;

    virtual const Dirname& IntermediateDir() const = 0;
    virtual const Dirname& OutputDir() const = 0;

    virtual void AddDependency(const Filename& filename) = 0;

private:
    ContentIdentity _source;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Import>
class ContentImporter;
//----------------------------------------------------------------------------
class IContentImporter : public ContentPipelineNode {
public:
    virtual ~IContentImporter() {}

    template <typename _Import>
    bool Import(ContentImporterContext& ctx, _Import& dst) const {
        const ContentImporter<_Import>* const importer = dynamic_cast<const ContentImporter<_Import>*>(this);
        if (nullptr == importer)
            throw ContentImporterException("invalid importer type", ctx.Identity(), this);
        else
            return importer->Import(ctx, dst);
    }

    RTTI_CLASS_HEADER(IContentImporter, ContentPipelineNode);
};
//----------------------------------------------------------------------------
template <typename _Import>
class ContentImporter : public IContentImporter {
public:
    typedef _Import import_type;

    virtual ~ContentImporter() {}

    virtual bool Import(ContentImporterContext& ctx, import_type& dst) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
