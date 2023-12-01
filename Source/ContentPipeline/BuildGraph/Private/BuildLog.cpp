// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "BuildLog.h"

#include "BuildEnums.h"
#include "BuildGraph.h" // LOGGER_CATEGORY()
#include "BuildNode.h"

#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "VirtualFileSystem_fwd.h"

#include <mutex>

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FDiagnosticBuildLog : public IBuildLog {
public:
    virtual void NodeBegin(const FBuildNode& node) override final {
        Unused(node);
        PPE_LOG(BuildGraph, Emphasis, " >> {0}", node.RTTI_PathName());
    }

    virtual void NodeEnd(const FBuildNode& node, EBuildResult result) override {
        Unused(node);
        switch (result) {
        case PPE::ContentPipeline::EBuildResult::Unbuilt:
            PPE_LOG(BuildGraph, Debug, " ---- {0}", node.RTTI_PathName());
            break;
        case PPE::ContentPipeline::EBuildResult::UpToDate:
            PPE_LOG(BuildGraph, Info, " ---> {0}", node.RTTI_PathName());
            break;
        case PPE::ContentPipeline::EBuildResult::Built:
            PPE_LOG(BuildGraph, Emphasis, " [OK] {0}", node.RTTI_PathName());
            break;
        case PPE::ContentPipeline::EBuildResult::Failed:
            PPE_LOG(BuildGraph, Error, " !KO! {0}", node.RTTI_PathName());
            break;
        default:
            AssertNotImplemented();
        }
    }

    virtual void TraceDebug(const FStringView& text) override final {
        Unused(text);
        PPE_LOG_DIRECT(BuildGraph, Debug, [&](FTextWriter& oss){ oss << text; });
    }
    virtual void TraceError(const FStringView& text) override final {
        Unused(text);
        PPE_LOG_DIRECT(BuildGraph, Error, [&](FTextWriter& oss){ oss << text; });
    }
    virtual void TraceInfo(const FStringView& text) override final {
        Unused(text);
        PPE_LOG_DIRECT(BuildGraph, Info, [&](FTextWriter& oss){ oss << text; });
    }
    virtual void TraceVerbose(const FStringView& text) override final {
        Unused(text);
        PPE_LOG_DIRECT(BuildGraph, Verbose, [&](FTextWriter& oss){ oss << text; });
    }
    virtual void TraceWarning(const FStringView& text) override final {
        Unused(text);
        PPE_LOG_DIRECT(BuildGraph, Warning, [&](FTextWriter& oss){ oss << text; });
    }

    virtual void TraceDebugArgs(const FStringView& fmt, const FFormatArgList& args) override final {
        Unused(fmt);
        Unused(args);
        PPE_LOG_DIRECT(BuildGraph, Debug, [&](FTextWriter& oss){ FormatArgs(oss, fmt, args); });
    }
    virtual void TraceErrorArgs(const FStringView& fmt, const FFormatArgList& args) override final {
        Unused(fmt);
        Unused(args);
        PPE_LOG_DIRECT(BuildGraph, Error, [&](FTextWriter& oss){ FormatArgs(oss, fmt, args); });
    }
    virtual void TraceInfoArgs(const FStringView& fmt, const FFormatArgList& args) override final {
        Unused(fmt);
        Unused(args);
        PPE_LOG_DIRECT(BuildGraph, Info, [&](FTextWriter& oss){ FormatArgs(oss, fmt, args); });
    }
    virtual void TraceVerboseArgs(const FStringView& fmt, const FFormatArgList& args) override final {
        Unused(fmt);
        Unused(args);
        PPE_LOG_DIRECT(BuildGraph, Verbose, [&](FTextWriter& oss){ FormatArgs(oss, fmt, args); });
    }
    virtual void TraceWarningArgs(const FStringView& fmt, const FFormatArgList& args) override final {
        Unused(fmt);
        Unused(args);
        PPE_LOG_DIRECT(BuildGraph, Warning, [&](FTextWriter& oss){ FormatArgs(oss, fmt, args); });
    }

    void Flush() override {
        PPE_LOG_FLUSH();
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UBuildLog MakeDiagnosticsBuildLog() {
    return MakeUnique<FDiagnosticBuildLog>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
