#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
template <typename T, typename _Allocator>
class TRawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ESeekOrigin {
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

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const = 0;

    virtual std::streamoff TellI() const = 0;
    virtual bool SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) = 0;

    virtual std::streamsize SizeInBytes() const = 0;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) = 0;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) = 0;

    virtual bool Peek(char& ch) = 0;
    virtual bool Peek(wchar_t& ch) = 0;

public: // read helpers
    template <typename T>
    bool ReadPOD(T* pod);

    template <typename T, size_t _Dim>
    bool ReadArray(T(&staticArray)[_Dim]);

    template <typename T, typename _Allocator>
    void ReadAll(TRawStorage<T, _Allocator>& dst);

    template <typename T>
    bool ReadView(const TMemoryView<T>& dst);

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
class IStreamWriter {
public: // virtual interface
    virtual ~IStreamWriter() {}

    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const = 0;

    virtual std::streamoff TellO() const = 0;
    virtual bool SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) = 0;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) = 0;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) = 0;

public: // helpers
    template <typename T>
    void WritePOD(const T& pod);

    template <typename T, size_t _Dim>
    void WriteArray(const T(&staticArray)[_Dim]);

    void WriteView(const FStringView& str);
    void WriteView(const FWStringView& wstr);

    template <typename T>
    void WriteView(const TMemoryView<T>& data);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class TBasicStreamReader : public IStreamReader {
public:
    typedef std::basic_istream<_Char, _Traits> stream_type;

    explicit TBasicStreamReader(stream_type& iss) : _iss(iss) { Assert(!_iss.bad()); }
    virtual ~TBasicStreamReader() { Assert(!_iss.bad()); }

    virtual bool Eof() const override { return _iss.eof(); }

    virtual bool IsSeekableI(ESeekOrigin ) const override { return true; }

    virtual std::streamoff TellI() const override;
    virtual bool SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override;

    virtual std::streamsize SizeInBytes() const override;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override;

    virtual bool Peek(char& ch) override;
    virtual bool Peek(wchar_t& wch) override;

private:
    typedef typename stream_type::traits_type traits_type;
    static constexpr auto Eof_ = traits_type::eof();

    stream_type& _iss;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class TBasicStreamWriter : public IStreamWriter {
public:
    typedef std::basic_ostream<_Char, _Traits> stream_type;

    explicit TBasicStreamWriter(stream_type& oss) : _oss(oss) { Assert(!_oss.bad()); }
    virtual ~TBasicStreamWriter() { Assert(!_oss.bad()); }

    virtual bool IsSeekableO(ESeekOrigin ) const override { return true; }

    virtual std::streamoff TellO() const override;
    virtual bool SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin);

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override;

private:
    stream_type& _oss;
};
//----------------------------------------------------------------------------
inline TBasicStreamReader<char> StdinReader() { return TBasicStreamReader<char>(std::cin); }
inline TBasicStreamWriter<char> StdoutWriter() { return TBasicStreamWriter<char>(std::cout); }
inline TBasicStreamWriter<char> StderrWriter() { return TBasicStreamWriter<char>(std::cerr); }
//----------------------------------------------------------------------------
inline TBasicStreamReader<wchar_t> WStdinReader() { return TBasicStreamReader<wchar_t>(std::wcin); }
inline TBasicStreamWriter<wchar_t> WStdoutWriter() { return TBasicStreamWriter<wchar_t>(std::wcout); }
inline TBasicStreamWriter<wchar_t> WStderrWriter() { return TBasicStreamWriter<wchar_t>(std::wcerr); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/StreamProvider-inl.h"
