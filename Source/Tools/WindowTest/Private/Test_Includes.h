#pragma once

#include "WindowTestApp.h"

#include "RHI/FrameGraph.h"

#include "RHI/BufferDesc.h"
#include "RHI/DrawContext.h"
#include "RHI/EnumToString.h"
#include "RHI/ImageView.h"
#include "RHI/MemoryDesc.h"
#include "RHI/PipelineDesc.h"
#include "RHI/SamplerDesc.h"

#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformNotification.h"
#include "HAL/RHIService.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "Maths/PackedVectors.h"
#include "Memory/RefPtr.h"
#include "Meta/Assert.h"
#include "Meta/Utility.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_LOG_CATEGORY(, WindowTest)
//----------------------------------------------------------------------------
struct FExpectAssertInScope : Meta::FNonCopyable {
    FExpectAssertInScope() NOEXCEPT
    :   AssertHandler(SetAssertionHandler(&OnAssert))
    ,   AssertReleaseHandler(SetAssertionReleaseHandler(&OnAssert)) {
        NumAssertsTLS_() = 0;
    }

    ~FExpectAssertInScope() {
        SetAssertionHandler(AssertHandler);
        SetAssertionReleaseHandler(AssertReleaseHandler);
        Assert_NoAssume(NumAsserts() > 0);
    }

    u32 NumAsserts() const {
        return NumAssertsTLS_();
    }

private:
    FAssertionHandler AssertHandler;
    FAssertionHandler AssertReleaseHandler;

    static u32& NumAssertsTLS_() {
        static THREAD_LOCAL u32 GNumAssertsTLS{ 0 };
        return GNumAssertsTLS;
    }

    static bool OnAssert(const wchar_t* msg, const wchar_t* , unsigned ) {
        LOG(WindowTest, Info, L"Expected assert: '{0}'", MakeCStringView(msg));
        ++NumAssertsTLS_();
        return false; // no failure
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
