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
        AssertRelease(sizeInBytes <= _rawData.SizeInBytes() - _offsetI);
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

    virtual std::streamoff TellI() const override { return checked_cast<std::streamsize>(_offsetI); }
    virtual bool SeekI(std::streamoff offset, SeekOrigin origin = SeekOrigin::Begin) override;

    virtual std::streamsize SizeInBytes() const override { return checked_cast<std::streamsize>(_rawData.SizeInBytes()); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override;

    virtual char PeekChar() override;
    virtual wchar_t PeekCharW() override;

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
} //!namespace Core