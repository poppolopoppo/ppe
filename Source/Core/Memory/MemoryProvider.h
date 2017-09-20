#pragma once

#include "Core/Core.h"

#include "Core/IO/StreamProvider.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryViewReader : public IBufferedStreamReader {
public:
    FMemoryViewReader() : _offsetI(0) {}
    explicit FMemoryViewReader(const TMemoryView<const u8>& rawData) : _offsetI(0), _rawData(rawData) {}

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

    virtual bool Eof() const override final { return _offsetI >= _rawData.SizeInBytes(); }

    virtual bool IsSeekableI(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellI() const override final { return checked_cast<std::streamsize>(_offsetI); }
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final { return checked_cast<std::streamsize>(_rawData.SizeInBytes()); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

    virtual bool Peek(char& ch) override final;
    virtual bool Peek(wchar_t& wch) override final;

    FMemoryViewReader SubRange(size_t offset, size_t sizeInBytes) const {
        return FMemoryViewReader(_rawData.SubRange(offset, sizeInBytes));
    }

private:
    size_t _offsetI;
    TMemoryView<const u8> _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryViewWriter : public IBufferedStreamWriter {
public:
    FMemoryViewWriter() : _size(0), _offsetO(0) {}
    explicit FMemoryViewWriter(const TMemoryView<u8>& rawData) : _size(0), _offsetO(0), _rawData(rawData) {}

    virtual bool IsSeekableO(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

    TMemoryView<u8> Written() const { return _rawData.CutBefore(_size); }

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

public:
    virtual void Flush() override final {}

private:
    size_t _size;
    size_t _offsetO;
    TMemoryView<u8> _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
