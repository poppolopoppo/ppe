#pragma once

#include "Core/Core.h"

#include "Core/IO/StreamProvider.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryViewReader : public IStreamReader {
public:
    MemoryViewReader() : _offsetI(0) {}
    MemoryViewReader(const MemoryView<const u8>& rawData) : _offsetI(0), _rawData(rawData) {}

    MemoryView<const u8> Eat(size_t sizeInBytes) {
        AssertRelease(sizeInBytes + _offsetI <= _rawData.SizeInBytes());
        const MemoryView<const u8> eaten = _rawData.SubRange(_offsetI, sizeInBytes);
        _offsetI += sizeInBytes;
        return eaten;
    }

    bool EatIFP(MemoryView<const u8> *peaten, size_t sizeInBytes) {
        Assert(peaten);
        if (sizeInBytes > _rawData.SizeInBytes() - _offsetI)
            return false;
        *peaten = Eat(sizeInBytes);
        return true;
    }

    virtual bool Eof() const override { return _offsetI >= _rawData.SizeInBytes(); }

    virtual bool IsSeekableI() const override { return true; }

    virtual std::streamoff TellI() const override { return checked_cast<std::streamsize>(_offsetI); }
    virtual bool SeekI(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) override;

    virtual std::streamsize SizeInBytes() const override { return checked_cast<std::streamsize>(_rawData.SizeInBytes()); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override;

    virtual bool Peek(char& ch) override;
    virtual bool Peek(wchar_t& wch) override;

    MemoryViewReader SubRange(size_t offset, size_t sizeInBytes) const {
        return MemoryViewReader(_rawData.SubRange(offset, sizeInBytes));
    }

private:
    size_t _offsetI;
    MemoryView<const u8> _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryViewWriter : public IStreamWriter {
public:
    MemoryViewWriter() : _size(0), _offsetO(0) {}
    MemoryViewWriter(const MemoryView<u8>& rawData) : _size(0), _offsetO(0), _rawData(rawData) {}

    virtual bool IsSeekableO() const override { return true; }

    virtual std::streamoff TellO() const override;
    virtual bool SeekO(std::streamoff offset, SeekOrigin policy = SeekOrigin::Begin) override;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override;
    virtual bool WriteSome(const void* storage, size_t eltsize, std::streamsize count) override;

    MemoryView<u8> Written() const { return _rawData.CutBefore(_size); }

    bool WriteAlignmentPadding(size_t boundary, u8 padvalue = 0);
    bool WriteAligned(const void* storage, std::streamsize sizeInBytes, size_t boundary);

    template <typename T>
    bool WritePODAligned(const T& pod, size_t boundary) { return WriteAligned(&pod, sizeof(T), boundary); }

    MemoryView<u8> Eat(size_t sizeInBytes) {
        AssertRelease(sizeInBytes + _offsetO <= _rawData.SizeInBytes());
        const MemoryView<u8> eaten = _rawData.SubRange(_offsetO, sizeInBytes);
        _offsetO += sizeInBytes;
        _size = std::max(_size, _offsetO);
        return eaten;
    }

    MemoryView<u8> EatAligned(size_t sizeInBytes, size_t boundary) {
        WriteAlignmentPadding(boundary);
        return Eat(sizeInBytes);
    }

private:
    size_t _size;
    size_t _offsetO;
    MemoryView<u8> _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core