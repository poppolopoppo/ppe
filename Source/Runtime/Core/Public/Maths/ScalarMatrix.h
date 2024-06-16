#pragma once

#include "Core.h"

#include "Maths/ScalarMatrix_fwd.h"
#include "Maths/ScalarVector.h"

#include "HAL/PlatformMemory.h"
#include "IO/TextWriter_fwd.h"

#include <initializer_list>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
union ScalarMatrixData {
    T m[_Width][_Height];
    T raw[_Width * _Height];
};
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
class TScalarMatrix {
public:
    template <typename U, u32 _Width2, u32 _Height2>
    friend class TScalarMatrix;

    STATIC_CONST_INTEGRAL(u32, Width, _Width);
    STATIC_CONST_INTEGRAL(u32, Height, _Height);

    typedef TScalarVector<T, _Width> row_type;
    typedef TScalarVector<T, _Height> column_type;

    explicit TScalarMatrix(Meta::FForceInit);
    explicit TScalarMatrix(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR TScalarMatrix() NOEXCEPT
        : TScalarMatrix(Meta::ForceInit)
    {}

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

#if 0
    template <u32 _Idx>
    FORCE_INLINE column_type Column() const;
    column_type Column(u32 i) const;
    void SetColumn(u32 i, const column_type& v);
#else
    template <u32 _Idx>
    FORCE_INLINE column_type& Column();
    template <u32 _Idx>
    FORCE_INLINE const column_type& Column() const;
    column_type& Column(u32 i);
    const column_type& Column(u32 i) const;
    void SetColumn(u32 i, const column_type& v);
#endif

    FORCE_INLINE column_type Column_x() const { return Column<0>(); }
    FORCE_INLINE column_type Column_y() const { return Column<1>(); }
    FORCE_INLINE column_type Column_z() const { return Column<2>(); }
    FORCE_INLINE column_type Column_w() const { return Column<3>(); }

    FORCE_INLINE void SetColumn_x(const column_type& value) { return SetColumn(0, value); }
    FORCE_INLINE void SetColumn_y(const column_type& value) { return SetColumn(1, value); }
    FORCE_INLINE void SetColumn_z(const column_type& value) { return SetColumn(2, value); }
    FORCE_INLINE void SetColumn_w(const column_type& value) { return SetColumn(3, value); }

    template <u32 _Idx>
    FORCE_INLINE row_type Row() const;
    row_type Row(u32 i) const;
    void SetRow(u32 i, const row_type& v);

    FORCE_INLINE row_type Row_x() const { return Row<0>(); }
    FORCE_INLINE row_type Row_y() const { return Row<1>(); }
    FORCE_INLINE row_type Row_z() const { return Row<2>(); }
    FORCE_INLINE row_type Row_w() const { return Row<3>(); }

    FORCE_INLINE void SetRow_x(const row_type& value) { return SetRow(0, value); }
    FORCE_INLINE void SetRow_y(const row_type& value) { return SetRow(1, value); }
    FORCE_INLINE void SetRow_z(const row_type& value) { return SetRow(2, value); }
    FORCE_INLINE void SetRow_w(const row_type& value) { return SetRow(3, value); }

    template <u32 _Idx>
    TScalarVector<T, 3> Axis() const;
    template <u32 _Idx>
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

    template <u32 _NWidth>
    TScalarMatrix<T, _NWidth, _Height> Multiply(const TScalarMatrix<T, _NWidth, _Width>& other) const;

    TScalarVector<T, _Height> Multiply(const TScalarVector<T, _Width>& v) const;
    TScalarVector<T, _Height> Multiply_OneExtend(const TScalarVector<T, _Width - 1>& v) const;
    TScalarVector<T, _Height> Multiply_ZeroExtend(const TScalarVector<T, _Width - 1>& v) const;

    T Trace() const;

    TScalarMatrix<T, _Height, _Width> Transpose() const;

    void Swap(TScalarMatrix& other);

    TMemoryView<T> MakeView() { return PPE::MakeView(_data.raw); }
    TMemoryView<const T> MakeView() const { return PPE::MakeView(_data.raw); }

    friend hash_t hash_value(const TScalarMatrix& m) NOEXCEPT { return hash_as_pod_array(m._data.raw); }

    template <typename U>
    TScalarMatrix<U, _Width, _Height> Cast() const;

    template <u32 _NWidth, u32 _NHeight>
    TScalarMatrix<T, _NWidth, _NHeight> Crop() const;

    TScalarMatrix<T, _Width + 1, _Height + 1> OneExtend() const;
    TScalarMatrix<T, _Width + 1, _Height + 1> ZeroExtend() const;

    static TScalarMatrix One() { return TScalarMatrix(T(1)); }
    static TScalarMatrix Zero() { return TScalarMatrix(T(0)); }
    static TScalarMatrix Identity();

    STATIC_CONST_INTEGRAL(u32, Dim, _Width * _Height);

    template <u32 _Col, u32 _Row>
    FORCE_INLINE T& at();
    template <u32 _Col, u32 _Row>
    FORCE_INLINE const T& at() const;

    FORCE_INLINE T& at(u32 col, u32 row);
    FORCE_INLINE const T& at(u32 col, u32 row) const;

    FORCE_INLINE T& operator ()(u32 col, u32 row) { return at(col, row); }
    FORCE_INLINE const T& operator ()(u32 col, u32 row) const { return at(col, row); }

    FORCE_INLINE column_type& operator [](u32 col) { return Column(col); }
    FORCE_INLINE const column_type& operator [](u32 col) const { return Column(col); }

    FORCE_INLINE ScalarMatrixData<T, _Width, _Height>& data() { return _data; }
    FORCE_INLINE const ScalarMatrixData<T, _Width, _Height>& data() const { return _data; }

    FORCE_INLINE T& at_(u32 col, u32 row) { return _data.m[col][row]; }
    FORCE_INLINE const T& at_(u32 col, u32 row) const { return _data.m[col][row]; }

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
    template <typename U, u32 _W, u32 _H> \
    friend TScalarMatrix<U, _W, _H> operator _Op(U lhs, const TScalarMatrix<U, _W, _H>& rhs); \
    template <typename U, typename V, u32 _W, u32 _H> \
    friend TScalarMatrix<U, _W, _H> operator _Op(U lhs, const TScalarMatrix<V, _W, _H>& rhs);

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

    template <u32 _NWidth>
    TScalarMatrix<T, _NWidth, _Height> operator *(const TScalarMatrix<T, _NWidth, _Width>& other) const {
        return Multiply(other);
    }

    friend void swap(TScalarMatrix& lhs, TScalarMatrix& rhs) NOEXCEPT {
        lhs.Swap(rhs);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename T, u32 _Width, u32 _Height>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TScalarMatrix<T, _Width, _Height>& v) {
    for (u32 i = 0; i < _Height; ++i)
        oss << v.Row(i) << Eol;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// All scalar matrices are considered as pods
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TScalarMatrix<T COMMA _Width COMMA _Height>, typename T, u32 _Width, u32 _Height)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarMatrix-inl.h"
