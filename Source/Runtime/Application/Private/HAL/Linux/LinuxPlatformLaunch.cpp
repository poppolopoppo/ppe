// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformLaunch.h"

#include "IO/StaticString.h"
#include "Memory/MemoryStream.h"
#include "Memory/UniqueView.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLinuxPlatformLaunch::OnPlatformLaunch(const char* filename, size_t argc, const char** argv) {
    // convert char to wchar_t, kind of ugly : #TODO refactor to replace wchar_t references with TCHAR
    auto wargs = NEW_ARRAY(Launch, FWString, argc);
    const auto wargv = NEW_ARRAY(Launch, const wchar_t*, argc);
    forrange(i, 0, argc) {
        wargs[i].assign(UTF_8_TO_WCHAR(argv[i]));
        wargv[i] = wargs[i].data();
    }

    FGLFWPlatformLaunch::OnPlatformLaunch(
        UTF_8_TO_WCHAR(filename),
        argc, wargv.data() );
}
//----------------------------------------------------------------------------
void FLinuxPlatformLaunch::OnPlatformShutdown() {
    FGLFWPlatformLaunch::OnPlatformShutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
