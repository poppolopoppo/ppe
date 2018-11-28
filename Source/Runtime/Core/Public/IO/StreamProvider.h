#pragma once

#include "Core.h"

#include "IO/StreamPolicies.h"
#include "IO/String_fwd.h"
#include "IO/StringView.h"
#include "Memory/UniquePtr.h"
#include "Meta/AlignedStorage.h"

namespace PPE {
template <typename T, typename _Allocator>
class TRawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(StreamReader);
//----------------------------------------------------------------------------
class PPE_CORE_API IStreamReader {
public: // virtual interface
    virtual ~IStreamReader() {}

    virtual bool Eof() const = 0;

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const = 0;

    virtual std::streamoff TellI() const = 0;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) = 0;

    virtual std::streamsize SizeInBytes() const = 0;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) = 0;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) = 0;

    virtual class IBufferedStreamReader* ToBufferedI() { return nullptr; }

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
    virtual IBufferedStreamReader* ToBufferedI() override final { return this; }

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

    TMemoryView<char> ReadUntil(const TMemoryView<char>& storage, char expected);
    TMemoryView<wchar_t> ReadUntil(const TMemoryView<wchar_t>& storage, wchar_t expected);

    TMemoryView<char> ReadUntil(const TMemoryView<char>& storage, const TMemoryView<const char>& any);
    TMemoryView<wchar_t> ReadUntil(const TMemoryView<wchar_t>& storage, const TMemoryView<const wchar_t>& any);

    TMemoryView<char> ReadLine(const TMemoryView<char>& storage);
    TMemoryView<wchar_t> ReadLine(const TMemoryView<wchar_t>& storage);

    TMemoryView<char> ReadWord(const TMemoryView<char>& storage);
    TMemoryView<wchar_t> ReadWord(const TMemoryView<wchar_t>& storage);

    bool SeekI_FirstOf(char cmp);
    bool SeekI_FirstOf(wchar_t cmp);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(StreamWriter);
//----------------------------------------------------------------------------
class PPE_CORE_API IStreamWriter {
public: // virtual interface
    virtual ~IStreamWriter() {}

    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const = 0;

    virtual std::streamoff TellO() const = 0;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) = 0;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) = 0;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) = 0;

    virtual class IBufferedStreamWriter* ToBufferedO() { return nullptr; }

public: // helpers
    template <typename T, size_t _Dim>
    void WriteArray(const T(&staticArray)[_Dim]);

    void WriteView(const FStringView& str);
    void WriteView(const FWStringView& wstr);

    template <typename T>
    void WriteView(const TMemoryView<T>& data);
};
//----------------------------------------------------------------------------
class PPE_CORE_API IBufferedStreamWriter : public IStreamWriter {
    virtual class IBufferedStreamWriter* ToBufferedO() override final { return this; }

public:
    virtual void Flush() = 0;

public: // helpers
    template <typename T>
    void WritePOD(const T& pod);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(StreamReadWriter);
//----------------------------------------------------------------------------
class PPE_CORE_API EMPTY_BASES IStreamReadWriter : public IStreamReader, public IStreamWriter {
public:
    virtual ~IStreamReadWriter() {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "IO/StreamProvider-inl.h"
