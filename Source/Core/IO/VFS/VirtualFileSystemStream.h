#pragma once

#include "Core/Core.h"

#include "Core/Memory/UniqueView.h"

#include <iosfwd>

namespace Core {
class Filename;
template <typename T, typename _Allocator>
class RawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVirtualFileSystemIStream {
protected:
    IVirtualFileSystemIStream() {}
public:
    virtual ~IVirtualFileSystemIStream() {}

    IVirtualFileSystemIStream(const IVirtualFileSystemIStream&) = delete;
    IVirtualFileSystemIStream& operator =(const IVirtualFileSystemIStream&) = delete;

    virtual std::streamoff TellI() = 0;
    virtual void SeekI(std::streamoff offset) = 0;

    virtual void Read(void* storage, std::streamsize count) = 0;
    virtual std::streamsize ReadSome(void* storage, std::streamsize count) = 0;

    virtual char PeekChar() = 0;

    virtual bool Eof() const = 0;
    virtual std::streamsize Size() const = 0;

    template <typename T>
    bool ReadPOD(T* pod);

    template <typename T, size_t _Dim>
    bool ReadArray(T(&staticArray)[_Dim]);

    template <typename T, typename _Allocator>
    void ReadAll(RawStorage<T, _Allocator>& dst);

    std::streamsize ReadLine(char *storage, std::streamsize capacity);
    std::streamsize ReadWord(char *storage, std::streamsize capacity);

    template <size_t _Capacity>
    std::streamsize ReadLine(char (&storage)[_Capacity]) { return ReadLine(storage, _Capacity); }
    template <size_t _Capacity>
    std::streamsize ReadWord(char (&storage)[_Capacity]) { return ReadWord(storage, _Capacity); }

    bool SeekI_FirstOf(char ch);
};
//----------------------------------------------------------------------------
class IVirtualFileSystemOStream {
protected:
    IVirtualFileSystemOStream() {}
public:
    virtual ~IVirtualFileSystemOStream() {}

    IVirtualFileSystemOStream(const IVirtualFileSystemOStream&) = delete;
    IVirtualFileSystemOStream& operator =(const IVirtualFileSystemOStream&) = delete;

    virtual std::streamoff TellO() = 0;
    virtual void SeekO(std::streamoff offset) = 0;
    virtual void Write(const void* storage, std::streamsize count) = 0;

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
class IVirtualFileSystemIOStream :
    public IVirtualFileSystemIStream
,   public IVirtualFileSystemOStream {
protected:
    IVirtualFileSystemIOStream() {}
public:
    virtual ~IVirtualFileSystemIOStream() {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/VFS/VirtualFileSystemStream-inl.h"
