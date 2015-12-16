#pragma once

#include "Core/Diagnostic/Logger.h"

#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, LogCategory category) {
    return oss << LogCategoryToWCStr(category);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void ILogger::Log(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args) {
    wchar_t buffer[4096];
    WOCStrStream oss(buffer);
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    this->Log(category, oss.NullTerminatedStr());
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void LoggerFrontend::Log(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args) {
    std::lock_guard<std::mutex> scopelock(_lock);
    if (_impl)
        _impl->Log(category, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename... _Args>
void Log(LogCategory category, const wchar_t* format, _Args&&... args) {
    Logger::Instance().Log(category, format, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
