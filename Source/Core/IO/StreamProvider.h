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

    template <typename T>
    void WriteView(const MemoryView<const T>& data);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicStreamReader : public IStreamReader {
public:
    typedef std::basic_istream<_Char, _Traits> stream_type;

    explicit BasicStreamReader(stream_type& iss) : _iss(iss) {}
    virtual ~BasicStreamReader() {}

    virtual bool Eof() const override { return _iss.eof(); }

    virtual std::streamoff TellI() const override { Assert(!_iss.bad()); return _iss.tellg(); }
    virtual bool SeekI(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) override { Assert(!_iss.bad()); _iss.seekg(offset, int(origin)); return true; }

    virtual std::streamsize SizeInBytes() const override {
        Assert(!_iss.bad());
        const std::streamoff off = _iss.tellg();
        _iss.seekg(0, int(SeekOrigin::End));
        const std::streamsize sz(_iss.tellg());
        _iss.seekg(off, int(SeekOrigin::Begin));
        return sz;
    }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override { Assert(!_iss.bad()); _iss.read((_Char*)storage, sizeInBytes/(sizeof(_Char))); return (false == _iss.bad()); }
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override { Assert(!_iss.bad()); return _iss.readsome((_Char*)storage, (count*eltsize)/(sizeof(_Char))); }

    virtual char PeekChar() override { Assert(!_iss.bad()); return _iss.peek(); }
    virtual wchar_t PeekCharW() override { AssertNotReached(); return _iss.peek(); }

private:
    stream_type& _iss;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicStreamWriter : public IStreamWriter {
public:
    typedef std::basic_ostream<_Char, _Traits> stream_type;

    explicit BasicStreamWriter(stream_type& oss) : _oss(oss) {}
    virtual ~BasicStreamWriter() {}

    virtual std::streamoff TellO() const override { Assert(!_oss.bad()); return _oss.tellp(); }
    virtual bool SeekO(std::streamoff offset, SeekOrigin policy = SeekOrigin::Begin) override { Assert(!_oss.bad()); _oss.seekp(offset, int(policy)); return true; }

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override { Assert(!_oss.bad()); _oss.write((const _Char*)storage, (sizeInBytes)/(sizeof(_Char))); return false == _oss.bad(); }
    virtual bool WriteSome(const void* storage, size_t eltsize, std::streamsize count) { Assert(!_oss.bad()); _oss.write((const _Char*)storage, (eltsize*count)/(sizeof(_Char))); return false == _oss.bad(); }

private:
    stream_type& _oss;
};
//----------------------------------------------------------------------------
inline BasicStreamReader<char> StdinReader() { return BasicStreamReader<char>(std::cin); }
inline BasicStreamWriter<char> StdoutWriter() { return BasicStreamWriter<char>(std::cout); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/StreamProvider-inl.h"
