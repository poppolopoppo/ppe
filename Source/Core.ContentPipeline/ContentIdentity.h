#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentIdentity {
public:
    ContentIdentity(const Filename& sourceFilename);
    ContentIdentity(const Filename& sourceFilename, const String& sourceTool);
    ContentIdentity(const Filename& sourceFilename, const String& sourceTool, const String& fragmentIdentifier);
    ~ContentIdentity();

    const Filename& SourceFilename() const { return _sourceFilename; }
    const String& SourceTool() const { return _sourceTool; }
    const String& FragmentIdentifier() const { return _fragmentIdentifier; }

private:
    Filename _sourceFilename;
    String _sourceTool;
    String _fragmentIdentifier;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
