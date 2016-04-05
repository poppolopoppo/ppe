#include "stdafx.h"

#include "Logger.h"

#ifdef USE_DEBUG_LOGGER

#include "Diagnostic/CurrentProcess.h"

#include "IO/String.h"
#include "IO/StringSlice.h"

#include <iostream>
#include <sstream>

#ifdef CPP_VISUALSTUDIO
#   include <Windows.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(0 == size_t(LogCategory::Info));
STATIC_ASSERT(1 == size_t(LogCategory::Warning));
STATIC_ASSERT(2 == size_t(LogCategory::Error));
STATIC_ASSERT(3 == size_t(LogCategory::Exception));
STATIC_ASSERT(4 == size_t(LogCategory::Debug));
STATIC_ASSERT(5 == size_t(LogCategory::Assertion));
STATIC_ASSERT(6 == size_t(LogCategory::Profiling));
STATIC_ASSERT(7 == size_t(LogCategory::Callstack));
//----------------------------------------------------------------------------
namespace {
    static const LogCategory sCategories[] = {
        LogCategory::Info,
        LogCategory::Warning,
        LogCategory::Error,
        LogCategory::Exception,
        LogCategory::Debug,
        LogCategory::Assertion,
        LogCategory::Profiling,
        LogCategory::Callstack,
    };

    static const wchar_t* sCategoriesWCStr[] = {
        L"Info",
        L"Warning",
        L"Error",
        L"Exception",
        L"Debug",
        L"Assertion",
        L"Profiling",
        L"Callstack",
    };

    STATIC_ASSERT(lengthof(sCategoriesWCStr) == lengthof(sCategories));
    STATIC_ASSERT(8 == lengthof(sCategories));
}
//----------------------------------------------------------------------------
MemoryView<const LogCategory> EachLogCategory() {
    return MakeView(sCategories);
}
//----------------------------------------------------------------------------
const wchar_t* LogCategoryToWCStr(LogCategory category) {
    Assert(size_t(category) < lengthof(sCategoriesWCStr));
    return sCategoriesWCStr[size_t(category)];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LoggerFrontend::LoggerFrontend()
:   LoggerFrontend(nullptr) {}
//----------------------------------------------------------------------------
LoggerFrontend::LoggerFrontend(ILogger* impl)
:   _impl(impl) {}
//----------------------------------------------------------------------------
LoggerFrontend::~LoggerFrontend() {}
//----------------------------------------------------------------------------
void LoggerFrontend::SetImpl(ILogger* impl) {
    std::lock_guard<std::mutex> scopelock(_lock);
    _impl.reset(impl);
}
//----------------------------------------------------------------------------
void LoggerFrontend::Log(LogCategory category, const WStringSlice& text) {
    std::lock_guard<std::mutex> scopelock(_lock);
    if (_impl)
        _impl->Log(category, text);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void OutputDebugLogger::Log(LogCategory category, const WStringSlice& text) {
#if 0
    if (LogCategory::Callstack != category) {
        wchar_t header[64];
        Format(header, L"[{0:12f7}][{1}] ", CurrentProcess::ElapsedSeconds(), category);
        OutputDebugStringW(header);
    }
#else
    if (LogCategory::Callstack != category &&
        LogCategory::Info != category ) {
        wchar_t header[32];
        Format(header, L"[{0}] ", category);
        OutputDebugStringW(header);
    }
#endif

    Assert('\0' == text.data()[text.size()]); // text must be null terminated !

    OutputDebugStringW(text.data());
    OutputDebugStringW(L"\n");
}
//----------------------------------------------------------------------------
void StdcoutLogger::Log(LogCategory category, const WStringSlice& text) {
    if (LogCategory::Callstack != category) {
        wchar_t header[64];
        Format(header, L"[{0:12f}][{1}] ", CurrentProcess::ElapsedSeconds(), category);
        std::wcout << header;
    }

    std::wcout << text << std::endl;
}
//----------------------------------------------------------------------------
void StderrLogger::Log(LogCategory category, const WStringSlice& text) {
    if (LogCategory::Callstack != category) {
        wchar_t header[64];
        Format(header, L"[{0:12f}][{1}] ", CurrentProcess::ElapsedSeconds(), category);
        std::wcerr << header;
    }

    std::wcerr << text << std::endl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void LoggerStartup::Start() {
    Logger::Create(new OutputDebugLogger());
}
//----------------------------------------------------------------------------
void LoggerStartup::Shutdown() {
    Logger::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!USE_DEBUG_LOGGER
