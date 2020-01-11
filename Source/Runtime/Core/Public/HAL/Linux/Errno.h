#pragma once

#include "Core_fwd.h"

#ifndef PLATFORM_LINUX
#   error "invalid platform"
#endif

#include "HAL/Linux/LinuxPlatformString.h"

#include "IO/StaticString.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

#include <errno.h>
#include <string.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FErrno {
    int Code;

    FErrno() : Code(errno) {}
    explicit FErrno(int code) NOEXCEPT : Code(code) {}

    FStringView Message() const {
        return MakeCStringView(::strerror(Code));
    }

    inline friend FTextWriter& operator <<(FTextWriter& oss, FErrno err) {
        return oss << err.Code << " (" << err.Message() << ')';
    }
    inline friend FWTextWriter& operator <<(FWTextWriter& oss, FErrno err) {
        return oss << err.Code << L" (" << UTF_8_TO_WCHAR<>(err.Message()) << L')';
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
