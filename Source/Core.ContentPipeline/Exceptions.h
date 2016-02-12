#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"

#include "Core/Diagnostic/Exception.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentImporter);
FWD_INTERFACE_REFPTR(ContentProcessor);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentPipelineException : public Exception {
public:
    ContentPipelineException(const char* what, const ContentIdentity& source);
    ~ContentPipelineException();
    const ContentIdentity& Source() const { return _source; }
private:
    ContentIdentity _source;
};
//----------------------------------------------------------------------------
class ContentImporterException : public ContentPipelineException {
public:
    ContentImporterException(const char* what, const ContentIdentity& source, const IContentImporter* importer);
    ~ContentImporterException();
    const IContentImporter* Importer() const { return _importer.get(); }
private:
    PCContentImporter _importer;
};
//----------------------------------------------------------------------------
class ContentProcessorException : public ContentPipelineException {
public:
    ContentProcessorException(const char* what, const ContentIdentity& source, const IContentProcessor* processor);
    ~ContentProcessorException();
    const IContentProcessor* Processor() const { return _processor.get(); }
private:
    PCContentProcessor _processor;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
