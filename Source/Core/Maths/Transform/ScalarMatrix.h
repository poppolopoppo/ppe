#pragma once

#include "Core/Core.h"

#include "Core/Maths/Transform/ScalarMatrix_fwd.h"
#include "Core/Maths/Geometry/ScalarVector_extern.h"

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
namespace details {
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrixSquare {};
template <typename T, size_t _Dim>
class ScalarMatrixSquare<T, _Dim, _Dim> {
public:
    T Trace() const;
    ScalarVector<T, _Dim> Diagonal() const;
    void SetDiagonal(const ScalarVector<T, _Dim>& v);
};
} //!details
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrixColumn {
public:
    typedef ScalarVector<T, _Height> column_type;
};
#define DECL_SCALARMATRIX_SCALAR_COLUMN(_Width, _Alias) \
    template <typename T, size_t _Height> \
    class ScalarMatrixColumn<T, _Width, _Height> : public ScalarMatrixColumn<T, _Width-1, _Height> { \
    public: \
        column_type Column_ ## _Alias() const; \
        void SetColumn_ ## _Alias(const column_type& value); \
    };
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrixRow {
public:
    typedef ScalarVector<T, _Width> row_type;
};
#define DECL_SCALARMATRIX_SCALAR_ROW(_Height, _Alias) \
    template <typename T, size_t _Width> \
    class ScalarMatrixRow<T, _Width, _Height> : public ScalarMatrixRow<T, _Width, _Height-1> { \
    public: \
        row_type Row_ ## _Alias() const; \
        void SetRow_ ## _Alias(const row_type& value); \
    };
#define DECL_SCALARMATRIX_SCALAR_COLUMN_ROW(_Dim, _Alias) \
    DECL_SCALARMATRIX_SCALAR_COLUMN(_Dim, _Alias) \
    DECL_SCALARMATRIX_SCALAR_ROW(_Dim, _Alias)
