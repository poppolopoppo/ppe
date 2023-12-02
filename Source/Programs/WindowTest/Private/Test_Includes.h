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
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
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
    ,   AssertReleaseHandler(SetAssertionReleaseHandler(&OnAssertRelease)) {
        NumAssertsTLS_() = 0;
    }

    ~FExpectAssertInScope() {
        SetAssertionHandler(AssertHandler);
        SetAssertionReleaseHandler(AssertReleaseHandler);
        Assert_NoAssume(NumAsserts() > 0);
    }

    NODISCARD u32 NumAsserts() const {
        return NumAssertsTLS_();
    }

private:
    FAssertionHandler AssertHandler;
    FReleaseAssertionHandler AssertReleaseHandler;

    static u32& NumAssertsTLS_() {
        static THREAD_LOCAL u32 GNumAssertsTLS{ 0 };
        return GNumAssertsTLS;
    }

    static bool OnAssert(const char* msg, const char* file, unsigned line, bool isEnsure) {
        Unused(msg, file, line);
        PPE_LOG(WindowTest, Info, "Expected {3} failed: '{0}' in {1}:{2}", MakeCStringView(msg), MakeCStringView(file), line,
            isEnsure ? MakeStringView(L"ensure") : MakeStringView(L"assert"));
        ++NumAssertsTLS_();
        return false; // no failure
    }
    static bool OnAssertRelease(const char* msg, const char* file, unsigned line) {
        Unused(msg, file, line);
        PPE_LOG(WindowTest, Info, "Expected release assert failed: '{0}' in {1}:{2}", MakeCStringView(msg), MakeCStringView(file), line);
        ++NumAssertsTLS_();
        return false; // no failure
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
