// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/RHIService.h"

#include "RHI/ResourceId.h"

#if USE_PPE_RHIDEBUG
#include "RHI/FrameGraph.h"
#include "Diagnostic/Logger.h"
namespace PPE::RHI {
extern void UnitTest_Id();
extern void UnitTest_ImageDesc();
extern void UnitTest_ImageSwizzle();
extern void UnitTest_PixelFormat();
extern void UnitTest_ResourceManager(PPE::RHI::IFrameGraph& fg);
extern void UnitTest_VertexInput();
}
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
IRHIService::IRHIService() NOEXCEPT = default;
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void IRHIService::UnitTest() NOEXCEPT {
    using namespace RHI;

    PPE_LOG(RHI, Emphasis, "start frame graph unit tests...");

    UnitTest_Id();
    UnitTest_ImageDesc();
    UnitTest_ImageSwizzle();
    UnitTest_PixelFormat();
    UnitTest_VertexInput();
    UnitTest_ResourceManager(*FrameGraph());
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
