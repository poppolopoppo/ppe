// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Linux/LinuxPlatformConsole.h"

#ifdef PLATFORM_LINUX

#include "Diagnostic/Logger.h"
#include "IO/StringView.h"

#include "HAL/Linux/LinuxPlatformIncludes.h"

#include <curses.h>
#include <iostream>

#include <term.h>
#ifdef generic_type
#   undef generic_type // term.h:165 :'(
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLinuxColorModifier {

    // ANSI color escape codes
    // https://en.wikipedia.org/wiki/ANSI_escape_code#graphics
    enum EColor {
        Black = 30,
        Red = 31,
        Green = 32,
        Yellow = 33,
        Blue = 34,
        Magenta = 35,
        Cyan = 36,
        White = 37,

        Bright = 60,
        Background = 10,
    };

    using EAttribute = FLinuxPlatformConsole::EAttribute;
    FLinuxPlatformConsole::EAttribute Code;

    // translates console attribute to ANSI color codes
    void Decode(unsigned *pFG, unsigned *pBG) const NOEXCEPT {
        unsigned FG = White + Bright, BG = Black;

        switch (unsigned(Code) & FLinuxPlatformConsole::_FG_MASK) {
        case FLinuxPlatformConsole::FG_BLACK: FG = Black; break;
        case FLinuxPlatformConsole::FG_BLUE: FG = Blue; break;
        case FLinuxPlatformConsole::FG_CYAN: FG = Cyan; break;
        case FLinuxPlatformConsole::FG_GREEN: FG = Green; break;
        case FLinuxPlatformConsole::FG_MAGENTA: FG = Magenta; break;
        case FLinuxPlatformConsole::FG_RED: FG = Red; break;
        case FLinuxPlatformConsole::FG_WHITE: FG = White; break;
        case FLinuxPlatformConsole::FG_YELLOW: FG = Yellow; break;
        };

        switch (unsigned(Code) & FLinuxPlatformConsole::_BG_MASK) {
        case FLinuxPlatformConsole::BG_BLACK: FG = Black; break;
        case FLinuxPlatformConsole::BG_BLUE: FG = Blue; break;
        case FLinuxPlatformConsole::BG_CYAN: FG = Cyan; break;
        case FLinuxPlatformConsole::BG_GREEN: FG = Green; break;
        case FLinuxPlatformConsole::BG_MAGENTA: FG = Magenta; break;
        case FLinuxPlatformConsole::BG_RED: FG = Red; break;
        case FLinuxPlatformConsole::BG_WHITE: FG = White; break;
        case FLinuxPlatformConsole::BG_YELLOW: FG = Yellow; break;
        };

        if (Code & FLinuxPlatformConsole::FG_INTENSITY)
            FG += Bright;
        if (Code & FLinuxPlatformConsole::BG_INTENSITY)
            BG += Bright;

        *pFG = FG;
        *pBG = BG;
    }

    inline friend std::ostream& operator <<(std::ostream& oss, FLinuxColorModifier color) {
        if (color.Code == FLinuxPlatformConsole::Unknown) {
            return oss << "\e[0m";
        }
        else {
            unsigned FG, BG;
            color.Decode(&FG, &BG);
            return oss << "\e[" << FG << ';' << BG << 'm';
        }
    }

    inline friend std::wostream& operator <<(std::wostream& woss, FLinuxColorModifier color) {
        if (color.Code == FLinuxPlatformConsole::Unknown) {
            return woss << L"\e[0m";
        }
        else {
            unsigned FG, BG;
            color.Decode(&FG, &BG);
            return woss << L"\e[" << FG << L';' << BG << L'm';
        }
    }

    static bool HasColorSupport() {
        static const bool GSupported = HasColorSupportImpl_();
        return GSupported;
    }

    template <typename _Char>
    struct TScope : Meta::FNonCopyableNorMovable {
        std::basic_ostream<_Char>& Oss;
        const EAttribute Attrs;

        TScope(std::basic_ostream<_Char>& oss, EAttribute attrs)
        :   Oss(oss), Attrs(attrs) {
            if (FLinuxPlatformConsole::Unknown != Attrs && HasColorSupport())
                Oss << FLinuxColorModifier{ Attrs };
        }

        ~TScope() {
            if (FLinuxPlatformConsole::Unknown != Attrs && HasColorSupport())
                Oss << FLinuxColorModifier{ FLinuxPlatformConsole::Unknown };
        }
    };

    using FScope = TScope<char>;
    using FWScope = TScope<wchar_t>;

private:
    // Must check if the terminal has support for ANSI colors
    // https://www.badprog.com/unix-gnu-linux-system-calls-using-tgetent
    static bool HasColorSupportImpl_() {
        // check if we're outputting to a terminal
        if (not ::isatty(STDOUT_FILENO))
            return false;

        // check terminal for color support
        const char* term = ::getenv("TERM");
        if (nullptr == term)
            return false;

        char bp;
        if (0 == ::tgetent(&bp/*ignored*/, term))
            return false;

        if (0 == ::tgetstr("Ip", nullptr))
            return false;

        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformConsole::Open() {
    NOOP(); // always available on linux
    return true;
}
//----------------------------------------------------------------------------d
void FLinuxPlatformConsole::Close() {
    NOOP();
}
//----------------------------------------------------------------------------
bool FLinuxPlatformConsole::ReadChar(char* ch) {
    Assert(ch);
    std::cin.get(*ch);
    return (not std::cin.bad());
}
//----------------------------------------------------------------------------
bool FLinuxPlatformConsole::ReadChar(wchar_t* wch) {
    Assert(wch);
    std::wcin.get(*wch);
    return (not std::wcin.bad());
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformConsole::Read(const TMemoryView<char>& buffer) {
    Assert(not buffer.empty());
    std::cin.getline(buffer.data(), buffer.size());
    return ::strnlen(buffer.data(), buffer.size());
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformConsole::Read(const TMemoryView<wchar_t>& buffer) {
    Assert(not buffer.empty());
    std::wcin.read(buffer.data(), buffer.size());
    return ::wcsnlen(buffer.data(), buffer.size());
}
//----------------------------------------------------------------------------
void FLinuxPlatformConsole::Write(const FStringView& text, EAttribute attrs/* = Default */) {
    Assert(not text.empty());
    const FLinuxColorModifier::FScope color(std::cout, attrs);
    std::cout.write(text.data(), text.size());
}
//----------------------------------------------------------------------------
void FLinuxPlatformConsole::Write(const FWStringView& text, EAttribute attrs/* = Default */) {
    Assert(not text.empty());
    const FLinuxColorModifier::FWScope color(std::wcout, attrs);
    std::wcout.write(text.data(), text.size());
}
//----------------------------------------------------------------------------
void FLinuxPlatformConsole::Flush() {
    std::wcout.flush();
    std::wcerr.flush();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
