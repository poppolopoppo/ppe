#pragma once

#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T ScalarMatrixSquare<T, _Dim, _Dim>::Trace() const {
    T result = 0;
    const auto* const pself = static_cast<const ScalarMatrix<T, _Dim, _Dim>*>(this);
    for (size_t i = 0; i < _Dim; ++i)
        result += pself->at_(i, i);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> ScalarMatrixSquare<T, _Dim, _Dim>::Diagonal() const {
    ScalarVector<T, _Dim> result(Meta::noinit_tag());
    const auto* const pself = static_cast<const ScalarMatrix<T, _Dim, _Dim>*>(this);
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = pself->at_(i, i);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarMatrixSquare<T, _Dim, _Dim>::SetDiagonal(const ScalarVector<T, _Dim>& v) {
    const auto* const pself = static_cast<const ScalarMatrix<T, _Dim, _Dim>*>(this);
    for (size_t i = 0; i < _Dim; ++i)
        pself->at_(i, i) = v._data[i];
}
//----------------------------------------------------------------------------
#define DEF_SCALARMATRIX_SCALAR_COLUMN(_Width, _Alias) \
    template <typename T, size_t _Height> \
    FORCE_INLINE auto ScalarMatrixColumn<T, _Width, _Height>::Column_ ## _Alias() const -> column_type { \
        return static_cast<const ScalarMatrix<T, _Width, _Height>*>(this)->Column<_Width-1>(); \
    } \
    template <typename T, size_t _Height> \
    FORCE_INLINE void ScalarMatrixColumn<T, _Width, _Height>::SetColumn_ ## _Alias(const column_type& value) { \
        static_cast<ScalarMatrix<T, _Width, _Height>*>(this)->SetColumn(_Width-1, value); \
    }
#define DEF_SCALARMATRIX_SCALAR_ROW(_Height, _Alias) \
    template <typename T, size_t _Width> \
    FORCE_INLINE auto ScalarMatrixRow<T, _Width, _Height>::Row_ ## _Alias() const -> row_type { \
        return static_cast<const ScalarMatrix<T, _Width, _Height>*>(this)->Column<_Height-1>(); \
    } \
    template <typename T, size_t _Width> \
    FORCE_INLINE void ScalarMatrixRow<T, _Width, _Height>::SetRow_ ## _Alias(const row_type& value) { \
        static_cast<ScalarMatrix<T, _Width, _Height>*>(this)->SetRow(_Height-1, value); \
    }
#define DEF_SCALARMATRIX_SCALAR_COLUMN_ROW(_Dim, _Alias) \
    DEF_SCALARMATRIX_SCALAR_COLUMN(_Dim, _Alias) \
    DEF_SCALARMATRIX_SCALAR_ROW(_Dim, _Alias)
DEF_SCALARMATRIX_SCALAR_COLUMN_ROW(1, x)
DEF_SCALARMATRIX_SCALAR_COLUMN_ROW(2, y)
DEF_SCALARMATRIX_SCALAR_COLUMN_ROW(3, z)
DEF_SCALARMATRIX_SCALAR_COLUMN_ROW(4, w)
#undef DEF_SCALARMATRIX_SCALAR_COLUMN_ROW
#undef DEF_SCALARMATRIX_SCALAR_COLUMN
#undef DEF_SCALARMATRIX_SCALAR_ROW
//----------------------------------------------------------------------------
#define DEF_SCALARMATRIX_SCALAR_ACCESSOR(_X, _Y, _Col, _Row) \
    template <typename T> \
    FORCE_INLINE T& ScalarMatrixAccessorImpl<T, _Col, _Row>::m ## _X ## _Y() { \
        return static_cast<ScalarMatrix<T, _Col, _Row>*>(this)->_data.m[_X][_Y]; \
    } \
    template <typename T> \
    FORCE_INLINE const T& ScalarMatrixAccessorImpl<T, _Col, _Row>::m ## _X ## _Y() const { \
        return static_cast<const ScalarMatrix<T, _Col, _Row>*>(this)->_data.m[_X][_Y]; \
    } \
    template <typename T> \
    FORCE_INLINE T& ScalarMatrixAccessorImpl<T, _Col, _Row>::_ ## _Col ## _Row() { \
        return static_cast<ScalarMatrix<T, _Col, _Row>*>(this)->_data.m[_X][_Y]; \
    } \
    template <typename T> \
    FORCE_INLINE const T& ScalarMatrixAccessorImpl<T, _Col, _Row>::_ ## _Col ## _Row() const { \
        return static_cast<const ScalarMatrix<T, _Col, _Row>*>(this)->_data.m[_X][_Y]; \
    }
