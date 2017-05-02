#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarMatrix_fwd.h"

#include "Core/Maths/ScalarVector.h"

#include <iosfwd>
#include <initializer_list>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
union ScalarMatrixData {
    T m[_Width][_Height];
    T raw[_Width * _Height];
};
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
class TScalarMatrix {
public:
    template <typename U, size_t _Width2, size_t _Height2>
    friend class TScalarMatrix;

    STATIC_CONST_INTEGRAL(size_t, Width, _Width);
    STATIC_CONST_INTEGRAL(size_t, Height, _Height);

    typedef TScalarVector<T, _Width> row_type;
    typedef TScalarVector<T, _Height> column_type;

    FORCE_INLINE TScalarMatrix() {}
    explicit TScalarMatrix(Meta::FForceInit);

    TScalarMatrix(T broadcast);
    TScalarMatrix(std::initializer_list<T> values);

    TScalarMatrix(const column_type& x);
    TScalarMatrix(const column_type& x, const column_type& y);
    TScalarMatrix(const column_type& x, const column_type& y, const column_type& z);
    TScalarMatrix(const column_type& x, const column_type& y, const column_type& z, const column_type& w);

    TScalarMatrix(const TScalarMatrix<T, _Width - 1, _Height>& other, const column_type& column);
    TScalarMatrix(const TScalarMatrix<T, _Width, _Height - 1>& other, const row_type& row);

    TScalarMatrix(const TMemoryView<const T>& data);

    FORCE_INLINE TScalarMatrix(const TScalarMatrix& other) { operator =(other); }
    TScalarMatrix& operator =(const TScalarMatrix& other);

    template <typename U>
    FORCE_INLINE TScalarMatrix(const TScalarMatrix<U, _Width, _Height>& other) { operator =(other); }
    template <typename U>
    TScalarMatrix& operator =(const TScalarMatrix<U, _Width, _Height>& other);

    template <size_t _Idx>
    FORCE_INLINE column_type Column() const;
    column_type Column(size_t i) const;
    void SetColumn(size_t i, const column_type& v);

    FORCE_INLINE column_type Column_x() const { return Column<0>(); }
    FORCE_INLINE column_type Column_y() const { return Column<1>(); }
    FORCE_INLINE column_type Column_z() const { return Column<2>(); }
    FORCE_INLINE column_type Column_w() const { return Column<3>(); }

    FORCE_INLINE void SetColumn_x(const column_type& value) { return SetColumn(0, value); }
    FORCE_INLINE void SetColumn_y(const column_type& value) { return SetColumn(1, value); }
    FORCE_INLINE void SetColumn_z(const column_type& value) { return SetColumn(2, value); }
    FORCE_INLINE void SetColumn_w(const column_type& value) { return SetColumn(3, value); }

    template <size_t _Idx>
    FORCE_INLINE row_type Row() const;
    row_type Row(size_t i) const;
    void SetRow(size_t i, const row_type& v);

    FORCE_INLINE row_type Row_x() const { return Row<0>(); }
    FORCE_INLINE row_type Row_y() const { return Row<1>(); }
    FORCE_INLINE row_type Row_z() const { return Row<2>(); }
    FORCE_INLINE row_type Row_w() const { return Row<3>(); }

    FORCE_INLINE void SetRow_x(const row_type& value) { return SetRow(0, value); }
    FORCE_INLINE void SetRow_y(const row_type& value) { return SetRow(1, value); }
    FORCE_INLINE void SetRow_z(const row_type& value) { return SetRow(2, value); }
    FORCE_INLINE void SetRow_w(const row_type& value) { return SetRow(3, value); }

    template <size_t _Idx>
    TScalarVector<T, 3> Axis() const;
    template <size_t _Idx>
    void SetAxis(const TScalarVector<T, 3>& v);

    FORCE_INLINE TScalarVector<T, 3> AxisX() const { return Axis<0>(); }
    FORCE_INLINE TScalarVector<T, 3> AxisY() const { return Axis<1>(); }
    FORCE_INLINE TScalarVector<T, 3> AxisZ() const { return Axis<2>(); }
    FORCE_INLINE TScalarVector<T, 3> AxisT() const { return Axis<3>(); }

    FORCE_INLINE void SetAxisX(const TScalarVector<T, 3>& v) { SetAxis<0>(v); }
    FORCE_INLINE void SetAxisY(const TScalarVector<T, 3>& v) { SetAxis<1>(v); }
    FORCE_INLINE void SetAxisZ(const TScalarVector<T, 3>& v) { SetAxis<2>(v); }
    FORCE_INLINE void SetAxisT(const TScalarVector<T, 3>& v) { SetAxis<3>(v); }

    TScalarVector<T, _Width> Diagonal() const;
    void SetDiagonal(const TScalarVector<T, _Width>& v);

    void SetBroadcast(T broadcast);

    bool operator ==(const TScalarMatrix& other) const;
    bool operator !=(const TScalarMatrix& other) const { return !operator ==(other); }

    template <size_t _NWidth>
    TScalarMatrix<T, _NWidth, _Height> Multiply(const TScalarMatrix<T, _NWidth, _Width>& other) const;

    TScalarVector<T, _Height> Multiply(const TScalarVector<T, _Width>& v) const;
    TScalarVector<T, _Height> Multiply_OneExtend(const TScalarVector<T, _Width - 1>& v) const;
    TScalarVector<T, _Height> Multiply_ZeroExtend(const TScalarVector<T, _Width - 1>& v) const;

