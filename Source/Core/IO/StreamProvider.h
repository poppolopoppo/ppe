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
    All         = 0xFF,
};
//----------------------------------------------------------------------------
class IStreamReader {
public: // virtual interface
    virtual ~IStreamReader() {}

    virtual bool Eof() const = 0;

    virtual bool IsSeekableI(SeekOrigin origin = SeekOrigin::All) const = 0;

    virtual std::streamoff TellI() const = 0;
    virtual bool SeekI(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) = 0;

    virtual std::streamsize SizeInBytes() const = 0;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) = 0;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) = 0;

    virtual bool Peek(char& ch) = 0;
    virtual bool Peek(wchar_t& ch) = 0;

public: // read helpers
    template <typename T>
    bool ReadPOD(T* pod);

    template <typename T, size_t _Dim>
    bool ReadArray(T(&staticArray)[_Dim]);

    template <typename T, typename _Allocator>
    void ReadAll(RawStorage<T, _Allocator>& dst);

    template <typename T>
    bool ReadView(const MemoryView<T>& dst);

    template <typename T>
    bool ExpectPOD(const T& pod);

    MemoryView<char> ReadUntil(const MemoryView<char>& storage, char expected);
    MemoryView<wchar_t> ReadUntil(const MemoryView<wchar_t>& storage, wchar_t expected);

    MemoryView<char> ReadUntil(const MemoryView<char>& storage, const MemoryView<const char>& any);
    MemoryView<wchar_t> ReadUntil(const MemoryView<wchar_t>& storage, const MemoryView<const wchar_t>& any);

    MemoryView<char> ReadLine(const MemoryView<char>& storage);
    MemoryView<wchar_t> ReadLine(const MemoryView<wchar_t>& storage);

    MemoryView<char> ReadWord(const MemoryView<char>& storage);
    MemoryView<wchar_t> ReadWord(const MemoryView<wchar_t>& storage);

    bool SeekI_FirstOf(char cmp);
    bool SeekI_FirstOf(wchar_t cmp);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IStreamWriter {
public: // virtual interface
    virtual ~IStreamWriter() {}

    virtual bool IsSeekableO(SeekOrigin origin = SeekOrigin::All) const = 0;

    virtual std::streamoff TellO() const = 0;
    virtual bool SeekO(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) = 0;

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
    void WriteView(const MemoryView<T>& data);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicStreamReader : public IStreamReader {
public:
    typedef std::basic_istream<_Char, _Traits> stream_type;

    explicit BasicStreamReader(stream_type& iss) : _iss(iss) { Assert(!_iss.bad()); }
    virtual ~BasicStreamReader() { Assert(!_iss.bad()); }

    virtual bool Eof() const override { return _iss.eof(); }

    virtual bool IsSeekableI(SeekOrigin ) const override { return true; }

    virtual std::streamoff TellI() const override {
        Assert(!_iss.bad());
        return _iss.tellg();
    }

    virtual bool SeekI(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) override {
        Assert(!_iss.bad());
        _iss.seekg(offset, int(origin));
        Assert(!_iss.bad());
        return true;
    }

    virtual std::streamsize SizeInBytes() const override {
        Assert(!_iss.bad());
        const std::streamoff off = _iss.tellg();
        _iss.seekg(0, int(SeekOrigin::End));
        const std::streamsize sz(_iss.tellg());
        _iss.seekg(off, int(SeekOrigin::Begin));
        return sz;
    }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override {
        Assert(!_iss.bad());
        _iss.read((_Char*)storage, sizeInBytes/(sizeof(_Char)));
        return (false == _iss.bad());
    }

    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override {
        Assert(!_iss.bad());
        return _iss.readsome((_Char*)storage, (count*eltsize)/(sizeof(_Char)));
    }

    virtual bool Peek(char& ch) override {
        Assert(!_iss.bad());
        const auto read = _iss.peek();
        ch = char(read);
        return (read != Eof_);
    }

    virtual bool Peek(wchar_t& wch) override {
        Assert(!_iss.bad());
        const auto read = _iss.peek();
        wch = wchar_t(read);
        return (read != Eof_);
    }

private:
    typedef typename stream_type::traits_type traits_type;
    static constexpr auto Eof_ = traits_type::eof();

    stream_type& _iss;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicStreamWriter : public IStreamWriter {
public:
    typedef std::basic_ostream<_Char, _Traits> stream_type;

    explicit BasicStreamWriter(stream_type& oss) : _oss(oss) { Assert(!_oss.bad()); }
    virtual ~BasicStreamWriter() { Assert(!_oss.bad()); }

    virtual bool IsSeekableO(SeekOrigin ) const override { return true; }

    virtual std::streamoff TellO() const override {
        Assert(!_oss.bad());
        return _oss.tellp();
    }

    virtual bool SeekO(std::streamoff offset, SeekOrigin policy = SeekOrigin::Begin) override {
        Assert(!_oss.bad());
        _oss.seekp(offset, int(policy));
        Assert(!_oss.bad());
        return true;
    }

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override {
        Assert(!_oss.bad());
        _oss.write((const _Char*)storage, (sizeInBytes)/(sizeof(_Char)));
        return (false == _oss.bad());
    }

    virtual bool WriteSome(const void* storage, size_t eltsize, std::streamsize count) override {
        Assert(!_oss.bad());
        _oss.write((const _Char*)storage, (eltsize*count)/(sizeof(_Char)));
        return (false == _oss.bad());
    }

private:
    stream_type& _oss;
};
//----------------------------------------------------------------------------
inline BasicStreamReader<char> StdinReader() { return BasicStreamReader<char>(std::cin); }
inline BasicStreamWriter<char> StdoutWriter() { return BasicStreamWriter<char>(std::cout); }
inline BasicStreamWriter<char> StderrWriter() { return BasicStreamWriter<char>(std::cerr); }
//----------------------------------------------------------------------------
inline BasicStreamReader<wchar_t> WStdinReader() { return BasicStreamReader<wchar_t>(std::wcin); }
inline BasicStreamWriter<wchar_t> WStdoutWriter() { return BasicStreamWriter<wchar_t>(std::wcout); }
inline BasicStreamWriter<wchar_t> WStderrWriter() { return BasicStreamWriter<wchar_t>(std::wcerr); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/StreamProvider-inl.h"
