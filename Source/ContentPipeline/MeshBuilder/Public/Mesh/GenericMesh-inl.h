#pragma once

#include "GenericMesh.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
Meta::TOptional<TGenericVertexSubPart<T>> FGenericVertexData::MakeSubPart() const NOEXCEPT {
    if (RHI::VertexAttrib<T>() == _format)
        return TGenericVertexSubPart<T>(*this);
    return Default;
}
//----------------------------------------------------------------------------
template <typename T>
TGenericVertexSubPart<T>::TGenericVertexSubPart(FGenericVertexData* data)
:   _data(data) {
    Assert(!_data || Format == _data->Format());
}
//----------------------------------------------------------------------------
template <typename T>
void TGenericVertexSubPart<T>::Write(const T& src) const {
    Write(TMemoryView<const T>(&src, 1));
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
    const size_t before = _data->_vertexCount;
    const TMemoryView<T> all = Resize(before + count, true);
    return all.CutStartingAt(before);
}
//----------------------------------------------------------------------------
template <typename T>
TMemoryView<T> TGenericVertexSubPart<T>::Resize(size_t count, bool keepData/* = false */) const {
    Assert(_data);

    const size_t strideInBytes = _data->StrideInBytes();
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
TMemoryView<T> TGenericVertexSubPart<T>::MakeView() const NOEXCEPT {
    if (Likely(_data)) {
        const TMemoryView<T> view = _data->MakeView().template Cast<T>();
        Assert(view.size() == _data->_vertexCount);
        return view;
    }
    return TMemoryView<T>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
