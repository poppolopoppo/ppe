#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
template <typename T, typename _Allocator>
class RawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class SeekOrigin {
    Begin       = 0,
    Relative    = 1,
    End         = 2,
};
//----------------------------------------------------------------------------
class IStreamReader {
public: // virtual interface
    virtual ~IStreamReader() {}

    virtual bool Eof() const = 0;

    virtual std::streamoff TellI() const = 0;
    virtual bool SeekI(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) = 0;

    virtual std::streamsize SizeInBytes() const = 0;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) = 0;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) = 0;

    virtual char PeekChar() = 0;
    virtual wchar_t PeekCharW() = 0;

public: // read helpers
    template <typename T>
    bool ReadPOD(T* pod);

    template <typename T, size_t _Dim>
    bool ReadArray(T(&staticArray)[_Dim]);

    template <typename T, typename _Allocator>
    void ReadAll(RawStorage<T, _Allocator>& dst);

    template <typename T>
    bool ExpectPOD(const T& pod);

    std::streamsize ReadLine(char *storage, std::streamsize capacity);
    std::streamsize ReadLine(wchar_t *storage, std::streamsize capacity);
    std::streamsize ReadWord(char *storage, std::streamsize capacity);
    std::streamsize ReadWord(wchar_t *storage, std::streamsize capacity);

    std::streamsize ReadLine(const MemoryView<char>& storage) { return ReadLine(storage.Pointer(), storage.size()); }
    std::streamsize ReadLine(const MemoryView<wchar_t>& storage) { return ReadLine(storage.Pointer(), storage.size()); }
    std::streamsize ReadWord(const MemoryView<char>& storage) { return ReadWord(storage.Pointer(), storage.size()); }
    std::streamsize ReadWord(const MemoryView<wchar_t>& storage) { return ReadWord(storage.Pointer(), storage.size()); }

    template <size_t _Capacity>
    std::streamsize ReadLine(char (&storage)[_Capacity]) { return ReadLine(storage, _Capacity); }
    template <size_t _Capacity>
    std::streamsize ReadLine(wchar_t (&storage)[_Capacity]) { return ReadLine(storage, _Capacity); }
    template <size_t _Capacity>
    std::streamsize ReadWord(char (&storage)[_Capacity]) { return ReadWord(storage, _Capacity); }
    template <size_t _Capacity>
    std::streamsize ReadWord(wchar_t (&storage)[_Capacity]) { return ReadWord(storage, _Capacity); }

    bool SeekI_FirstOf(char cmp);
    bool SeekI_FirstOf(wchar_t cmp);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IStreamWriter {
public: // virtual interface
    virtual ~IStreamWriter() {}

    virtual std::streamoff TellO() const = 0;
    virtual bool SeekO(std::streamoff offset, SeekOrigin policy = SeekOrigin::Begin) = 0;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) = 0;
    virtual bool WriteSome(const void* storage, size_t eltsize, std::streamsize count) = 0;

public: // helpers
    template <typename T>
    void WritePOD(const T& pod);

    template <typename T, size_t _Dim>
    void WriteArray(const T(&staticArray)[_Dim]);

    template <size_t _Dim>
    void WriteCStr(const char (&cstr)[_Dim]);
    template <size_t _Dim>
    void WriteCStr(const wchar_t (&wcstr)[_Dim]);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/StreamProvider-inl.h"
