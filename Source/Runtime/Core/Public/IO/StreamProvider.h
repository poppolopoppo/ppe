#pragma once

#include "Core.h"

#include "Container/RawStorage_fwd.h"
#include "IO/Stream_fwd.h"
#include "IO/StreamPolicies.h"
#include "IO/String_fwd.h"
#include "IO/StringView.h"
#include "Memory/UniquePtr.h"
#include "Meta/AlignedStorage.h"
#include "Misc/Function_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(StreamReader);
//----------------------------------------------------------------------------
class PPE_CORE_API IStreamReader {
public: // virtual interface
    virtual ~IStreamReader() = default;

    virtual bool Eof() const NOEXCEPT = 0;

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT = 0;

    virtual std::streamoff TellI() const NOEXCEPT = 0;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) = 0;

    virtual std::streamsize SizeInBytes() const NOEXCEPT = 0;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) = 0;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) = 0;

    virtual class IBufferedStreamReader* ToBufferedI() NOEXCEPT { return nullptr; }

public: // helpers
    template <typename T, size_t _Dim>
    bool ReadArray(T(&staticArray)[_Dim]);

    template <typename T, typename _Allocator>
    void ReadAll(TRawStorage<T, _Allocator>& dst);

    template <typename T>
    bool ReadAt(const TMemoryView<T>& dst, std::streamoff absolute);
    template <typename T, typename _Allocator>
    bool ReadAt(TRawStorage<T, _Allocator>& dst, std::streamoff absolute, std::streamsize sizeInBytes);

    template <typename T>
    bool ReadView(const TMemoryView<T>& dst);
};
//----------------------------------------------------------------------------
class PPE_CORE_API IBufferedStreamReader : public IStreamReader {
    virtual IBufferedStreamReader* ToBufferedI() NOEXCEPT override final { return this; }

public:
    virtual bool Peek(char& ch) = 0;
    virtual bool Peek(wchar_t& ch) = 0;

    // if you're reading a large chunk and need to seek, but don't want to pollute the read buffer
    virtual bool ReadAt_SkipBuffer(const FRawMemory& storage, std::streamoff absolute) = 0;

public: // helpers
    template <typename T>
    bool ReadPOD(T* pod);

    template <typename T>
    bool ExpectPOD(const T& pod);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(StreamWriter);
//----------------------------------------------------------------------------
class PPE_CORE_API IStreamWriter {
public: // virtual interface
    virtual ~IStreamWriter() = default;

    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT = 0;

    virtual std::streamoff TellO() const NOEXCEPT = 0;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) = 0;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) = 0;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) = 0;

    virtual class IBufferedStreamWriter* ToBufferedO() NOEXCEPT { return nullptr; }

public: // helpers
    template <typename T, size_t _Dim>
    void WriteArray(const T(&staticArray)[_Dim]);

    template <typename T>
    void WritePOD(const T& pod);

    void WriteView(const FStringView& str);
    void WriteView(const FWStringView& wstr);

    void WriteView(FStringLiteral str) { WriteView(str.MakeView()); }
    void WriteView(FWStringLiteral wstr) { WriteView(wstr.MakeView()); }

    template <typename T>
    void WriteView(const TMemoryView<T>& data);
};
//----------------------------------------------------------------------------
class PPE_CORE_API IBufferedStreamWriter : public IStreamWriter {
    virtual class IBufferedStreamWriter* ToBufferedO() NOEXCEPT override final { return this; }

public:
    using read_f = TFunction<size_t(const FRawMemory&)>;
    virtual size_t StreamCopy(const read_f& read, size_t blockSz) = 0;

    virtual void Flush() = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(StreamReadWriter);
//----------------------------------------------------------------------------
class PPE_CORE_API EMPTY_BASES IStreamReadWriter : public IStreamReader, public IStreamWriter {
public:
    virtual ~IStreamReadWriter() = default;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "IO/StreamProvider-inl.h"
