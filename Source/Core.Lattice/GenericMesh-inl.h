#pragma once

#include "Core.Lattice/GenericMesh.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TGenericVertexSubPart<T>::TGenericVertexSubPart(const FGenericVertexData* data)
:   _data(data) {
    Assert(!_data || Type == _data->Type());
}
//----------------------------------------------------------------------------
template <typename T>
void TGenericVertexSubPart<T>::Write(const T& src) const {
    Assert(_data);
    _data->_vertexCount++;
    _data->_stream.WritePOD(src);
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);
}
//----------------------------------------------------------------------------
template <typename T>
void TGenericVertexSubPart<T>::Write(const TMemoryView<const T>& src) const {
    Assert(_data);
    _data->_vertexCount += src.size();
    _data->_stream.WriteView(src);
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<T> TGenericVertexSubPart<T>::Append(size_t count) const {
    Assert(_data);

    const size_t strideInBytes = Graphics::ValueSizeInBytes(_data->Type());
    const size_t sizeInBytes = strideInBytes * count;

    _data->_vertexCount += count;
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);

    const TMemoryView<u8> reserved = _data->_stream.Append(sizeInBytes);
    const TMemoryView<T> values = reserved.Cast<T>();
    Assert(values.size() == count);
    return values;
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<T> TGenericVertexSubPart<T>::Resize(size_t count, bool keepData/* = false */) const {
    Assert(_data);

    const size_t strideInBytes = Graphics::ValueSizeInBytes(_data->Type());
    const size_t sizeInBytes = strideInBytes * count;

    _data->_vertexCount = count;
    _data->_owner->_vertexCount = Max(_data->_vertexCount, _data->_owner->_vertexCount);
    _data->_stream.resize(sizeInBytes, keepData);

    const TMemoryView<u8> resized(_data->_stream.Pointer(), sizeInBytes);
    const TMemoryView<T> values = resized.Cast<T>();
    Assert(values.size() == count);
    return values;
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<T> TGenericVertexSubPart<T>::MakeView() const {
    if (_data) {
        const TMemoryView<T> view = remove_const(_data)->MakeView().Cast<T>();
        Assert(view.size() == _data->_vertexCount);
        return view;
    }
    else {
        return TMemoryView<T>();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