DECL_SCALARMATRIX_SCALAR_COLUMN_ROW(1, x)
DECL_SCALARMATRIX_SCALAR_COLUMN_ROW(2, y)
DECL_SCALARMATRIX_SCALAR_COLUMN_ROW(3, z)
DECL_SCALARMATRIX_SCALAR_COLUMN_ROW(4, w)
#undef DECL_SCALARMATRIX_SCALAR_COLUMN_ROW
#undef DECL_SCALARMATRIX_SCALAR_COLUMN
#undef DECL_SCALARMATRIX_SCALAR_ROW
} //!details
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrixAccessorImpl {}; // HLSL-like scalar accessors
#define DECL_SCALARMATRIX_SCALAR_ACCESSOR(_X, _Y, _Col, _Row) \
    template <typename T> \
    class ScalarMatrixAccessorImpl<T, _Col, _Row> { \
    public: \
        T& m ## _X ## _Y(); \
        const T& m ## _X ## _Y() const; \
    \
        T& _ ## _Col ## _Row(); \
        const T& _ ## _Col ## _Row() const; \
    };
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
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrixAccessorEachRow
:   public ScalarMatrixAccessorEachRow<T, _Width, _Height-1>
,   public ScalarMatrixAccessorImpl<T, _Width, _Height> {};
template <typename T, size_t _Width>
class ScalarMatrixAccessorEachRow<T, _Width, 0> {};
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrixAccessorEachColumn
:   public ScalarMatrixAccessorEachColumn<T, _Width-1, _Height>
,   public ScalarMatrixAccessorEachRow<T, _Width, _Height> {};
template <typename T, size_t _Height>
class ScalarMatrixAccessorEachColumn<T, 0, _Height> {};
template <typename T, size_t _Width, size_t _Height>
using ScalarMatrixAccessor = ScalarMatrixAccessorEachColumn<T, _Width, _Height>;
} //!details
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
class ScalarMatrix
:   public details::ScalarMatrixSquare<T, _Width, _Height>
,   public details::ScalarMatrixColumn<T, _Width, _Height>
,   public details::ScalarMatrixRow<T, _Width, _Height>
,   public details::ScalarMatrixAccessor<T, _Width, _Height> {
public:
    template <typename U, size_t _Width2, size_t _Height2>
    friend class ScalarMatrix;
    template <typename U, size_t _Width2, size_t _Height2>
    friend class details::ScalarMatrixAccessorImpl;

    STATIC_CONST_INTEGRAL(size_t, Width, _Width);
    STATIC_CONST_INTEGRAL(size_t, Height, _Height);

    STATIC_ASSERT(_Width >= 1 && _Width <= 4);
    STATIC_ASSERT(_Height >= 1 && _Height <= 4);

    typedef ScalarVector<T, _Width> row_type;
    typedef ScalarVector<T, _Height> column_type;

    ScalarMatrix();
    explicit ScalarMatrix(Meta::noinit_tag);
    ~ScalarMatrix();

    ScalarMatrix(T broadcast);
    ScalarMatrix(std::initializer_list<T> values);

    ScalarMatrix(const column_type (&columns)[_Width]);

    ScalarMatrix(const ScalarMatrix<T, _Width - 1, _Height>& other, const column_type& column);
    ScalarMatrix(const ScalarMatrix<T, _Width, _Height - 1>& other, const row_type& row);

    ScalarMatrix(const MemoryView<const T>& data);

    FORCE_INLINE ScalarMatrix(const ScalarMatrix& other) { operator =(other); }
    ScalarMatrix& operator =(const ScalarMatrix& other);

    template <typename U>
    FORCE_INLINE ScalarMatrix(const ScalarMatrix<U, _Width, _Height>& other) { operator =(other); }
    template <typename U>
    ScalarMatrix& operator =(const ScalarMatrix<U, _Width, _Height>& other);

    template <size_t _Idx>
    FORCE_INLINE column_type Column() const;
    column_type Column(size_t i) const;
    void SetColumn(size_t i, const column_type& v);

    template <size_t _Idx>
    FORCE_INLINE row_type Row() const;
    row_type Row(size_t i) const;
    void SetRow(size_t i, const row_type& v);

    template <size_t _Idx>
    ScalarVector<T, 3> Axis() const;
    template <size_t _Idx>
    void SetAxis(const ScalarVector<T, 3>& v);

    void SetBroadcast(T broadcast);

    bool operator ==(const ScalarMatrix& other) const;
    bool operator !=(const ScalarMatrix& other) const { return !operator ==(other); }

    template <size_t _NWidth>
    ScalarMatrix<T, _NWidth, _Height> Multiply(const ScalarMatrix<T, _NWidth, _Width>& other) const;

    ScalarVector<T, _Height> Multiply(const ScalarVector<T, _Width>& v) const;
    ScalarVector<T, _Height> Multiply_OneExtend(const ScalarVector<T, _Width - 1>& v) const;
    ScalarVector<T, _Height-1> Multiply_ZeroExtend(const ScalarVector<T, _Width - 1>& v) const;

    ScalarMatrix<T, _Height, _Width> Transpose() const;

    void Swap(ScalarMatrix& other);

    MemoryView<const T> MakeView() const { return Core::MakeView(_data.raw); }

    friend hash_t hash_value(const ScalarMatrix& m) { return hash_as_pod_array(m._data.raw); }

    template <typename U>
    ScalarMatrix<U, _Width, _Height> Cast() const;

    template <size_t _NWidth, size_t _NHeight>
    ScalarMatrix<T, _NWidth, _NHeight> Crop() const;

    static ScalarMatrix One() { return ScalarMatrix(T(1)); }
    static ScalarMatrix Zero() { return ScalarMatrix(T(0)); }
    static ScalarMatrix Identity();

    STATIC_CONST_INTEGRAL(size_t, Dim, _Width * _Height);

    template <size_t _Col, size_t _Row>
    FORCE_INLINE T& at();
    template <size_t _Col, size_t _Row>
    FORCE_INLINE const T& at() const;

    FORCE_INLINE T& at(size_t col, size_t row);
    FORCE_INLINE const T& at(size_t col, size_t row) const;

    FORCE_INLINE T& operator ()(size_t col, size_t row) { return at(col, row); }
    FORCE_INLINE const T& operator ()(size_t col, size_t row) const { return at(col, row); }

    FORCE_INLINE ScalarMatrixData<T, _Width, _Height>& data_() { return _data; }
    FORCE_INLINE const ScalarMatrixData<T, _Width, _Height>& data_() const { return _data; }

    FORCE_INLINE T& at_(size_t col, size_t row) { return _data.m[col][row]; }
    FORCE_INLINE const T& at_(size_t col, size_t row) const { return _data.m[col][row]; }

private:
    STATIC_ASSERT(0 < _Width);
    STATIC_ASSERT(0 < _Height);

    ScalarMatrixData<T, _Width, _Height> _data;

public:
#define DECL_SCALARMATRIX_SCALAR_OP_LHS(_Op) \
    ScalarMatrix&   operator _Op##=(T scalar); \
    ScalarMatrix    operator _Op(T scalar) const;

    DECL_SCALARMATRIX_SCALAR_OP_LHS(+)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(-)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(*)
    DECL_SCALARMATRIX_SCALAR_OP_LHS(/)

#undef DECL_SCALARMATRIX_SCALAR_OP_LHS

#define DECL_SCALARMATRIX_SCALAR_OP_RHS(_Op) \
    template <typename T, size_t _Width, size_t _Height> \
    friend ScalarMatrix<T, _Width, _Height> operator _Op(T lhs, const ScalarMatrix<T, _Width, _Height>& rhs); \
    template <typename U, typename T, size_t _Width, size_t _Height> \
    friend ScalarMatrix<T, _Width, _Height> operator _Op(U lhs, const ScalarMatrix<T, _Width, _Height>& rhs);

    DECL_SCALARMATRIX_SCALAR_OP_RHS(+)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(-)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(*)
    DECL_SCALARMATRIX_SCALAR_OP_RHS(/)

#undef DECL_SCALARMATRIX_SCALAR_OP_RHS

    ScalarMatrix    operator -() const;

    ScalarMatrix&   operator +=(const ScalarMatrix& other);
    ScalarMatrix&   operator -=(const ScalarMatrix& other);

    ScalarMatrix    operator +(const ScalarMatrix& other) const;
    ScalarMatrix    operator -(const ScalarMatrix& other) const;

    template <size_t _NWidth>
    ScalarMatrix<T, _NWidth, _Height> operator *(const ScalarMatrix<T, _NWidth, _Width>& other) const {
        return Multiply(other);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T, size_t _Width, size_t _Height>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const ScalarMatrix<T, _Width, _Height>& v) {
    for (size_t i = 0; i < _Height; ++i)
        oss << v.Row(i) << std::endl;
    return oss;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
void swap(ScalarMatrix<T, _Width, _Height>& lhs, ScalarMatrix<T, _Width, _Height>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Transform/ScalarMatrix-inl.h"
