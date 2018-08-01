#include "stdafx.h"

#include "Exception.h"

#include "IO/TextWriter.h"
#include "Memory/MemoryView.h"

#if USE_PPE_EXCEPTION_CALLSTACK
#   include "Callstack.h"
#   include "DecodedCallstack.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FException::FException(const char* what) noexcept
:   _what(what) {
    Assert(_what);
#if USE_PPE_EXCEPTION_CALLSTACK
    _siteHash = 0;
    const size_t depth = Core::FCallstack::Capture(MakeView(_callstack), &_siteHash, 1, lengthof(_callstack));
    if (depth < lengthof(_callstack))
        _callstack[depth] = nullptr;
#endif
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_CALLSTACK
FDecodedCallstack FException::Callstack() const {
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
FTextWriter& operator <<(FTextWriter& oss, const FException& e) {
    return oss << e.What()
#if USE_PPE_EXCEPTION_CALLSTACK
        << " (" << (void*)e.SiteHash() << ")" << Eol
        << e.Callstack()
#endif
        ;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FException& e) {
    return oss << e.What()
#if USE_PPE_EXCEPTION_CALLSTACK
        << L" (" << (void*)e.SiteHash() << L")" << Eol
        << e.Callstack()
#endif
        ;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
