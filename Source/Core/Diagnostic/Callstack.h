#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDecodedCallstack;
//----------------------------------------------------------------------------
class ALIGN(16) FCallstack {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxDepth, 46);
    // 46 * 4 + 4 + 4 = 192 -> align on 16, no overhead (32 bits)
    // 46 * 8 + 8 + 8 = 384 -> align on 16, no overhead (64 bits)

    FCallstack();
    FCallstack(size_t framesToSkip, size_t framesToCapture);
    ~FCallstack();

    FCallstack(const FCallstack& other);
    FCallstack& operator =(const FCallstack& other);

    size_t Hash() const { return _hash; }
    size_t Depth() const { return _depth; }

    TMemoryView<void*> Frames() { return MakeView(&_frames[0], &_frames[_depth]); }
    TMemoryView<void* const> Frames() const { return MakeView(&_frames[0], &_frames[_depth]); }

    static void Capture(FCallstack* callstack, size_t framesToSkip, size_t framesToCapture);
    static size_t Capture(
        const TMemoryView<void*>& frames,
        size_t* backtraceHash,
        size_t framesToSkip,
        size_t framesToCapture
        );

    void Decode(FDecodedCallstack* decoded) const;
    static void Decode(FDecodedCallstack* decoded, size_t hash, const TMemoryView<void* const>& frames);

    void SetFrames(const TMemoryView<void* const>& frames);

    static void Start();
    static void ReloadSymbols();
    static void Shutdown();

private:
    size_t _hash;
    size_t _depth;
    void* _frames[MaxDepth];
};
//----------------------------------------------------------------------------
inline hash_t hash_value(const FCallstack& callstack) {
    return callstack.Hash();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
