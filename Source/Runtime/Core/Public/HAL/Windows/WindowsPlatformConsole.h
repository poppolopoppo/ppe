#pragma once

#include "HAL/Generic/GenericPlatformConsole.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_API FWindowsPlatformConsole : FGenericPlatformConsole {
public:
    STATIC_CONST_INTEGRAL(bool, HasConsole, true);

    using FGenericPlatformConsole::EAttribute;

    static void Open();
    static void Close();

    static size_t Read(const TMemoryView<char>& buffer);
    static size_t Read(const TMemoryView<wchar_t>& buffer);

    static void Write(const FStringView& text, EAttribute attrs = Default);
    static void Write(const FWStringView& text, EAttribute attrs = Default);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
