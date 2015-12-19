#pragma once

#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix() : ScalarMatrix(0) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(Meta::noinit_tag) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::~ScalarMatrix() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(T broadcast) {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] = broadcast;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(std::initializer_list<T> values) {
    AssertRelease(values.size() == Dim);
    T *data = _data.raw;
    for (const T& it : values)
        *(data++) == it;
    Assert(&_data.raw[Dim] == data);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const column_type& x) {
    STATIC_ASSERT(1 == _Width);
    SetColumn(0, x);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const column_type& x, const column_type& y) {
    STATIC_ASSERT(2 == _Width);
    SetColumn(0, x);
    SetColumn(1, y);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const column_type& x, const column_type& y, const column_type& z) {
    STATIC_ASSERT(3 == _Width);
    SetColumn(0, x);
    SetColumn(1, y);
    SetColumn(2, z);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const column_type& x, const column_type& y, const column_type& z, const column_type& w) {
    STATIC_ASSERT(4 == _Width);
    SetColumn(0, x);
    SetColumn(1, y);
    SetColumn(2, z);
    SetColumn(3, w);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const ScalarMatrix<T, _Width - 1, _Height>& other, const column_type& column) {
    for (size_t i = 0; i < _Width - 1; ++i)
        SetColumn(i, other.Column(i));
    SetColumn(_Width - 1, column);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const ScalarMatrix<T, _Width, _Height - 1>& other, const row_type& row) {
    for (size_t i = 0; i < _Height - 1; ++i)
        SetRow(i, other.Row(i));
    SetRow(Height - 1, row);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const MemoryView<const T>& data) {
    Assert(data.size() == lengthof(_data.raw));
    memcpy(_data.raw, data.Pointer(), lengthof(_data.raw)*sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator =(const ScalarMatrix<T, _Width, _Height>& other) -> ScalarMatrix& {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] = other._data.raw[i];
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <typename U>
auto ScalarMatrix<T, _Width, _Height>::operator =(const ScalarMatrix<U, _Width, _Height>& other) -> ScalarMatrix& {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] = static_cast<T>(other._data.raw[i]);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _Idx>
auto ScalarMatrix<T, _Width, _Height>::Column() const -> column_type {
    STATIC_ASSERT(_Idx < _Width);
    column_type result;
    for (size_t j = 0; j < _Height; ++j)
        result._data[j] = at_(_Idx, j);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::Column(size_t i) const -> column_type {
    Assert(i < _Width);
    column_type result;
    for (size_t j = 0; j < _Height; ++j)
        result._data[j] = at_(i, j);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void ScalarMatrix<T, _Width, _Height>::SetColumn(size_t i, const column_type& v) {
    Assert(i < _Width);
    for (size_t j = 0; j < _Height; ++j)
        at_(i, j) = v._data[j];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _Idx>
auto ScalarMatrix<T, _Width, _Height>::Row() const -> row_type {
    STATIC_ASSERT(_Idx < _Height);
    row_type result;
    for (size_t i = 0; i < _Width; ++i)
        result._data[i] = at_(i, _Idx);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::Row(size_t j) const -> row_type {
    Assert(j < _Height);
    row_type result;
    for (size_t i = 0; i < _Width; ++i)
        result._data[i] = at_(i, j);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void ScalarMatrix<T, _Width, _Height>::SetRow(size_t j, const row_type& v) {
    Assert(j < _Height);
    for (size_t i = 0; i < _Width; ++i)
        at_(i, j) = v._data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarVector<T, _Width> ScalarMatrix<T, _Width, _Height>::Diagonal() const {
    STATIC_ASSERT(_Width == _Height);
    ScalarVector<T, _Width> result;
    for (size_t i = 0; i < _Width; ++i)
        result._data[i] = at_(i, i);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void ScalarMatrix<T, _Width, _Height>::SetDiagonal(const ScalarVector<T, _Width>& v) {
    STATIC_ASSERT(_Width == _Height);
    for (size_t i = 0; i < _Width; ++i)
        at_(i, i) = v._data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void ScalarMatrix<T, _Width, _Height>::SetBroadcast(T broadcast) {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] = broadcast;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _Idx>
ScalarVector<T, 3> ScalarMatrix<T, _Width, _Height>::Axis() const {
    STATIC_ASSERT(_Idx < _Width);
    STATIC_ASSERT(3 <= _Height);
    ScalarVector<T, 3> result;
    for (size_t j = 0; j < 3; ++j)
        result._data[j] = at_(_Idx, j);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _Idx>
void ScalarMatrix<T, _Width, _Height>::SetAxis(const ScalarVector<T, 3>& v) {
    STATIC_ASSERT(_Idx < _Width);
    STATIC_ASSERT(3 <= _Height);
    for (size_t j = 0; j < 3; ++j)
        at_(_Idx, j) = v._data[j];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _Col, size_t _Row>
T& ScalarMatrix<T, _Width, _Height>::at() {
    STATIC_ASSERT(_Col < _Width);
    STATIC_ASSERT(_Row < _Height);
    return _data.m[_Col][_Row];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _Col, size_t _Row>
const T& ScalarMatrix<T, _Width, _Height>::at() const {
    STATIC_ASSERT(_Col < _Width);
    STATIC_ASSERT(_Row < _Height);
    return _data.m[_Col][_Row];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
T& ScalarMatrix<T, _Width, _Height>::at(size_t col, size_t row) {
    Assert(col < _Width);
    Assert(row < _Height);
    return at_(col, row);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
const T& ScalarMatrix<T, _Width, _Height>::at(size_t col, size_t row) const {
    Assert(col < _Width);
    Assert(row < _Height);
    return at_(col, row);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
bool ScalarMatrix<T, _Width, _Height>::operator ==(const ScalarMatrix& other) const {
    for (size_t i = 0; i < Dim; ++i)
        if (_data.raw[i] != other._data.raw[i])
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _NWidth>
ScalarMatrix<T, _NWidth, _Height> ScalarMatrix<T, _Width, _Height>::Multiply(const ScalarMatrix<T, _NWidth, _Width>& other) const {
    ScalarMatrix<T, _NWidth, _Height> result;
    for (size_t i = 0; i < _NWidth; ++i)
        for (size_t j = 0; j < _Height; ++j)
        {
            T val = 0;
            for (size_t k = 0; k < _Width; ++k)
                val += at_(j, k) * other.at_(k, i);
            result.at_(j, i) = val;
        }
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarVector<T, _Height> ScalarMatrix<T, _Width, _Height>::Multiply(const ScalarVector<T, _Width>& v) const {
    ScalarVector<T, _Height> result;
    for (size_t j = 0; j < _Height; ++j) {
        T& x = result._data[j];
        x = 0;
        for (size_t i = 0; i < _Width; ++i)
            x += v._data[i] * at_(j, i);
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
T ScalarMatrix<T, _Width, _Height>::Trace() const {
    STATIC_ASSERT(_Width == _Height);
    T result = 0;
    for (size_t i = 0; i < _Width; ++i)
        result += at_(i, i);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Height, _Width> ScalarMatrix<T, _Width, _Height>::Transpose() const {
    ScalarMatrix<T, _Height, _Width> result;
    for (size_t row = 0; row < _Height; ++row)
        for (size_t col = 0; col < _Width; ++col)
            result.at_(row, col) = at_(col, row);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void ScalarMatrix<T, _Width, _Height>::Swap(ScalarMatrix& other) {
    using std::swap;
    using Core::swap;
    for (size_t i = 0; i < Dim; ++i)
        swap(_data.raw[i], other._data.raw[i]);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <typename U>
ScalarMatrix<U, _Width, _Height> ScalarMatrix<T, _Width, _Height>::Cast() const {
    ScalarMatrix<U, _Width, _Height> result;
    for (size_t i = 0; i < Dim; ++i)
        result._data.raw[i] = static_cast<U>(_data.raw[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
template <size_t _NWidth, size_t _NHeight>
ScalarMatrix<T, _NWidth, _NHeight> ScalarMatrix<T, _Width, _Height>::Crop() const {
    STATIC_ASSERT(_NWidth <= _Width);
    STATIC_ASSERT(_NHeight <= _Height);
    ScalarMatrix<T, _NWidth, _NHeight> result;
    for (size_t col = 0; col < _NWidth; ++col)
        for (size_t row = 0; row < _NHeight; ++row)
            result.at_(col, row) = at_(col, row);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height> ScalarMatrix<T, _Width, _Height>::Identity() {
    ScalarMatrix result;
    for (size_t col = 0; col < _Width; ++col)
        for (size_t row = 0; row < _Height; ++row)
            result.at_(col, row) = (col == row) ? T(1) : T(0);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator -() const -> ScalarMatrix {
    ScalarMatrix result;
    for (size_t i = 0; i < Dim; ++i)
        result._data.raw[i] = -_data.raw[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator +=(const ScalarMatrix& other) -> ScalarMatrix& {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] += other._data.raw[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator -=(const ScalarMatrix& other) -> ScalarMatrix& {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] -= other._data.raw[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator +(const ScalarMatrix& other) const -> ScalarMatrix {
    ScalarMatrix result;
    for (size_t i = 0; i < Dim; ++i)
        result._data.raw[i] = _data.raw[i] + other._data.raw[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator -(const ScalarMatrix& other) const -> ScalarMatrix {
    ScalarMatrix result;
    for (size_t i = 0; i < Dim; ++i)
        result._data.raw[i] = _data.raw[i] - other._data.raw[i];
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SCALARMATRIX_SCALAR_OP_LHS(_Op) \
    template <typename T, size_t _Width, size_t _Height> \
    auto ScalarMatrix<T, _Width, _Height>::operator _Op##=(T scalar) -> ScalarMatrix& { \
        for (size_t i = 0; i < Dim; ++i) \
            _data.raw[i] _Op= scalar; \
        return *this; \
    } \
    template <typename T, size_t _Width, size_t _Height> \
    auto ScalarMatrix<T, _Width, _Height>::operator _Op(T scalar) const -> ScalarMatrix { \
        ScalarMatrix result; \
        for (size_t i = 0; i < Dim; ++i) \
            result._data.raw[i] = _data.raw[i] _Op scalar; \
        return *this; \
    }
//----------------------------------------------------------------------------
DEF_SCALARMATRIX_SCALAR_OP_LHS(+)
DEF_SCALARMATRIX_SCALAR_OP_LHS(-)
DEF_SCALARMATRIX_SCALAR_OP_LHS(*)
DEF_SCALARMATRIX_SCALAR_OP_LHS(/)
DEF_SCALARMATRIX_SCALAR_OP_LHS(^)
DEF_SCALARMATRIX_SCALAR_OP_LHS(%)
//----------------------------------------------------------------------------
#undef DEF_SCALARMATRIX_SCALAR_OP_LHS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SCALARMATRIX_SCALAR_OP_RHS(_Op) \
    template <typename T, size_t _Width, size_t _Height> \
    ScalarMatrix<T, _Width, _Height> operator _Op(T lhs, const ScalarMatrix<T, _Width, _Height>& rhs) { \
        ScalarMatrix<T, _Width, _Height> result; \
        for (size_t i = 0; i < ScalarMatrix<T, _Width, _Height>::Dim; ++i) \
            result._data.raw[i] = lhs _Op rhs._data.raw[i]; \
        return result; \
    } \
    template <typename U, typename T, size_t _Width, size_t _Height> \
    ScalarMatrix<T, _Width, _Height> operator _Op(U lhs, const ScalarMatrix<T, _Width, _Height>& rhs) { \
        ScalarMatrix<T, _Width, _Height> result; \
        for (size_t i = 0; i < ScalarMatrix<T, _Width, _Height>::Dim; ++i) \
            result._data.raw[i] = static_cast<T>(lhs _Op rhs._data.raw[i]); \
        return result; \
    }
//----------------------------------------------------------------------------
DEF_SCALARMATRIX_SCALAR_OP_RHS(+)
DEF_SCALARMATRIX_SCALAR_OP_RHS(-)
DEF_SCALARMATRIX_SCALAR_OP_RHS(*)
DEF_SCALARMATRIX_SCALAR_OP_RHS(/)
DEF_SCALARMATRIX_SCALAR_OP_RHS(^)
DEF_SCALARMATRIX_SCALAR_OP_RHS(%)
//----------------------------------------------------------------------------
#undef DEF_SCALARMATRIX_SCALAR_OP_RHS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
