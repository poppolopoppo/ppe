#include "stdafx.h"

#include "RHI_fwd.h"

#if USE_PPE_RHIDEBUG

#include "RHIApi.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE static void Test_PixelFormat1_() {
    forrange(i, 0, static_cast<u32>(EPixelFormat::_Count)) {
        const EPixelFormat fmt = static_cast<EPixelFormat>(i);
        const FPixelFormatInfo info = EPixelFormat_Infos(fmt);
        AssertRelease(info.Format == fmt);
    }
}
//----------------------------------------------------------------------------
void UnitTest_PixelFormat() {
    Test_PixelFormat1_();

    LOG(RHI, Info, L"UnitTest_PixelFormat [PASSED]");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
