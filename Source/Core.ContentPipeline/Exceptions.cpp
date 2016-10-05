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
FContentPipelineException::FContentPipelineException(const char* what, const FFilename& sourceFilename)
    : FException(what)
    , _sourceFilename(sourceFilename) {}
//----------------------------------------------------------------------------
FContentPipelineException::~FContentPipelineException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FContentImporterException::FContentImporterException(const char* what, const FContentIdentity& source, const IContentImporter* importer)
    : FContentPipelineException(what, source.SourceFilename())
    , _source(source)
    , _importer(importer) {}
//----------------------------------------------------------------------------
FContentImporterException::~FContentImporterException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FContentProcessorException::FContentProcessorException(const char* what, const FContentIdentity& source, const IContentProcessor* processor)
    : FContentPipelineException(what, source.SourceFilename())
    , _source(source)
    , _processor(processor) {}
//----------------------------------------------------------------------------
FContentProcessorException::~FContentProcessorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FContentSerializerException::FContentSerializerException(const char* what, const FFilename& sourceFilename, const IContentSerializer* serializer)
    : FContentPipelineException(what, sourceFilename)
    , _serializer(serializer) {}
//----------------------------------------------------------------------------
FContentSerializerException::~FContentSerializerException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FContentToolchainException::FContentToolchainException(const char* what, const FFilename& sourceFilename, const IContentToolchain* toolchain)
    : FContentPipelineException(what, sourceFilename)
    , _toolchain(toolchain) {}
//----------------------------------------------------------------------------
FContentToolchainException::~FContentToolchainException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
