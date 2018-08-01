#include "stdafx.h"

#include "Callstack.h"

#include "HAL/PlatformCallstack.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformHash.h"

#include "DecodedCallstack.h"
#include "Logger.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCallstack::FCallstack()
: _hash(0), _depth(0) {
#ifdef WITH_PPE_ASSERT
    FPlatformMemory::Memset(_frames, 0xCD, sizeof(_frames));
#endif
}
//----------------------------------------------------------------------------
FCallstack::FCallstack(size_t framesToSkip, size_t framesToCapture)
: FCallstack() {
    Capture(this, framesToSkip, framesToCapture);
}
//----------------------------------------------------------------------------
FCallstack::~FCallstack() {}
//----------------------------------------------------------------------------
FCallstack::FCallstack(const FCallstack& other)
: _hash(other._hash), _depth(other._depth) {
    FPlatformMemory::Memcpy(_frames, other._frames, sizeof(_frames));
}
//----------------------------------------------------------------------------
FCallstack& FCallstack::operator =(const FCallstack& other) {
    if (this != &other) {
        _hash = other._hash;
        _depth = other._depth;
        FPlatformMemory::Memcpy(_frames, other._frames, sizeof(_frames));
    }
    return *this;
}
//----------------------------------------------------------------------------
bool FCallstack::Decode(FDecodedCallstack* decoded) const {
    return Decode(decoded, _hash, Frames());
}
//----------------------------------------------------------------------------
bool FCallstack::Decode(FDecodedCallstack* decoded, size_t hash, const TMemoryView<void* const>& frames) {
    Assert(decoded);

    IF_CONSTEXPR(not FPlatformCallstack::HasSymbols)
        return false;

    decoded->_hash = hash;
    decoded->_depth = frames.size();

    FPlatformCallstack::FProgramCounterSymbolInfo symbolInfo;

    void* const* address = frames.data();
    auto frame = reinterpret_cast<FDecodedCallstack::FFrame *>(&decoded->_frames);
    for (size_t i = 0; i < frames.size(); ++i, ++frame, ++address) {
        Verify(FPlatformCallstack::ProgramCounterToSymbolInfo(&symbolInfo, *address));
        INPLACE_NEW(frame, FDecodedCallstack::FFrame)(*address, std::move(symbolInfo.Function), std::move(symbolInfo.Filename), symbolInfo.Line);
    }

    return true;
}
//----------------------------------------------------------------------------
void FCallstack::Capture(FCallstack* callstack, size_t framesToSkip, size_t framesToCapture) {
    Assert(callstack);
    callstack->_depth = Capture(
        MakeView(&callstack->_frames[0], &callstack->_frames[MaxDepth]),
        &callstack->_hash,
        framesToSkip,
        framesToCapture);
}
//----------------------------------------------------------------------------
size_t FCallstack::Capture(
    const TMemoryView<void*>& frames,
    size_t* backtraceHash,
    size_t framesToSkip,
    size_t framesToCapture) {
    Assert(frames.size());
    // backtraceHash is optional and can be null
    Assert(framesToCapture <= frames.size());
    Assert(framesToCapture <= MaxDepth);

    framesToCapture = Min(framesToCapture, MaxDepth);
    Assert(framesToCapture <= FPlatformCallstack::MaxStackDepth);

    const size_t depth = FPlatformCallstack::CaptureCallstack(frames.FirstNElements(framesToCapture), framesToSkip);

    if (backtraceHash)
        *backtraceHash = FPlatformHash::CRC32(0, frames.data(), depth * sizeof(void*));

    return depth;
}
//----------------------------------------------------------------------------
void FCallstack::SetFrames(const TMemoryView<void* const>& frames) {
    _hash = 0;
    const size_t n = Min(MaxDepth, frames.size());
    for (_depth = 0; _depth < n; ++_depth) {
        if (nullptr == frames[_depth]) break;
        _frames[_depth] = frames[_depth];
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
