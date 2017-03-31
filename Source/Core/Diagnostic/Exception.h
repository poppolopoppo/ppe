#pragma once

#include <exception>
#include <iosfwd>

#ifndef FINAL_RELEASE
#   define WITH_CORE_EXCEPTION_CALLSTACK 1 //%_NOCOMMIT%
#endif

namespace Core {
#if WITH_CORE_EXCEPTION_CALLSTACK
class FDecodedCallstack;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FException : public std::runtime_error {
public:
    typedef std::runtime_error parent_type;

    FException(const char* what) noexcept;

#if WITH_CORE_EXCEPTION_CALLSTACK
    size_t SiteHash() const { return _siteHash; }
    FDecodedCallstack Callstack() const;
#endif

private:
#if WITH_CORE_EXCEPTION_CALLSTACK
    size_t _siteHash;
    void* _callstack[8];
#endif
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const FException& e) {
    return oss << static_cast<const std::runtime_error&>(e)
#if WITH_CORE_EXCEPTION_CALLSTACK
        << " (#0x" << std::hex << std::setfill('0') << std::setw(8) << e.SiteHash()
        << ")" << std::dec << std::setfill(' ') << std::setw(0) << eol
        << e.Callstack()
#endif
        ;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
