#pragma once

#include "Core.h"

#include "Allocator/Alloca.h"
#include "IO/StreamProvider.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_STREAMWRITER(_NAME, _COUNT) \
    MALLOCA(u8, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::FMemoryViewWriter _NAME(CONCAT(_Alloca_, _NAME).MakeView())
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMemoryViewReader : public IBufferedStreamReader {
public:
    FMemoryViewReader() : _offsetI(0) {}

    template <typename T>
    explicit FMemoryViewReader(const TMemoryView<const T>& rawData)
        : _offsetI(0)
        , _rawData(rawData.template Cast<const u8>())
    {}

    TMemoryView<const u8> Eat(size_t sizeInBytes) {
        AssertRelease(sizeInBytes + _offsetI <= _rawData.SizeInBytes());
        const TMemoryView<const u8> eaten = _rawData.SubRange(_offsetI, sizeInBytes);
        _offsetI += sizeInBytes;
        return eaten;
    }

    bool EatIFP(TMemoryView<const u8> *peaten, size_t sizeInBytes) {
        Assert(peaten);
        if (sizeInBytes > _rawData.SizeInBytes() - _offsetI)
            return false;
        *peaten = Eat(sizeInBytes);
        return true;
    }

    FMemoryViewReader SubRange(size_t offset, size_t sizeInBytes) const {
        return FMemoryViewReader(_rawData.SubRange(offset, sizeInBytes));
    }

public: // IStreamReader
    virtual bool Eof() const override final { return _offsetI >= _rawData.SizeInBytes(); }

    virtual bool IsSeekableI(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellI() const override final { return checked_cast<std::streamsize>(_offsetI); }
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final { return checked_cast<std::streamsize>(_rawData.SizeInBytes()); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamReader
    virtual bool Peek(char& ch) override final;
    virtual bool Peek(wchar_t& wch) override final;

    virtual bool ReadAt_SkipBuffer(const FRawMemory& storage, std::streamoff absolute) override final;

private:
    size_t _offsetI;
    FRawMemoryConst _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMemoryViewWriter : public IBufferedStreamWriter {
public:
    FMemoryViewWriter() : _size(0), _offsetO(0) {}

    template <typename T>
    FMemoryViewWriter(T* ptr, size_t count)
        : FMemoryViewWriter(TMemoryView<T>(ptr, count))
    {}

    template <typename T, size_t _Dim>
    FMemoryViewWriter(T(&staticArray)[_Dim])
        : FMemoryViewWriter(MakeView(staticArray))
    {}

    template <typename T>
    explicit FMemoryViewWriter(const TMemoryView<T>& rawData)
        : _size(0), _offsetO(0)
        , _rawData(rawData.template Cast<u8>())
    {}

    size_t size() const { return _size; }

    TMemoryView<const u8> Written() const { return _rawData.CutBefore(_size); }
    TMemoryView<const u8> WrittenSince(std::streamoff off) const {
        const size_t o = checked_cast<size_t>(off);
        Assert(o <= _size);
        return _rawData.SubRange(o, _size - o);
    }

    bool WriteAlignmentPadding(size_t boundary, u8 padvalue = 0);
    bool WriteAligned(const void* storage, std::streamsize sizeInBytes, size_t boundary);

    template <typename T>
    bool WritePODAligned(const T& pod, size_t boundary) { return WriteAligned(&pod, sizeof(T), boundary); }

    TMemoryView<u8> Eat(size_t sizeInBytes) {
        AssertRelease(sizeInBytes + _offsetO <= _rawData.SizeInBytes());
        const TMemoryView<u8> eaten = _rawData.SubRange(_offsetO, sizeInBytes);
        _offsetO += sizeInBytes;
        _size = std::max(_size, _offsetO);
        return eaten;
    }

    TMemoryView<u8> EatAligned(size_t sizeInBytes, size_t boundary) {
        WriteAlignmentPadding(boundary);
        return Eat(sizeInBytes);
    }

    void Reset() {
        _offsetO = _size = 0;
    }

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

public: // IBufferedStreamWriter
    using typename IBufferedStreamWriter::read_f;
    virtual size_t StreamCopy(const read_f& read, size_t blockSz) override final;

    virtual void Flush() override final {}

private:
    size_t _size;
    size_t _offsetO;
    FRawMemory _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
