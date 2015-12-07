#include "stdafx.h"

#include "Exception.h"

#if WITH_CORE_EXCEPTION_CALLSTACK
#   include "Callstack.h"
#   include "DecodedCallstack.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Exception::Exception(const char* what) noexcept
:   parent_type(what) {
    Assert(what);
#if WITH_CORE_EXCEPTION_CALLSTACK
    _siteHash = 0;
    const size_t depth = Core::Callstack::Capture(MakeView(_callstack), &_siteHash, 1, lengthof(_callstack));
    if (depth < lengthof(_callstack))
        _callstack[depth] = nullptr;
#endif
}
//----------------------------------------------------------------------------
#if WITH_CORE_EXCEPTION_CALLSTACK
DecodedCallstack Exception::Callstack() const {
    size_t depth = 0;
    forrange(i, 0, lengthof(_callstack))
        if (nullptr == _callstack[depth])
            break;

    DecodedCallstack decoded;
    Core::Callstack::Decode(&decoded, _siteHash, MemoryView<void* const>(_callstack, depth));
    return decoded;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core