    T Trace() const;

    TScalarMatrix<T, _Height, _Width> Transpose() const;

    void Swap(TScalarMatrix& other);

    TMemoryView<T> MakeView() { return Core::MakeView(_data.raw); }
    TMemoryView<const T> MakeView() const { return Core::MakeView(_data.raw); }

    friend hash_t hash_value(const TScalarMatrix& m) { return hash_as_pod_array(m._data.raw); }

    template <typename U>
    TScalarMatrix<U, _Width, _Height> Cast() const;

    template <size_t _NWidth, size_t _NHeight>
    TScalarMatrix<T, _NWidth, _NHeight> Crop() const;

    static TScalarMatrix One() { return TScalarMatrix(T(1)); }
    static TScalarMatrix Zero() { return TScalarMatrix(T(0)); }
    static TScalarMatrix Identity();

    STATIC_CONST_INTEGRAL(size_t, Dim, _Width * _Height);

    template <size_t _Col, size_t _Row>
    FORCE_INLINE T& at();
    template <size_t _Col, size_t _Row>
    FORCE_INLINE const T& at() const;

    FORCE_INLINE T& at(size_t col, size_t row);
    FORCE_INLINE const T& at(size_t col, size_t row) const;

    FORCE_INLINE T& operator ()(size_t col, size_t row) { return at(col, row); }
    FORCE_INLINE const T& operator ()(size_t col, size_t row) const { return at(col, row); }

    FORCE_INLINE ScalarMatrixData<T, _Width, _Height>& data() { return _data; }
    FORCE_INLINE const ScalarMatrixData<T, _Width, _Height>& data() const { return _data; }

    FORCE_INLINE T& at_(size_t col, size_t row) { return _data.m[col][row]; }
    FORCE_INLINE const T& at_(size_t col, size_t row) const { return _data.m[col][row]; }

public:
    STATIC_ASSERT(0 < _Width);
    STATIC_ASSERT(0 < _Height);

    ScalarMatrixData<T, _Width, _Height> _data;

public:
    // HLSL-like scalar accessors :
#define DECL_SCALARMATRIX_SCALAR_ACCESSOR(_X, _Y, _Col, _Row) \
    FORCE_INLINE T& m ## _X ## _Y() { return _data.m[_X][_Y]; } \
    FORCE_INLINE const T& m ## _X ## _Y() const { return _data.m[_X][_Y]; } \
    \
    FORCE_INLINE T& _ ## _Col ## _Row() { return _data.m[_X][_Y]; } \
    FORCE_INLINE const T& _ ## _Col ## _Row() const { return _data.m[_X][_Y]; }

    DECL_SCALARMATRIX_SCALAR_ACCESSOR(0, 0, 1, 1)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(0, 1, 1, 2)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(0, 2, 1, 3)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(0, 3, 1, 4)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(1, 0, 2, 1)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(1, 1, 2, 2)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(1, 2, 2, 3)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(1, 3, 2, 4)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(2, 0, 3, 1)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(2, 1, 3, 2)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(2, 2, 3, 3)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(2, 3, 3, 4)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(3, 0, 4, 1)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(3, 1, 4, 2)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(3, 2, 4, 3)
    DECL_SCALARMATRIX_SCALAR_ACCESSOR(3, 3, 4, 4)

#undef DECL_SCALARMATRIX_SCALAR_ACCESSOR

#define DECL_SCALARMATRIX_SCALAR_OP_LHS(_Op) \
    TScalarMatrix&   operator _Op##=(T scalar); \
    TScalarMatrix    operator _Op(T scalar) const;

    DECL_SCALARMATRIX_SCALAR_OP_LHS(+)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(-)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(*)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(/)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(^)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(%)

#undef DECL_SCALARMATRIX_SCALAR_OP_LHS

#define DECL_SCALARMATRIX_SCALAR_OP_RHS(_Op) \
    template <typename T, size_t _Width, size_t _Height> \
    friend TScalarMatrix<T, _Width, _Height> operator _Op(T lhs, const TScalarMatrix<T, _Width, _Height>& rhs); \
    template <typename U, typename T, size_t _Width, size_t _Height> \
    friend TScalarMatrix<T, _Width, _Height> operator _Op(U lhs, const TScalarMatrix<T, _Width, _Height>& rhs);

    DECL_SCALARMATRIX_SCALAR_OP_RHS(+)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(-)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(*)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(/)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(^)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(%)

#undef DECL_SCALARMATRIX_SCALAR_OP_RHS

    TScalarMatrix    operator -() const;

    TScalarMatrix&   operator +=(const TScalarMatrix& other);
    TScalarMatrix&   operator -=(const TScalarMatrix& other);

    TScalarMatrix    operator +(const TScalarMatrix& other) const;
    TScalarMatrix    operator -(const TScalarMatrix& other) const;

    template <size_t _NWidth>
    TScalarMatrix<T, _NWidth, _Height> operator *(const TScalarMatrix<T, _NWidth, _Width>& other) const {
        return Multiply(other);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T, size_t _Width, size_t _Height>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const TScalarMatrix<T, _Width, _Height>& v) {
    for (size_t i = 0; i < _Height; ++i)
        oss << v.Row(i) << eol;
    return oss;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void swap(TScalarMatrix<T, _Width, _Height>& lhs, TScalarMatrix<T, _Width, _Height>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarMatrix-inl.h"