DEF_SCALARMATRIX_SCALAR_ACCESSOR(0, 0, 1, 1)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(0, 1, 1, 2)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(0, 2, 1, 3)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(0, 3, 1, 4)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(1, 0, 2, 1)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(1, 1, 2, 2)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(1, 2, 2, 3)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(1, 3, 2, 4)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(2, 0, 3, 1)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(2, 1, 3, 2)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(2, 2, 3, 3)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(2, 3, 3, 4)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(3, 0, 4, 1)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(3, 1, 4, 2)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(3, 2, 4, 3)
DEF_SCALARMATRIX_SCALAR_ACCESSOR(3, 3, 4, 4)
#undef DEF_SCALARMATRIX_SCALAR_ACCESSOR
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix() : ScalarMatrix(0) {
    STATIC_ASSERT(sizeof(*this) == _Width*_Height*sizeof(T));
}
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
        *(data++) = it;
    Assert(&_data.raw[Dim] == data);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height>::ScalarMatrix(const column_type (&columns)[_Width]) {
    for (size_t i = 0; i < _Width; ++i)
        SetColumn(i, columns[i]);
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
    column_type result(Meta::noinit_tag{});
    for (size_t j = 0; j < _Height; ++j)
        result._data[j] = at_(_Idx, j);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::Column(size_t i) const -> column_type {
    Assert(i < _Width);
    column_type result(Meta::noinit_tag{});
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
    row_type result(Meta::noinit_tag{});
    for (size_t i = 0; i < _Width; ++i)
        result._data[i] = at_(i, _Idx);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::Row(size_t j) const -> row_type {
    Assert(j < _Height);
    row_type result(Meta::noinit_tag{});
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
    ScalarVector<T, 3> result(Meta::noinit_tag{});
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
    ScalarMatrix<T, _NWidth, _Height> result(Meta::noinit_tag{});
    for (size_t i = 0; i < _NWidth; ++i)
        for (size_t j = 0; j < _Height; ++j) {
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
    ScalarVector<T, _Height> result(Meta::noinit_tag{});
    for (size_t j = 0; j < _Height; ++j) {
        T& x = result._data[j] = 0;
        for (size_t i = 0; i < _Width; ++i)
            x += v._data[i] * at_(j, i);
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarVector<T, _Height> ScalarMatrix<T, _Width, _Height>::Multiply_OneExtend(const ScalarVector<T, _Width - 1>& v) const {
    STATIC_ASSERT(_Width > 1);
    ScalarVector<T, _Height> result(Meta::noinit_tag{});
    for (size_t j = 0; j < _Height; ++j) {
        T& x = result._data[j] = 0;
        for (size_t i = 0; i < _Width-1; ++i)
            x += v._data[i] * at_(j, i);
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarVector<T, _Height-1> ScalarMatrix<T, _Width, _Height>::Multiply_ZeroExtend(const ScalarVector<T, _Width - 1>& v) const {
    STATIC_ASSERT(_Height-1 <= _Width);
    //STATIC_ASSERT(_Width == _Height);
    ScalarVector<T, _Height-1> result(Meta::noinit_tag{});
    for (size_t j = 0; j < _Height-1; ++j) {
        T& x = result._data[j] = 0;
        for (size_t i = 0; i < _Width-1; ++i)
            x += v._data[i] * at_(j, i);
    }
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Height, _Width> ScalarMatrix<T, _Width, _Height>::Transpose() const {
    ScalarMatrix<T, _Height, _Width> result(Meta::noinit_tag{});
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
    ScalarMatrix<U, _Width, _Height> result(Meta::noinit_tag{});
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
    ScalarMatrix<T, _NWidth, _NHeight> result(Meta::noinit_tag{});
    for (size_t col = 0; col < _NWidth; ++col)
        for (size_t row = 0; row < _NHeight; ++row)
            result.at_(col, row) = at_(col, row);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
ScalarMatrix<T, _Width, _Height> ScalarMatrix<T, _Width, _Height>::Identity() {
    ScalarMatrix result(Meta::noinit_tag{});
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
    ScalarMatrix result(Meta::noinit_tag{});
    for (size_t i = 0; i < Dim; ++i)
        result._data.raw[i] = -_data.raw[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator +=(const ScalarMatrix& other) -> ScalarMatrix& {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] += other._data.raw[i];
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator -=(const ScalarMatrix& other) -> ScalarMatrix& {
    for (size_t i = 0; i < Dim; ++i)
        _data.raw[i] -= other._data.raw[i];
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator +(const ScalarMatrix& other) const -> ScalarMatrix {
    ScalarMatrix result(Meta::noinit_tag{});
    for (size_t i = 0; i < Dim; ++i)
        result._data.raw[i] = _data.raw[i] + other._data.raw[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
auto ScalarMatrix<T, _Width, _Height>::operator -(const ScalarMatrix& other) const -> ScalarMatrix {
    ScalarMatrix result(Meta::noinit_tag{});
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
        ScalarMatrix result(Meta::noinit_tag{}); \
        for (size_t i = 0; i < Dim; ++i) \
            result._data.raw[i] = _data.raw[i] _Op scalar; \
        return *this; \
    }
//----------------------------------------------------------------------------
DEF_SCALARMATRIX_SCALAR_OP_LHS(+)
DEF_SCALARMATRIX_SCALAR_OP_LHS(-)
DEF_SCALARMATRIX_SCALAR_OP_LHS(*)
DEF_SCALARMATRIX_SCALAR_OP_LHS(/)
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
