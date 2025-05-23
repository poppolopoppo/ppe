#pragma once

#include "HAL/Generic/GenericPlatformConsole.h"
#include "IO/StringView.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformConsole : FGenericPlatformConsole {
public:
    STATIC_CONST_INTEGRAL(bool, HasConsole, true);

    using FGenericPlatformConsole::EAttribute;

    static bool Open();
    static void Close();

    static size_t Read(const TMemoryView<char>& buffer);
    static size_t Read(const TMemoryView<wchar_t>& buffer);

    static void Write(const FStringView& text, EAttribute attrs = Default);
    static void Write(const FWStringView& text, EAttribute attrs = Default);

    static void Write(FStringLiteral text, EAttribute attrs = Default) { Write(text.MakeView(), attrs); }
    static void Write(FWStringLiteral text, EAttribute attrs = Default) { Write(text.MakeView(), attrs); }

    static void Flush();

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
