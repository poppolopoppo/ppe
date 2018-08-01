#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"

#include "Diagnostic/Exception.h"

namespace PPE {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentImporter);
FWD_INTERFACE_REFPTR(ContentProcessor);
FWD_INTERFACE_REFPTR(ContentSerializer);
FWD_REFPTR(AbstractContentToolchain);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FContentPipelineException : public FException {
public:
    FContentPipelineException(const char* what, const FFilename& sourceFilename);
    ~FContentPipelineException();
    const FFilename& SourceFilename() const { return _sourceFilename; }
private:
    FFilename _sourceFilename;
};
//----------------------------------------------------------------------------
class FContentImporterException : public FContentPipelineException {
public:
    FContentImporterException(const char* what, const FContentIdentity& source, const IContentImporter* importer);
    ~FContentImporterException();
    const FContentIdentity& Source() const { return _source; }
    const IContentImporter* Importer() const { return _importer.get(); }
private:
    FContentIdentity _source;
    PCContentImporter _importer;
};
//----------------------------------------------------------------------------
class FContentProcessorException : public FContentPipelineException {
public:
    FContentProcessorException(const char* what, const FContentIdentity& source, const IContentProcessor* processor);
    ~FContentProcessorException();
    const FContentIdentity& Source() const { return _source; }
    const IContentProcessor* Processor() const { return _processor.get(); }
private:
    FContentIdentity _source;
    PCContentProcessor _processor;
};
//----------------------------------------------------------------------------
class FContentSerializerException : public FContentPipelineException {
public:
    FContentSerializerException(const char* what, const FFilename& sourceFilename, const IContentSerializer* serializer);
    ~FContentSerializerException();
    const IContentSerializer* Serializer() const { return _serializer.get(); }
private:
    PCContentSerializer _serializer;
};
//----------------------------------------------------------------------------
class FContentToolchainException : public FContentPipelineException {
public:
    FContentToolchainException(const char* what, const FFilename& sourceFilename, const FAbstractContentToolchain* serializer);
    ~FContentToolchainException();
    const FAbstractContentToolchain* Toolchain() const { return _toolchain.get(); }
private:
    PCAbstractContentToolchain _toolchain;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
