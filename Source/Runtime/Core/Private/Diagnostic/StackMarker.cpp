#include "stdafx.h"

#include "Diagnostic/StackMarker.h"

#if USE_PPE_STACKMARKER

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

#   define USE_PPE_STACKMARKER_LOG 0 // turn to 1 to print each marker (very slow) %_NOCOMMIT%

namespace PPE {
#if USE_PPE_STACKMARKER_LOG
    LOG_CATEGORY_VERBOSITY(, StackMarker, All)
#else
    LOG_CATEGORY_VERBOSITY(, StackMarker, NoDebug)
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FStackInfo_ {
    uintptr_t Base;
    u32 Depth;
    u32 Offset;
    u32 MaxDepth;
    u32 MaxSize;
    u32 Committed;

    FStackInfo_()
    :   Depth(0)
    ,   Offset(0)
    ,   MaxDepth(0)
    ,   MaxSize(0)
    ,   Committed(0)
    {}

    void RefreshBase() {
        const FPlatformMemory::FStackUsage usage = FPlatformMemory::StackUsage();
        Base = uintptr_t(usage.BaseAddr) + usage.Committed;
        Committed = checked_cast<u32>(usage.Committed);
    }

    void Reset() {
        RefreshBase();
        MaxDepth = Depth;
        MaxSize = Offset;
    }

    u32 OffsetFromRSP(void* rsp) const {
        return checked_cast<u32>(Base - uintptr_t(rsp));
    }

    void OnEnterFrame(void* rsp) {
        if (Unlikely(0 == Depth))
            Reset();
        if (++Depth > MaxDepth)
            MaxDepth = Depth;
        Offset = OffsetFromRSP(rsp);
        if (Offset > MaxSize)
            MaxSize = Offset;
    }

    void OnLeaveFrame(void* rsp, u32 depth) {
        Assert_NoAssume(Depth >= depth);

        Offset = OffsetFromRSP(rsp);
        Depth = depth;

        if (0 == depth)
            Report();
    }

    NO_INLINE void Report() {
        RefreshBase();
        LOG(StackMarker, Info,
            L"stack usage : max depth = {0}, max size = {1:6f3} ({2:6f3} committed)",
            MaxDepth,
            Fmt::SizeInBytes(MaxSize),
            Fmt::SizeInBytes(Committed) );
    }

};
//----------------------------------------------------------------------------
static FStackInfo_& StackInfoTLS_() {
    ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(FStackInfo_, GInstanceTLS);
    return GInstanceTLS;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FThreadStackInfo ThreadStackInfo() NOEXCEPT {
    FStackInfo_& stack = StackInfoTLS_();
    return FThreadStackInfo{
        stack.Depth,
        stack.Offset,
        stack.MaxSize,
        FPlatformMemory::StackUsage() };
}
//----------------------------------------------------------------------------
u32 StackMarkerBegin(
    void* rsp,
    const char* caption,
    const char* function,
    const char* filename,
    u32 line) NOEXCEPT {
    FStackInfo_& stack = StackInfoTLS_();

    const u32 off = stack.Offset;
    stack.OnEnterFrame(rsp);

    Unused(caption);
    Unused(function);
    Unused(filename);
    Unused(line);

#if USE_PPE_STACKMARKER_LOG
#   if 0
    LOG(StackMarker, Debug,
        L" {0} [{1}] = {5:6f3}, {2} at {3}({4})",
        Fmt::Repeat(L' ', stack.Depth),
        MakeCStringView(caption),
        MakeCStringView(function),
        MakeCStringView(filename), line,
        Fmt::SizeInBytes(stack.Offset > off ? stack.Offset - off : off - stack.Offset));
#   else
    LOG(StackMarker, Debug,
        L" {0} [{1}] = {2:6f3} / {3:6f3} / {4:6f3}",
        Fmt::Repeat(L' ', stack.Depth),
        MakeCStringView(caption),
        Fmt::SizeInBytes(stack.Offset > off ? stack.Offset - off : off - stack.Offset),
        Fmt::SizeInBytes(stack.Offset),
        Fmt::SizeInBytes(stack.Committed) );
#   endif
#endif

    return (stack.Depth - 1);
}
//----------------------------------------------------------------------------
void StackMarkerEnd(void* rsp, u32 depth) NOEXCEPT {
    FStackInfo_& stack = StackInfoTLS_();
    stack.OnLeaveFrame(rsp, depth);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_STACKMARKER
