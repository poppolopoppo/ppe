#pragma once

#include "Core.Lattice/GenericMesh.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
GenericVertexSubPart<T>::GenericVertexSubPart(const GenericVertexData* data)
:   _data(data) {
    Assert(!_data || Type == _data->Type());
}
//----------------------------------------------------------------------------
template <typename T>
void GenericVertexSubPart<T>::Write(const T& src) const {
    Assert(_data);
    _data->_vertexCount++;
    _data->_stream.WritePOD(src);
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);
}
//----------------------------------------------------------------------------
template <typename T>
void GenericVertexSubPart<T>::Write(const MemoryView<const T>& src) const {
    Assert(_data);
    _data->_vertexCount += src.size();
    _data->_stream.WriteView(src);
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T> GenericVertexSubPart<T>::Append(size_t count) const {
    Assert(_data);

    const size_t strideInBytes = Graphics::ValueSizeInBytes(_data->Type());
    const size_t sizeInBytes = strideInBytes * count;

    _data->_vertexCount += count;
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);

    const MemoryView<u8> reserved = _data->_stream.Append(sizeInBytes);
    const MemoryView<T> values = reserved.Cast<T>();
    Assert(values.size() == count);
    return values;
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T> GenericVertexSubPart<T>::Resize(size_t count, bool keepData/* = false */) const {
    Assert(_data);

    const size_t strideInBytes = Graphics::ValueSizeInBytes(_data->Type());
    const size_t sizeInBytes = strideInBytes * count;

    _data->_vertexCount = count;
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);
    _data->_stream.resize(sizeInBytes, keepData);

    const MemoryView<u8> resized(_data->_stream.Pointer(), sizeInBytes);
    const MemoryView<T> values = resized.Cast<T>();
    Assert(values.size() == count);
    return values;
}
//----------------------------------------------------------------------------
template <typename T>
MemoryView<T> GenericVertexSubPart<T>::MakeView() const {
    if (_data) {
        const MemoryView<T> view = remove_const(_data)->MakeView().Cast<T>();
        Assert(view.size() == _data->_vertexCount);
        return view;
    }
    else {
        return MemoryView<T>();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
