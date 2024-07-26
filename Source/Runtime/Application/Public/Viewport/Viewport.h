#pragma once

#include "Application_fwd.h"

#include "Viewport/Camera.h"

#include "Maths/ScalarRectangle.h"

#include "Thread/ThreadSafe.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EViewportFlags : u8 {
    Unknown = 0,
};
ENUM_FLAGS(EViewportFlags);
//----------------------------------------------------------------------------
class FViewport {
public:
    FViewport() = default;

    PPE_APPLICATION_API FViewport(
        const PMainWindow& window,
        const FRectangle2i& clientRect,
        EViewportFlags flags = Default ) NOEXCEPT;

    NODISCARD const PMainWindow& Window() const { return _data.LockShared()->Window; }

    NODISCARD const FRectangle2i& ClientRect() const { return _data.LockShared()->ClientRect; }
    PPE_APPLICATION_API void SetClientRect(const FRectangle2i& value);

    NODISCARD EViewportFlags Flags() const { return _data.LockShared()->Flags; }
    void SetFlags(EViewportFlags value) { _data.LockExclusive()->Flags = value; }

    NODISCARD PPE_APPLICATION_API FRectangle2f WindowClip() const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API int2 ScreenToClient(const int2& screenPos) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API int2 ClientToScreen(const int2& clientPos) const NOEXCEPT;

    NODISCARD PPE_APPLICATION_API float2 ClientToTexCoord(const int2& clientPos) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API int2 TexCoordToClient(const float2& texCoord) const NOEXCEPT;

private:
    struct FInternalData {
        PMainWindow Window;
        FRectangle2i ClientRect;
        EViewportFlags Flags{ Default };
    };

    TThreadSafe<FInternalData, EThreadBarrier::RWDataRaceCheck> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
