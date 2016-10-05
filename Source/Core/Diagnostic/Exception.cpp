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
FException::FException(const char* what) noexcept
:   parent_type(what) {
    Assert(what);
#if WITH_CORE_EXCEPTION_CALLSTACK
    _siteHash = 0;
    const size_t depth = Core::FCallstack::Capture(MakeView(_callstack), &_siteHash, 1, lengthof(_callstack));
    if (depth < lengthof(_callstack))
        _callstack[depth] = nullptr;
#endif
}
//----------------------------------------------------------------------------
#if WITH_CORE_EXCEPTION_CALLSTACK
FDecodedCallstack FException::FCallstack() const {
    size_t depth = 0;
    forrange(i, 0, lengthof(_callstack))
        if (nullptr == _callstack[depth])
            break;

    FDecodedCallstack decoded;
    Core::FCallstack::Decode(&decoded, _siteHash, TMemoryView<void* const>(_callstack, depth));
    return decoded;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
