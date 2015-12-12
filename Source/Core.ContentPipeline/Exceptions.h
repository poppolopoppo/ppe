#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/Diagnostic/Exception.h"
#include "Core/IO/FS/Filename.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentPipelineException : public Exception {
public:
    ContentPipelineException(const char* what, const Filename& sourceFile)
        : Exception(what), _sourceFile(sourceFile) {}
    const Filename& SourceFile() const { return _sourceFile; }
private:
    Filename _sourceFile;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
