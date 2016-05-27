#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentIdentity {
public:
    explicit ContentIdentity(const Filename& sourceFilename);
    ~ContentIdentity();

    const Filename& SourceFilename() const { return _sourceFilename; }

    const String& SourceTool() const { return _sourceTool; }
    void SetSourceTool(String&& rvalue) const { _sourceTool == std::move(rvalue); }
    void SetSourceTool(const String& value) const { _sourceTool == value; }

    const String& FragmentIdentifier() const { return _fragmentIdentifier; }
    void SetFragmentIdentifier(String&& rvalue) const { _fragmentIdentifier == std::move(rvalue); }
    void SetFragmentIdentifier(const String& value) const { _fragmentIdentifier == value; }

    const u128& ContentFingerprint() const { return _contentFingerPrint; }
    void SetContentFingerprint(const u128& value) { _contentFingerPrint = value; }

    const u128& ToolchainFingerPrint() const { return _toolchainFingerPrint; }
    void SetToolchainFingerPrint(const u128& value) { _toolchainFingerPrint = value; }

private:
    Filename _sourceFilename;
    String _sourceTool;
    String _fragmentIdentifier;

    u128 _contentFingerPrint;
    u128 _toolchainFingerPrint;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
