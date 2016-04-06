#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"

#include "Core/Diagnostic/Exception.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentImporter);
FWD_INTERFACE_REFPTR(ContentProcessor);
FWD_INTERFACE_REFPTR(ContentSerializer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentPipelineException : public Exception {
public:
    ContentPipelineException(const char* what, const Filename& sourceFilename);
    ~ContentPipelineException();
    const Filename& SourceFilename() const { return _sourceFilename; }
private:
    Filename _sourceFilename;
};
//----------------------------------------------------------------------------
class ContentImporterException : public ContentPipelineException {
public:
    ContentImporterException(const char* what, const ContentIdentity& source, const IContentImporter* importer);
    ~ContentImporterException();
    const ContentIdentity& Source() const { return _source; }
    const IContentImporter* Importer() const { return _importer.get(); }
private:
    ContentIdentity _source;
    PCContentImporter _importer;
};
//----------------------------------------------------------------------------
class ContentProcessorException : public ContentPipelineException {
public:
    ContentProcessorException(const char* what, const ContentIdentity& source, const IContentProcessor* processor);
    ~ContentProcessorException();
    const ContentIdentity& Source() const { return _source; }
    const IContentProcessor* Processor() const { return _processor.get(); }
private:
    ContentIdentity _source;
    PCContentProcessor _processor;
};
//----------------------------------------------------------------------------
class ContentSerializerException : public ContentPipelineException {
public:
    ContentSerializerException(const char* what, const Filename& sourceFilename, const IContentSerializer* serializer);
    ~ContentSerializerException();
    const IContentSerializer* Serializer() const { return _serializer.get(); }
private:
    PCContentSerializer _serializer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
