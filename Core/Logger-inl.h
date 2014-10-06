#pragma once

#include "Logger.h"

#include "Format.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, LogCategory category) {
    return oss << LogCategoryToWCStr(category);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void LoggerFrontend::LogFormat(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args) {
    if (!_impl)
        return;

    wchar_t msg[4096];
    const size_t length = Format(msg, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    _impl->Log(*this, category, msg, length);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void Log(LogCategory category, const wchar_t* format, _Arg0&& arg0, _Args&&... args) {
    Logger::Instance().LogFormat(category, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
