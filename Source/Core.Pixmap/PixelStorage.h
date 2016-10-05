#pragma once

#include "Core.Pixmap/Pixmap.h"

#include "Core/Container/RawStorage.h"

namespace Core {
namespace Pixmap {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PIXELSTORAGE(_Domain, T) \
    Core::Pixmap::TPixelStorage< T, MEMORY_DOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
class TPixelStorage {
public:
    TPixelStorage() : _size(0) {}
    explicit TPixelStorage(size_t size) : _size(0) { Resize_DiscardData(size); }

    size_t size() const { return _size; }
    size_t capacity() const { return _storage.size(); }

    T* data() { return _storage.data(); }
    const T* data() const { return _storage.data(); }

    T& operator[](size_t index) {
        Assert(index < _size);
        return _storage[index];
    }

    const T& operator[](size_t index) const {
        Assert(index < _size);
        return _storage[index];
    }

    TMemoryView<T> MakeView() { return _storage.MakeView().SubRange(0, _size); }
    TMemoryView<const T> MakeConstView() const { return _storage.MakeConstView().SubRange(0, _size); }

    void Resize_DiscardData(size_t size) {
        // skip allocation if there is enough space :
        if (size > _storage.size())
            _storage.Resize_DiscardData(size);
        _size = size;
        Assert(_storage.size() >= _size);
    }

    void TrimData() {
        if (_size < _storage.size())
            _storage.Resize_KeepData(_size);
        Assert(_storage.size() == _size);
    }

private:
    TRawStorage<T, TAllocator<T, _Domain> > _storage;
    size_t _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace Core
