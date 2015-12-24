#include "stdafx.h"

#include "ContentIdentity.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ContentIdentity::ContentIdentity(const Filename& sourceFilename)
:   ContentIdentity(sourceFilename, String(), String()) {}
//----------------------------------------------------------------------------
ContentIdentity::ContentIdentity(const Filename& sourceFilename, const String& sourceTool)
:   ContentIdentity(sourceFilename, sourceTool, String()) {}
//----------------------------------------------------------------------------
ContentIdentity::ContentIdentity(const Filename& sourceFilename, const String& sourceTool, const String& fragmentIdentifier)
:   _sourceFilename(sourceFilename)
,   _sourceTool(sourceTool)
,   _fragmentIdentifier(fragmentIdentifier) {
}
//----------------------------------------------------------------------------
ContentIdentity::~ContentIdentity() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core