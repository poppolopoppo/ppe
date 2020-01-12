#pragma once

#include "BuildGraph_fwd.h"

#include "IO/FileSystem_fwd.h"
#include "IO/Format.h"
#include "IO/String_fwd.h"
#include "Memory/UniquePtr.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(BuildLog);
//----------------------------------------------------------------------------
class IBuildLog {
public:
    virtual ~IBuildLog() = default;

    virtual void NodeBegin(const FBuildNode& node) = 0;
    virtual void NodeEnd(const FBuildNode& node, EBuildResult result) = 0;

    virtual void TraceDebug(const FWStringView& text) = 0;
    virtual void TraceError(const FWStringView& text) = 0;
    virtual void TraceInfo(const FWStringView& text) = 0;
    virtual void TraceVerbose(const FWStringView& text) = 0;
    virtual void TraceWarning(const FWStringView& text) = 0;

    virtual void TraceDebugArgs(const FWStringView& fmt, const FWFormatArgList& args) = 0;
    virtual void TraceErrorArgs(const FWStringView& fmt, const FWFormatArgList& args) = 0;
    virtual void TraceInfoArgs(const FWStringView& fmt, const FWFormatArgList& args) = 0;
    virtual void TraceVerboseArgs(const FWStringView& fmt, const FWFormatArgList& args) = 0;
    virtual void TraceWarningArgs(const FWStringView& fmt, const FWFormatArgList& args) = 0;

    virtual void Flush() = 0;

public: // Trace helpers:
    template <typename _Arg0, typename... _Args>
    void TraceDebug(const FWStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        TraceLevel_<&IBuildLog::TraceDebugArgs>(fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }
    template <typename _Arg0, typename... _Args>
    void TraceError(const FWStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        TraceLevel_<&IBuildLog::TraceErrorArgs>(fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }
    template <typename _Arg0, typename... _Args>
    void TraceInfo(const FWStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        TraceLevel_<&IBuildLog::TraceInfoArgs>(fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }
    template <typename _Arg0, typename... _Args>
    void TraceVerbose(const FWStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        TraceLevel_<&IBuildLog::TraceVerboseArgs>(fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }
    template <typename _Arg0, typename... _Args>
    void TraceWarning(const FWStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        TraceLevel_<&IBuildLog::TraceWarningArgs>(fmt, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }

private:
    using trace_args_f = void (IBuildLog::*)(const FWStringView & fmt, const FWFormatArgList & args);

    template <trace_args_f _TraceArgs, typename _Arg0, typename... _Args >
    void TraceLevel_(const FWStringView& fmt, _Arg0&& arg0, _Args&&... args) {
        // args are always passed by pointer, wrapped in a void *
        // this avoids unintended copies and de-correlates from actual types (_FormatArgs is defined in Format.cpp)
        typedef details::FWFormatFunctor_ formatfunc_type;
        const formatfunc_type functors[] = {
            formatfunc_type::Make(std::forward<_Arg0>(arg0)),
            formatfunc_type::Make(std::forward<_Args>(args))...
        };
        (this->*_TraceArgs)(fmt, MakeView(functors));
    }
};
//----------------------------------------------------------------------------
PPE_BUILDGRAPH_API UBuildLog MakeDiagnosticsBuildLog();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
