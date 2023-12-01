#pragma once

#include "Core_fwd.h"

#include "IO/StreamProvider.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Can be used as a null pipe or to count memory used by a write process
//----------------------------------------------------------------------------
class FDummyStreamWriter final : public IStreamWriter {
public:
    FSizeInBytes SizeInBytes() const { return FSizeInBytes(checked_cast<size_t>(_capacity)); }

    void Reset() {
        _offset = _capacity = 0;
    }

    virtual bool IsSeekableO(ESeekOrigin = ESeekOrigin::All) const NOEXCEPT override {
        return true;
    }
    virtual std::streamoff TellO() const NOEXCEPT override final {
        return _offset;
    }
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final {
        switch (origin) {
        case ESeekOrigin::Begin:
            if (offset > _capacity)
                return -1;
            _offset = offset;
            break;
        case ESeekOrigin::Relative:
            if (_offset + offset < 0 ||
                _offset + offset > _capacity )
                return -1;
            _offset += offset;
            break;
        case ESeekOrigin::End:
            if (_capacity + offset < 0 ||
                _capacity + offset > _capacity )
                return -1;
            _offset = _capacity + offset;
            break;
        default:
            AssertNotImplemented();
        }
        return _offset;
    }
    virtual bool Write(const void*, std::streamsize sizeInBytes) override final {
        _offset += sizeInBytes;
        _capacity = Max(_offset, _capacity);
        return true;
    }
    virtual size_t WriteSome(const void* storage, size_t eltSize, size_t count) override final {
        const size_t sizeInBytes = (eltSize * count);
        Verify(Write(storage, sizeInBytes));
        return sizeInBytes;
    }
    virtual class IBufferedStreamWriter* ToBufferedO() NOEXCEPT override final {
        return nullptr;
    }

private:
    std::streamsize _capacity{ 0 };
    std::streamoff _offset{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
