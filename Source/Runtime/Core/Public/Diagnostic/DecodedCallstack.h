#pragma once

#include "Core.h"

#include "IO/String.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/AlignedStorage.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FCallstack;
//----------------------------------------------------------------------------
class PPE_API FDecodedCallstack {
public:
    friend class FCallstack;
    enum { MaxDeph = 46 };

    class FFrame {
    public:
        FFrame();
        FFrame(void* address, const wchar_t* symbol, const wchar_t* filename, size_t line);
        FFrame(void* address, FWString&& symbol, FWString&& filename, size_t line);
        ~FFrame();

        FFrame(FFrame&& rvalue);
        FFrame& operator =(FFrame&& rvalue);

        FFrame(const FFrame& other) = delete;
        FFrame& operator =(const FFrame& other) = delete;

        void* Address() const { return _address; }
        const FWString& Symbol() const { return _symbol; }
        const FWString& Filename() const { return _filename; }
        size_t Line() const { return _line; }

    private:
        void*   _address;
        FWString _symbol;
        FWString _filename;
        size_t  _line;
    };

    FDecodedCallstack();
    FDecodedCallstack(const FCallstack& callstack);
    ~FDecodedCallstack();

    FDecodedCallstack(FDecodedCallstack&& rvalue);
    FDecodedCallstack& operator =(FDecodedCallstack&& rvalue);

    FDecodedCallstack(const FDecodedCallstack& other) = delete;
    FDecodedCallstack& operator =(const FDecodedCallstack& other) = delete;

    size_t Hash() const { return _hash; }
    size_t Depth() const { return _depth; }

    TMemoryView<const FFrame> Frames() const {
        return TMemoryView<const FFrame>(reinterpret_cast<const FFrame*>(&_frames), _depth);
    }

private:
    // no ctor/dtor overhead for unused frames
    typedef ALIGNED_STORAGE(sizeof(FFrame) * MaxDeph, std::alignment_of<FFrame>::value)
        frames_type;

    size_t _hash;
    size_t _depth;
    frames_type _frames;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API FTextWriter& operator <<(FTextWriter& oss, const FDecodedCallstack::FFrame& frame);
PPE_API FWTextWriter& operator <<(FWTextWriter& oss, const FDecodedCallstack::FFrame& frame);
//----------------------------------------------------------------------------
PPE_API FTextWriter& operator <<(FTextWriter& oss, const FDecodedCallstack& decoded);
PPE_API FWTextWriter& operator <<(FWTextWriter& oss, const FDecodedCallstack& decoded);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
