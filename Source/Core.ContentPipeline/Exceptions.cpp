#include "stdafx.h"

#include "Exceptions.h"

#include "ContentImporter.h"
#include "ContentProcessor.h"
#include "ContentSerializer.h"
#include "ContentToolchain.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentPipelineException::ContentPipelineException(const char* what, const Filename& sourceFilename)
    : Exception(what)
    , _sourceFilename(sourceFilename) {}
//----------------------------------------------------------------------------
ContentPipelineException::~ContentPipelineException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentImporterException::ContentImporterException(const char* what, const ContentIdentity& source, const IContentImporter* importer)
    : ContentPipelineException(what, source.SourceFilename())
    , _source(source)
    , _importer(importer) {}
//----------------------------------------------------------------------------
ContentImporterException::~ContentImporterException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentProcessorException::ContentProcessorException(const char* what, const ContentIdentity& source, const IContentProcessor* processor)
    : ContentPipelineException(what, source.SourceFilename())
    , _source(source)
    , _processor(processor) {}
//----------------------------------------------------------------------------
ContentProcessorException::~ContentProcessorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentSerializerException::ContentSerializerException(const char* what, const Filename& sourceFilename, const IContentSerializer* serializer)
    : ContentPipelineException(what, sourceFilename)
    , _serializer(serializer) {}
//----------------------------------------------------------------------------
ContentSerializerException::~ContentSerializerException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentToolchainException::ContentToolchainException(const char* what, const Filename& sourceFilename, const IContentToolchain* toolchain)
    : ContentPipelineException(what, sourceFilename)
    , _toolchain(toolchain) {}
//----------------------------------------------------------------------------
ContentToolchainException::~ContentToolchainException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
