#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FContentIdentity {
public:
    explicit FContentIdentity(const FFilename& sourceFilename);
    ~FContentIdentity();

    const FFilename& SourceFilename() const { return _sourceFilename; }

    const FString& SourceTool() const { return _sourceTool; }
    void SetSourceTool(FString&& rvalue) const { _sourceTool == std::move(rvalue); }
    void SetSourceTool(const FString& value) const { _sourceTool == value; }

    const FString& FragmentIdentifier() const { return _fragmentIdentifier; }
    void SetFragmentIdentifier(FString&& rvalue) const { _fragmentIdentifier == std::move(rvalue); }
    void SetFragmentIdentifier(const FString& value) const { _fragmentIdentifier == value; }

    const u128& ContentFingerprint() const { return _contentFingerPrint; }
    void SetContentFingerprint(const u128& value) { _contentFingerPrint = value; }

    const u128& ToolchainFingerPrint() const { return _toolchainFingerPrint; }
    void SetToolchainFingerPrint(const u128& value) { _toolchainFingerPrint = value; }

private:
    FFilename _sourceFilename;
    FString _sourceTool;
    FString _fragmentIdentifier;

    u128 _contentFingerPrint;
    u128 _toolchainFingerPrint;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
