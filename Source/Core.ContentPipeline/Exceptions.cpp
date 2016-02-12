#pragma once

#include "Exceptions.h"

#include "ContentImporter.h"
#include "ContentProcessor.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentPipelineException::ContentPipelineException(const char* what, const ContentIdentity& source)
    : Exception(what)
    , _source(source) {}
//----------------------------------------------------------------------------
ContentPipelineException::~ContentPipelineException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentImporterException::ContentImporterException(const char* what, const ContentIdentity& source, const IContentImporter* importer)
    : ContentPipelineException(what, source)
    , _importer(importer) {}
//----------------------------------------------------------------------------
ContentImporterException::~ContentImporterException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentProcessorException::ContentProcessorException(const char* what, const ContentIdentity& source, const IContentProcessor* processor)
    : ContentPipelineException(what, source)
    , _processor(processor) {}
//----------------------------------------------------------------------------
ContentProcessorException::~ContentProcessorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
