#pragma once

#include "Core.h"
#include "MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DecodedCallstack;
//----------------------------------------------------------------------------
class ALIGN(16) Callstack {
public:
    enum { MaxDeph = 46};
    // 46 * 4 + 4 + 4 = 192 -> align on 16, no overhead (32 bits)
    // 46 * 8 + 8 + 8 = 384 -> align on 16, no overhead (64 bits)

    Callstack();
    Callstack(size_t framesToSkip, size_t framesToCapture);
    ~Callstack();

    Callstack(const Callstack& other);
    Callstack& operator =(const Callstack& other);

    size_t Hash() const { return _hash; }
    size_t Depth() const { return _depth; }

    MemoryView<void*> Frames() { return MakeView(&_frames[0], &_frames[_depth]); }
    MemoryView<void* const> Frames() const { return MakeView(&_frames[0], &_frames[_depth]); }

    static void Capture(Callstack* callstack, size_t framesToSkip, size_t framesToCapture);
    static size_t Capture(
        const MemoryView<void*>& frames,
        size_t* backtraceHash,
        size_t framesToSkip,
        size_t framesToCapture
        );

    void Decode(DecodedCallstack* decoded) const;
    static void Decode(DecodedCallstack* decoded, size_t hash, const MemoryView<void* const>& frames);

    static void Start();
    static void ReloadSymbols();
    static void Shutdown();

private:
    size_t _hash;
    size_t _depth;
    void* _frames[MaxDeph];
};
//----------------------------------------------------------------------------
inline size_t hash_value(const Callstack& callstack) {
    return callstack.Hash();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
