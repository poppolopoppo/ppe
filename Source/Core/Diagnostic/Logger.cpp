#include "stdafx.h"

#include "Logger.h"

#include "IO/String.h"
#include "IO/StringSlice.h"

#include <iostream>

#ifdef CPP_VISUALSTUDIO
#   include <Windows.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(0 == size_t(LogCategory::Information));
STATIC_ASSERT(1 == size_t(LogCategory::Warning));
STATIC_ASSERT(2 == size_t(LogCategory::Error));
STATIC_ASSERT(3 == size_t(LogCategory::Exception));
STATIC_ASSERT(4 == size_t(LogCategory::Assertion));
STATIC_ASSERT(5 == size_t(LogCategory::Profiling));
//----------------------------------------------------------------------------
namespace {
    static const LogCategory sCategories[] = {
        LogCategory::Information,
        LogCategory::Warning,
        LogCategory::Error,
        LogCategory::Exception,
        LogCategory::Assertion,
        LogCategory::Profiling,
    };

    static const wchar_t* sCategoriesWCStr[] = {
        L"Information",
        L"Warning",
        L"Error",
        L"Exception",
        L"Assertion",
        L"Profiling",
    };
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
:   _impl(impl), _startedAt(clock_type::now()) {}
//----------------------------------------------------------------------------
LoggerFrontend::~LoggerFrontend() {}
//----------------------------------------------------------------------------
void LoggerFrontend::SetImpl(ILogger* impl) {
    std::lock_guard <std::mutex> scopelock(_lock);
    _impl.reset(impl);
}
//----------------------------------------------------------------------------
void LoggerFrontend::Log(LogCategory category, const wchar_t* text) {
    std::lock_guard <std::mutex> scopelock(_lock);
    if (_impl)
        _impl->Log(*this, category, text, Length(text));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void OutputDebugLogger::Log(const LoggerFrontend& frontend, LogCategory category, const wchar_t* text, size_t length) {
    if (!IsDebuggerPresent())
        return;

    if (LogCategory::Callstack != category) {
        wchar_t header[128];
        Format(header, L"[{0:12f}][{1}] ", frontend.Now(), category);
        OutputDebugStringW(header);
    }

    OutputDebugStringW(text);
    OutputDebugStringW(L"\n");
}
//----------------------------------------------------------------------------
void StdErrorLogger::Log(const LoggerFrontend& frontend, LogCategory category, const wchar_t* text, size_t length) {
    if (LogCategory::Callstack != category) {
        wchar_t header[128];
        Format(header, L"[{0:12f}][{1}] ", frontend.Now(), category);
        std::cerr << header << text << std::endl;
    }
    else {
        std::cerr << text << std::endl;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Log(LogCategory category, const wchar_t* text) {
    Logger::Instance().Log(category, text);
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
