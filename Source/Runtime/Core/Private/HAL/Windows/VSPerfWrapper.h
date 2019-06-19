#pragma once

#include "HAL/Windows/WindowsPlatformProfiler.h"

#if USE_PPE_PLATFORM_PROFILER

#   include "HAL/Windows/WindowsPlatformIncludes.h"

#   include "Diagnostic/Logger.h"
#   include "IO/String.h"
#   include "Meta/Singleton.h"
#   include "Misc/DynamicLibrary.h"

#   include <atomic> // NextMarker()

#   define VSPERF_NO_DEFAULT_LIB	// Don't use #pragma lib to import the library, we'll handle this stuff ourselves
#   define PROFILERAPI				// We won't be statically importing anything (we're dynamically binding), so define PROFILERAPI to a empty value
#   include "VSPerf.h"				// NOTE: This header is in <Visual Studio install directory>/Team Tools/Performance Tools/x64/PerfSDK

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVSPerfWrapper : Meta::TSingleton<FVSPerfWrapper> {
public:
    typedef ::PROFILE_COMMAND_STATUS (*FStopProfile)(::PROFILE_CONTROL_LEVEL Level, unsigned int dwId);
    typedef ::PROFILE_COMMAND_STATUS (*FStartProfile)(::PROFILE_CONTROL_LEVEL Level, unsigned int dwId);

    typedef ::PROFILE_COMMAND_STATUS (*FSuspendProfile)(::PROFILE_CONTROL_LEVEL  Level, unsigned int dwId);
    typedef ::PROFILE_COMMAND_STATUS (*FResumeProfile)(::PROFILE_CONTROL_LEVEL Level, unsigned int dwId);

    typedef ::PROFILE_COMMAND_STATUS (*FMarkProfile)(long lMarker);
    typedef ::PROFILE_COMMAND_STATUS (*FCommentMarkProfileA)(long lMarker, const char* szComment);
    typedef ::PROFILE_COMMAND_STATUS (*FCommentMarkAtProfileA)(__int64 dnTimestamp, long lMarker, const char* szComment);

    typedef ::PROFILE_COMMAND_STATUS (*FCommentMarkProfileW)(long lMarker, const VSPWCHAR* szComment);
    typedef ::PROFILE_COMMAND_STATUS (*FCommentMarkAtProfileW)(__int64 dnTimestamp, long lMarker, const VSPWCHAR* szComment);

    typedef ::PROFILE_COMMAND_STATUS (*FNameProfileA)(const char* pszName, ::PROFILE_CONTROL_LEVEL Level, unsigned int dwId);
    typedef ::PROFILE_COMMAND_STATUS (*FNameProfileW)(const VSPWCHAR* pszName, ::PROFILE_CONTROL_LEVEL Level, unsigned int dwId);

    struct FAPI {
        FStopProfile StopProfile;
        FStartProfile StartProfile;

        FSuspendProfile SuspendProfile;
        FResumeProfile ResumeProfile;

        FMarkProfile MarkProfile;
        FCommentMarkProfileA CommentMarkProfileA;
        FCommentMarkAtProfileA CommentMarkAtProfileA;

        FCommentMarkProfileW CommentMarkProfileW;
        FCommentMarkAtProfileW CommentMarkAtProfileW;

        FNameProfileA NameProfileA;
        FNameProfileW NameProfileW;
    };

    static CONSTEXPR const FAPI GDummyAPI{
        [](::PROFILE_CONTROL_LEVEL, unsigned int) CONSTEXPR { return PROFILE_OK; },
        [](::PROFILE_CONTROL_LEVEL, unsigned int) CONSTEXPR { return PROFILE_OK; },
        [](::PROFILE_CONTROL_LEVEL, unsigned int) CONSTEXPR { return PROFILE_OK; },
        [](::PROFILE_CONTROL_LEVEL, unsigned int) CONSTEXPR { return PROFILE_OK; },
        [](long) CONSTEXPR { return PROFILE_OK; },
        [](long, const char*) CONSTEXPR { return PROFILE_OK; },
        [](__int64, long, const char*) CONSTEXPR { return PROFILE_OK; },
        [](long, const wchar_t*) CONSTEXPR { return PROFILE_OK; },
        [](__int64, long, const wchar_t*) CONSTEXPR { return PROFILE_OK; },
        [](const char*, ::PROFILE_CONTROL_LEVEL, unsigned int) CONSTEXPR { return PROFILE_OK; },
        [](const wchar_t*, ::PROFILE_CONTROL_LEVEL, unsigned int) CONSTEXPR { return PROFILE_OK; }
        };

    static FAPI API;

    ~FVSPerfWrapper();

    bool Available() const { return (_dll.IsValid()); }

    using TSingleton<FVSPerfWrapper>::Get;
    using TSingleton<FVSPerfWrapper>::Destroy;
    static void Create() { TSingleton<FVSPerfWrapper>::Create(); }

private:
    friend class TSingleton<FVSPerfWrapper>;

    FVSPerfWrapper();
    FDynamicLibrary _dll;
};
//----------------------------------------------------------------------------
inline FVSPerfWrapper::FAPI FVSPerfWrapper::API{ GDummyAPI };
//----------------------------------------------------------------------------
inline FVSPerfWrapper::FVSPerfWrapper() {
    static const wchar_t* GVSPerfDllPossiblePaths[] = {
        // VSPerfXXX.dll is installed in Windows/System32 by Visual Studio
        L"VSPerf160.dll", // VS2019
        L"VSPerf150.dll", // VS2017
        L"VSPerf140.dll", // VS2015
        L"VSPerf120.dll", // VS2013
    };

    for (const wchar_t* filename : GVSPerfDllPossiblePaths) {
        if (_dll.AttachOrLoad(filename))
            break;
    }

    if (_dll) {
        FAPI& api = API;

        Verify((api.StopProfile = (FStopProfile)_dll.FunctionAddr("StopProfile")) != nullptr);
        Verify((api.StartProfile = (FStartProfile)_dll.FunctionAddr("StartProfile")) != nullptr);

        Verify((api.SuspendProfile = (FSuspendProfile)_dll.FunctionAddr("SuspendProfile")) != nullptr);
        Verify((api.ResumeProfile = (FResumeProfile)_dll.FunctionAddr("ResumeProfile")) != nullptr);

        Verify((api.MarkProfile = (FMarkProfile)_dll.FunctionAddr("MarkProfile")) != nullptr);
        Verify((api.CommentMarkProfileA = (FCommentMarkProfileA)_dll.FunctionAddr("CommentMarkProfileA")) != nullptr);
        Verify((api.CommentMarkAtProfileA = (FCommentMarkAtProfileA)_dll.FunctionAddr("CommentMarkAtProfileA")) != nullptr);

        Verify((api.CommentMarkProfileW = (FCommentMarkProfileW)_dll.FunctionAddr("CommentMarkProfileW")) != nullptr);
        Verify((api.CommentMarkAtProfileW = (FCommentMarkAtProfileW)_dll.FunctionAddr("CommentMarkAtProfileW")) != nullptr);

        Verify((api.NameProfileA = (FNameProfileA)_dll.FunctionAddr("NameProfileA")) != nullptr);
        Verify((api.NameProfileW = (FNameProfileW)_dll.FunctionAddr("NameProfileW")) != nullptr);

        LOG(HAL, Info, L"successfully loaded VSPerf dll from '{0}'", _dll.ModuleName());
    }
    else {
        API = GDummyAPI;

        LOG(HAL, Warning, L"failed to load VSPerf dll, fallbacking to dummies !");
    }
}
//----------------------------------------------------------------------------
inline FVSPerfWrapper::~FVSPerfWrapper() {
    if (_dll) {
        LOG(HAL, Info, L"unloading VSPerf dll");

        API = GDummyAPI;
        _dll.Unload();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_PROFILER
