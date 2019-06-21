#include "stdafx.h"

#include "Diagnostic/Exception.h"

#include "IO/TextWriter.h"
#include "Memory/MemoryView.h"

#if USE_PPE_EXCEPTION_CALLSTACK
#   include "Diagnostic/Callstack.h"
#   include "Diagnostic/DecodedCallstack.h"
#   include "IO/Format.h"
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
    const size_t depth = PPE::FCallstack::Capture(MakeView(_callstack), &_siteHash, 3, lengthof(_callstack));
    if (depth < lengthof(_callstack))
        _callstack[depth] = nullptr;
#endif
}
//----------------------------------------------------------------------------
FException::~FException() = default;
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_CALLSTACK
FDecodedCallstack FException::Callstack() const {
    const TMemoryView<void* const> frames = MakeView(_callstack);
    const size_t depth = std::distance(
        frames.begin(), frames.Find(nullptr) );

    FDecodedCallstack decoded;
    PPE::FCallstack::Decode(&decoded, _siteHash, frames.FirstNElements(depth));
    return decoded;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FException::DefaultDescription(FWTextWriter& oss, const wchar_t* name, const FException& e) {
    Assert_NoAssume(name);
    return oss
        << L"caught exception "
        << MakeCStringView(name)
        << L": "
        << MakeCStringView(e.What())
        << L" !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FException& e) {
    oss << MakeCStringView(e.What());
#if USE_PPE_EXCEPTION_CALLSTACK
    Format(oss, ", site hash = {0:#X}", e.SiteHash());
    oss << Eol << e.Callstack();
#endif
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FException& e) {
    oss << MakeCStringView(e.What());
#if USE_PPE_EXCEPTION_CALLSTACK
    Format(oss, L", site hash = {0:#X}", e.SiteHash());
    oss << Eol << e.Callstack();
#endif
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
