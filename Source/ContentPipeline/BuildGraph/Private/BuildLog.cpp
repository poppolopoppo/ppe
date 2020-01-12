#include "stdafx.h"

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
        UNUSED(node);
        LOG(BuildGraph, Emphasis, L" >> {0}", node.RTTI_PathName());
    }

    virtual void NodeEnd(const FBuildNode& node, EBuildResult result) override {
        UNUSED(node);
        switch (result) {
        case PPE::ContentPipeline::EBuildResult::Unbuilt:
            LOG(BuildGraph, Debug, L" ---- {0}", node.RTTI_PathName());
            break;
        case PPE::ContentPipeline::EBuildResult::UpToDate:
            LOG(BuildGraph, Info, L" ---> {0}", node.RTTI_PathName());
            break;
        case PPE::ContentPipeline::EBuildResult::Built:
            LOG(BuildGraph, Emphasis, L" [OK] {0}", node.RTTI_PathName());
            break;
        case PPE::ContentPipeline::EBuildResult::Failed:
            LOG(BuildGraph, Error, L" !KO! {0}", node.RTTI_PathName());
            break;
        default:
            AssertNotImplemented();
        }
    }

    virtual void TraceDebug(const FWStringView& text) override final {
        UNUSED(text);
        LOG_DIRECT(BuildGraph, Debug, text);
    }
    virtual void TraceError(const FWStringView& text) override final {
        UNUSED(text);
        LOG_DIRECT(BuildGraph, Error, text);
    }
    virtual void TraceInfo(const FWStringView& text) override final {
        UNUSED(text);
        LOG_DIRECT(BuildGraph, Info, text);
    }
    virtual void TraceVerbose(const FWStringView& text) override final {
        UNUSED(text);
        LOG_DIRECT(BuildGraph, Verbose, text);
    }
    virtual void TraceWarning(const FWStringView& text) override final {
        UNUSED(text);
        LOG_DIRECT(BuildGraph, Warning, text);
    }

    virtual void TraceDebugArgs(const FWStringView& fmt, const FWFormatArgList& args) override final {
        UNUSED(fmt);
        UNUSED(args);
        LOG_ARGS(BuildGraph, Debug, fmt, args);
    }
    virtual void TraceErrorArgs(const FWStringView& fmt, const FWFormatArgList& args) override final {
        UNUSED(fmt);
        UNUSED(args);
        LOG_ARGS(BuildGraph, Error, fmt, args);
    }
    virtual void TraceInfoArgs(const FWStringView& fmt, const FWFormatArgList& args) override final {
        UNUSED(fmt);
        UNUSED(args);
        LOG_ARGS(BuildGraph, Info, fmt, args);
    }
    virtual void TraceVerboseArgs(const FWStringView& fmt, const FWFormatArgList& args) override final {
        UNUSED(fmt);
        UNUSED(args);
        LOG_ARGS(BuildGraph, Verbose, fmt, args);
    }
    virtual void TraceWarningArgs(const FWStringView& fmt, const FWFormatArgList& args) override final {
        UNUSED(fmt);
        UNUSED(args);
        LOG_ARGS(BuildGraph, Warning, fmt, args);
    }

    void Flush() override {
        FLUSH_LOG();
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UBuildLog MakeDiagnosticsBuildLog() {
    return UBuildLog{ new FDiagnosticBuildLog() };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
