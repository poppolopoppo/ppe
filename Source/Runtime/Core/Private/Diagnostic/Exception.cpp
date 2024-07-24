// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
:   std::runtime_error(what) {
    Assert(what);
#if USE_PPE_EXCEPTION_CALLSTACK
    _siteHash = 0;
    const size_t depth = FCallstack::Capture(MakeView(_callstack), &_siteHash, 3, lengthof(_callstack));
    if (depth < lengthof(_callstack))
        _callstack[depth] = nullptr;
#endif
}
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
FTextWriter& FException::DefaultDescription(FTextWriter& oss, const wchar_t* name, const FException& e) {
    Assert_NoAssume(name);
    return oss
        << "caught exception "
        << MakeCStringView(name)
        << ": "
        << MakeCStringView(e.What())
        << " !";
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
