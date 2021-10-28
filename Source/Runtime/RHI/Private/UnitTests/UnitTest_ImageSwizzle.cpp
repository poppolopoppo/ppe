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
NO_INLINE static void Test_ImageSwizzle1_() {
    constexpr FImageSwizzle s1 = "RGBA"_swizzle;
    STATIC_ASSERT(*s1 == 0x1234);
    STATIC_ASSERT(Unswizzle(s1) == uint4(1,2,3,4));

    constexpr FImageSwizzle s2 = "R000"_swizzle;
    STATIC_ASSERT(*s2 == 0x1555);
    STATIC_ASSERT(Unswizzle(s2) == uint4(1,5,5,5));

    constexpr FImageSwizzle s3 = "0G01"_swizzle;
    STATIC_ASSERT(*s3 == 0x5256);
    STATIC_ASSERT(Unswizzle(s3) == uint4(5,2,5,6));
}
//----------------------------------------------------------------------------
void UnitTest_ImageSwizzle() {
    Test_ImageSwizzle1_();

    LOG(RHI, Info, L"UnitTest_ImageSwizzle [PASSED]");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
