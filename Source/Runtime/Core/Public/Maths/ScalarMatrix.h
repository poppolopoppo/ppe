#pragma once

#include "Core.h"

#include "Maths/ScalarMatrix_fwd.h"
#include "Maths/ScalarVector.h"

#include "HAL/PlatformMemory.h"
#include "IO/TextWriter_fwd.h"

#include <initializer_list>

PRAGMA_DISABLE_RUNTIMECHECKS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
// TScalarMatrixIdentity
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
struct TScalarMatrixIdentity {
    NODISCARD inline CONSTEXPR T Get(u32 row, u32 col) const NOEXCEPT {
        return static_cast<T>(col == row ? 1 : 0);
    }
};
//----------------------------------------------------------------------------
// TScalarMatrixLiteral
//----------------------------------------------------------------------------
template <typename T>
struct TScalarMatrixLiteral {
    const T Literal;

    inline CONSTEXPR explicit TScalarMatrixLiteral(T&& literal) NOEXCEPT
        : Literal(std::move(literal))
    {}

    NODISCARD inline CONSTEXPR const T& Get(u32 , u32 ) const NOEXCEPT {
        return Literal;
    }
};
//----------------------------------------------------------------------------
template <u32 _Rows, u32 _Cols, typename T>
NODISCARD inline CONSTEXPR decltype(auto) MakeScalarMatrixLiteral(T&& literal) NOEXCEPT {
    using literal_type = TScalarMatrixLiteral<std::decay_t<T>>;
    return TScalarMatrixExpr<std::decay_t<T>, _Rows, _Cols, literal_type>{ std::forward<T>(literal) };
}
//----------------------------------------------------------------------------
// TScalarMatrixRef
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Ref, bool _bConst>
struct TScalarMatrixRef {
    using expression_type = Meta::TAddConstIFN<
        TScalarMatrixExpr<T, _Rows, _Cols, _Ref>,
        _bConst >;

    expression_type* Ref{};

    inline CONSTEXPR explicit TScalarMatrixRef(expression_type* ref) NOEXCEPT
        : Ref(ref)
    {}

    NODISCARD inline CONSTEXPR decltype(auto) Get(u32 row, u32 col) const NOEXCEPT {
        return Ref->Get(row, col);
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Ref>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixRef(TScalarMatrixExpr<T, _Rows, _Cols, _Ref>& ref) NOEXCEPT {
    return TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixRef<T, _Rows, _Cols, _Ref, false>>{ std::addressof(ref) };
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Ref>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixRef(const TScalarMatrixExpr<T, _Rows, _Cols, _Ref>& ref) NOEXCEPT {
    return TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixRef<T, _Rows, _Cols, _Ref, true>>{ std::addressof(ref) };
}
//----------------------------------------------------------------------------
template <u32 _NRows, u32 _NCols, typename T, u32 _Rows, u32 _Cols, typename _Ref>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixView(TScalarMatrixExpr<T, _Rows, _Cols, _Ref>& ref) NOEXCEPT
    requires (_NRows <= _Rows and _NCols <= _Cols) {
    return TScalarMatrixExpr<T, _NRows, _NCols, TScalarMatrixRef<T, _Rows, _Cols, _Ref, false>>{ std::addressof(ref) };
}
//----------------------------------------------------------------------------
template <u32 _NRows, u32 _NCols, typename T, u32 _Rows, u32 _Cols, typename _Ref>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixView(const TScalarMatrixExpr<T, _Rows, _Cols, _Ref>& ref) NOEXCEPT
    requires (_NRows <= _Rows and _NCols <= _Cols) {
    return TScalarMatrixExpr<T, _NRows, _NCols, TScalarMatrixRef<T, _Rows, _Cols, _Ref, true>>{ std::addressof(ref) };
}
//----------------------------------------------------------------------------
// TScalarMatrixAssignable
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Expr>
struct TScalarMatrixAssignable : _Expr {
    using expression_type = _Expr;
    using expression_type::expression_type;
    using expression_type::Get;
    using expression_type::data;

    static_assert(sizeof(T) * _Rows * _Cols <= sizeof(expression_type::data), "invalid assignable matrix dimension");

    inline CONSTEXPR TScalarMatrixAssignable(Meta::FForceInit) NOEXCEPT {
        Broadcast(Meta::MakeForceInit<T>());
    }

    inline CONSTEXPR TScalarMatrixAssignable(Meta::FDefaultValue) NOEXCEPT {
        Broadcast(Meta::DefaultValue<T>());
    }

    template <typename _Cast, typename _Other>
    inline CONSTEXPR explicit TScalarMatrixAssignable(const TScalarMatrixExpr<_Cast, _Rows, _Cols, _Other>& other) NOEXCEPT {
        Assign(other);
    }

    template <typename _Cast, typename _Other>
    inline CONSTEXPR explicit TScalarMatrixAssignable(TScalarMatrixExpr<_Cast, _Rows, _Cols, _Other>&& rother) NOEXCEPT {
        Assign(std::move(rother));
    }

    template <typename _Diagonal>
    inline CONSTEXPR explicit TScalarMatrixAssignable(const TScalarVectorExpr<T, Min(_Rows, _Cols), _Diagonal>& diagonal) NOEXCEPT
        : TScalarMatrixAssignable(Meta::ForceInit) {
        SetDiagonal(diagonal);
    }

    template <typename _Other>
    inline CONSTEXPR void Assign(const TScalarMatrixExpr<T, _Rows, _Cols, _Other>& other) NOEXCEPT {
        Meta::static_for<u32, _Rows, _Cols>([&](auto... it) {
            FOLD_EXPR( Get(it.first, it.second) = static_cast<T>(other.template Get<it.first, it.second>()) );
        });
    }

    template <typename _Other>
    inline CONSTEXPR void Assign(TScalarMatrixExpr<T, _Rows, _Cols, _Other>&& rvalue) NOEXCEPT {
        Meta::static_for<u32, _Rows, _Cols>([&](auto... it) {
            FOLD_EXPR( Get(it.first, it.second) = std::move(rvalue.template Get<it.first, it.second>()) );
        });
    }

    inline CONSTEXPR void Broadcast(const T& value) NOEXCEPT {
        Meta::static_for<u32, _Rows, _Cols>([&](auto... it) {
            FOLD_EXPR( Get(it.first, it.second) = value );
        });
    }

    template <typename _Other>
    inline CONSTEXPR void SetColumn(u32 col, const TScalarVectorExpr<T, _Rows, _Other>& value) NOEXCEPT {
        Meta::static_for<u32, _Rows>([&](auto... row) {
            FOLD_EXPR( Get(row, col) = value.template Get<row>() );
        });
    }

    template <typename _Other>
    inline CONSTEXPR void SetRow(u32 row, const TScalarVectorExpr<T, _Cols, _Other>& value) NOEXCEPT {
        Meta::static_for<u32, _Cols>([&](auto... col) {
            FOLD_EXPR( Get(row, col) = value.template Get<col>()) ;
        });
    }

    template <u32 _Col, typename _Other>
    inline CONSTEXPR Meta::TEnableIf<_Col < _Rows> SetColumn(const TScalarVectorExpr<T, _Rows, _Other>& value) NOEXCEPT
        requires (_Col < _Cols) {
        Meta::static_for<u32, _Rows>([&](auto... row) {
            FOLD_EXPR( Get(row, _Col) = value.template Get<row>() );
        });
    }

    template <u32 _Row, typename _Other>
    inline CONSTEXPR Meta::TEnableIf<_Row < _Cols> SetRow(const TScalarVectorExpr<T, _Cols, _Other>& value) NOEXCEPT
        requires (_Row < _Rows) {
        Meta::static_for<u32, _Cols>([&](auto... col) {
            FOLD_EXPR( Get(_Row, col) = value.template Get<col>() );
        });
    }

    template <typename _Other>
    inline CONSTEXPR void SetDiagonal(const TScalarVectorExpr<T, Min(_Rows, _Cols), _Other>& value) NOEXCEPT {
        Meta::static_for<u32, Min(_Rows, _Cols)>([&](auto... idx) {
            FOLD_EXPR( Get(idx, idx) = value.template Get<idx>() );
        });
    }

    inline friend CONSTEXPR void swap(
        TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixAssignable>& lhs,
        TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixAssignable>& rhs) NOEXCEPT {
        Meta::static_for<u32, _Rows, _Cols>([&](auto... it) {
            FOLD_EXPR( std::swap(lhs.template Get<it.first, it.second>(), rhs.template Get<it.first, it.second>()) );
        });
    }

    static CONSTEXPR const TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixLiteral<T>> One{ static_cast<T>(1) };
    static CONSTEXPR const TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixLiteral<T>> Zero{ static_cast<T>(0) };
    static CONSTEXPR const TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixLiteral<T>> MinusOne{ static_cast<T>(-1) };

    static CONSTEXPR const TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixLiteral<T>> MaxValue{ TNumericLimits<T>::MaxValue() };
    static CONSTEXPR const TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixLiteral<T>> MinValue{ TNumericLimits<T>::MinValue() };
    static CONSTEXPR const TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixLiteral<T>> Lowest{ TNumericLimits<T>::Lowest() };

    static CONSTEXPR const TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixIdentity<T, _Rows, _Cols>> Identity{};
};
//----------------------------------------------------------------------------
// TScalarMatrixTranspose<T, w, H>
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
struct TScalarMatrixTranspose {
    T data[_Rows][_Cols];

    NODISCARD inline CONSTEXPR T& Get(u32 col, u32 row) NOEXCEPT {
        return data[row][col]; // transposed
    }

    NODISCARD inline CONSTEXPR const T& Get(u32 col, u32 row) const NOEXCEPT {
        return data[row][col]; // transposed
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
using TScalarMatrixTransposeAssignable = TScalarMatrixExpr<T, _Cols, _Rows,
    TScalarMatrixAssignable<T, _Cols, _Rows,
        TScalarMatrixTranspose<T, _Rows, _Cols>
    >
>;
//----------------------------------------------------------------------------
// TScalarMatrixStorage<T, w, H> (ROW-MAJOR)
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
struct TScalarMatrixStorage {
    union {
        T data[_Rows * _Cols];
        T m[_Rows][_Cols];
        TScalarVector<T, _Cols> rows[_Rows];
        TScalarMatrixTransposeAssignable<T, _Rows, _Cols> transposed;
    };

    inline CONSTEXPR TScalarMatrixStorage() NOEXCEPT
        : data{}
    {}

    inline CONSTEXPR TScalarMatrixStorage(Meta::FNoInit) NOEXCEPT
    {}

    inline CONSTEXPR TScalarMatrixStorage(T broadcast) NOEXCEPT
        : data{ broadcast } {
    }

    template <typename... _Args, decltype(decltype(data){std::forward<_Args>(std::declval<_Args&&>())...})* = nullptr>
    inline CONSTEXPR TScalarMatrixStorage(_Args&&... args) NOEXCEPT
        : data{ std::forward<_Args>(args)... }
    {}

    template <typename... _Args, decltype(decltype(rows){std::declval<const TScalarVectorExpr<T, _Rows, _Args>&>()...})* = nullptr>
    inline CONSTEXPR TScalarMatrixStorage(const TScalarVectorExpr<T, _Cols, _Args>&... args) NOEXCEPT
        : rows{ args... }
    {}

    // allow promotion from {...} to TScalarVector<> without specializing TScalarMatricxStorage<> for each dimension
    template <class = void>
    inline CONSTEXPR TScalarMatrixStorage(const TScalarVector<T, _Cols>& row0) NOEXCEPT
        requires (_Rows == 1)
        : rows{ row0 }
    {}
    template <class = void>
    inline CONSTEXPR TScalarMatrixStorage(const TScalarVector<T, _Cols>& row0, const TScalarVector<T, _Cols>& row1) NOEXCEPT
        requires (_Rows == 2)
        : rows{ row0, row1 }
    {}
    template <class = void>
    inline CONSTEXPR TScalarMatrixStorage(const TScalarVector<T, _Cols>& row0, const TScalarVector<T, _Cols>& row1, const TScalarVector<T, _Cols>& row2) NOEXCEPT
        requires (_Rows == 3)
        : rows{ row0, row1, row2 }
    {}
    template <class = void>
    inline CONSTEXPR TScalarMatrixStorage(const TScalarVector<T, _Cols>& row0, const TScalarVector<T, _Cols>& row1, const TScalarVector<T, _Cols>& row2, const TScalarVector<T, _Cols>& row3) NOEXCEPT
        requires (_Rows == 4)
        : rows{ row0, row1, row2, row3 }
    {}

    NODISCARD inline CONSTEXPR T& Get(u32 row, u32 col) NOEXCEPT {
        return m[row][col];
    }

    NODISCARD inline CONSTEXPR const T& Get(u32 row, u32 col) const NOEXCEPT {
        return m[row][col];
    }

};
//----------------------------------------------------------------------------
// TScalarMatrixShuffle
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Expr>
struct TScalarMatrixExprBase;
template <typename T, u32 _Rows, u32 _Cols, typename _Ref, auto... _Shuffle>
struct TScalarMatrixShuffle {
    using expression_type = const TScalarMatrixExprBase<T, _Rows, _Cols, _Ref>;

    expression_type* Ref{};

    inline CONSTEXPR explicit TScalarMatrixShuffle(expression_type* ref) NOEXCEPT
        : Ref(ref)
    {}

    NODISCARD inline CONSTEXPR decltype(auto) Get(u32 index) const NOEXCEPT {
        static CONSTEXPR const std::pair<u32, u32> shuffle_v[] = { {_Shuffle.first, _Shuffle.second}... };
        const std::pair<u32, u32>& it = shuffle_v[index];
        return Ref->Get(it.first, it.second);
    }
};
//----------------------------------------------------------------------------
template <auto... _Shuffle, typename T, u32 _Rows, u32 _Cols, typename _Ref>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixShuffle(const TScalarMatrixExprBase<T, _Rows, _Cols, _Ref>& ref) NOEXCEPT
    requires (... && (_Shuffle.first <= _Rows and _Shuffle.second <= _Cols)) {
    return TScalarVectorExpr<T, sizeof...(_Shuffle), TScalarMatrixShuffle<T, _Rows, _Cols, _Ref, _Shuffle...>>{ std::addressof(ref) };
}
//----------------------------------------------------------------------------
// TScalarMatrixCrop
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Ref, u32 _OffsetR, u32 _OffsetC>
struct TScalarMatrixCrop {
    using expression_type = const TScalarMatrixExprBase<T, _Rows, _Cols, _Ref>;

    expression_type* Ref{};

    inline CONSTEXPR explicit TScalarMatrixCrop(expression_type* ref) NOEXCEPT
        : Ref(ref)
    {}

    NODISCARD inline CONSTEXPR decltype(auto) Get(u32 row, u32 col) const NOEXCEPT {
        return Ref->Get(row + _OffsetR, col + _OffsetC);
    }
};
//----------------------------------------------------------------------------
template <u32 _NRows, u32 _NCols, u32 _OffsetR, u32 _OffsetC, typename T, u32 _Rows, u32 _Cols, typename _Ref>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixCrop(const TScalarMatrixExprBase<T, _Rows, _Cols, _Ref>& ref) NOEXCEPT
    requires (_OffsetR + _NRows <= _Rows and _OffsetC + _NCols <= _Cols) {
    return TScalarMatrixExpr<T, _NRows, _NCols, TScalarMatrixCrop<T, _Rows, _Cols, _Ref, _OffsetR, _OffsetC>>{ std::addressof(ref) };
}
//----------------------------------------------------------------------------
// TScalarMatrixHomogeneous
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Ref>
struct TScalarMatrixHomogeneous {
    using expression_type = const TScalarMatrixExprBase<T, _Rows, _Cols, _Ref>;

    expression_type* Ref{};

    inline CONSTEXPR explicit TScalarMatrixHomogeneous(expression_type* ref) NOEXCEPT
        : Ref(ref)
    {}

    NODISCARD inline CONSTEXPR decltype(auto) Get(u32 row, u32 col) const NOEXCEPT {
        return (row < _Rows && col < _Cols
            ? Ref->Get(row, col)
            : (row == col ? T(1) : T(0)));
    }
};
//----------------------------------------------------------------------------
template <u32 _NRows, u32 _NCols, typename T, u32 _Rows, u32 _Cols, typename _Ref>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixHomogeneous(const TScalarMatrixExprBase<T, _Rows, _Cols, _Ref>& ref) NOEXCEPT
    requires ((_NRows > _Rows or _NCols > _Cols) and (_NRows >= _Rows and _NCols >= _Cols)) {
    return TScalarMatrixExpr<T, _NRows, _NCols, TScalarMatrixHomogeneous<T, _Rows, _Cols, _Ref>>{ std::addressof(ref) };
}
//----------------------------------------------------------------------------
// TScalarMatrixExprBase
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Expr>
struct TScalarMatrixExprBase : _Expr {
    STATIC_ASSERT(_Rows > 0 && _Cols > 0);

    using expression_type = _Expr;
    using expression_type::expression_type;

    STATIC_CONST_INTEGRAL(u32, Rows, _Rows);
    STATIC_CONST_INTEGRAL(u32, Cols, _Cols);
    STATIC_CONST_INTEGRAL(u32, Dim, _Rows * _Cols);
    STATIC_CONST_INTEGRAL(u32, DiagonalDim, Min(_Rows, _Cols));

    using component_type = T;
    using reference_type = decltype(std::declval<expression_type&>().Get(0, 0));
    using const_reference_type = decltype(std::declval<const expression_type&>().Get(0, 0));

    using column_type = TScalarVector<T, _Rows>;
    using row_type = TScalarVector<T, _Cols>;
    using diagonal_type = TScalarVector<T, DiagonalDim>;

    CONSTEXPR TScalarMatrixExprBase() NOEXCEPT = default;

    template <u32 _Row, u32 _Col>
    CONSTEXPR reference_type Get() NOEXCEPT
        requires (_Row < _Rows and _Col < _Cols) {
        return expression_type::Get(_Row, _Col);
    }

    template <u32 _Row, u32 _Col>
    CONSTEXPR const_reference_type Get() const NOEXCEPT
        requires (_Row < _Rows and _Col < _Cols) {
        return expression_type::Get(_Row, _Col);
    }

    CONSTEXPR reference_type Get(u32 row, u32 col) NOEXCEPT {
        AssertMessage("out-of-bounds", row < _Rows && col < _Cols);
        return expression_type::Get(row, col);
    }

    CONSTEXPR const_reference_type Get(u32 row, u32 col) const NOEXCEPT {
        AssertMessage("out-of-bounds", row < _Rows && col < _Cols);
        return expression_type::Get(row, col);
    }

    FORCE_INLINE CONSTEXPR reference_type UnsafeGet(u32 row, u32 col) NOEXCEPT {
        return expression_type::Get(row, col);
    }

    FORCE_INLINE CONSTEXPR const_reference_type UnsafeGet(u32 row, u32 col) const NOEXCEPT {
        return expression_type::Get(row, col);
    }

    NODISCARD CONSTEXPR column_type Column(u32 col) const NOEXCEPT {
        return Meta::static_for<u32, _Rows>([&](auto... row) CONSTEXPR -> column_type {
            return { Get(row, col)... };
        });
    }

    NODISCARD CONSTEXPR row_type Row(u32 row) const NOEXCEPT {
        return Meta::static_for<u32, _Cols>([&](auto... col) CONSTEXPR -> row_type {
            return { Get(row, col)... };
        });
    }

    template <u32 _Col, u32 _NRows = _Rows>
    NODISCARD CONSTEXPR decltype(auto) Column() const NOEXCEPT
        requires (_Col < _Cols and _NRows <= _Rows) {
        return Meta::static_for<u32, _NRows, 1>([&](auto... it) CONSTEXPR -> auto {
            return MakeScalarMatrixShuffle<Meta::integral_constant_pair<u32, it.first, it.second + _Col>{}...>(*this);
        });
    }

    template <u32 _Row, u32 _NCols = _Cols>
    NODISCARD CONSTEXPR decltype(auto) Row() const NOEXCEPT
        requires (_Row < _Rows and _NCols <= _Cols) {
        return Meta::static_for<u32, 1, _NCols>([&](auto... it) CONSTEXPR -> auto {
            return MakeScalarMatrixShuffle<Meta::integral_constant_pair<u32, it.first + _Row, it.second>{}...>(*this);
        });
    }

    NODISCARD CONSTEXPR decltype(auto) Diagonal() const NOEXCEPT {
        return Meta::static_for<u32, DiagonalDim, 1>([&](auto... it) CONSTEXPR -> auto {
            return MakeScalarMatrixShuffle<Meta::integral_constant_pair<u32, it.first, it.first>{}...>(*this);
        });
    }

    NODISCARD CONSTEXPR T Trace() const NOEXCEPT {
        return Meta::static_for<u32, DiagonalDim>([&](auto... idx) CONSTEXPR {
            return static_cast<T>((... + Get<idx, idx>()));
        });
    }

    template <u32 _NRows, u32 _NCols, u32 _OffsetR = 0, u32 _OffsetC = 0>
    NODISCARD CONSTEXPR decltype(auto) Crop() const NOEXCEPT
        requires (_NRows + _OffsetR <= _Rows && _NCols + _OffsetC <= _Cols) {
        return MakeScalarMatrixCrop<_NRows, _NCols, _OffsetR, _OffsetC>(*this);
    }

    template <u32 _NDim = Min(_Rows + 1, _Cols + 1)>
    NODISCARD CONSTEXPR decltype(auto) OneExtend() const NOEXCEPT
        requires ((_NDim > _Rows or _NDim > _Cols) and (_NDim >= _Rows and _NDim >= _Cols)) {
        return MakeScalarMatrixHomogeneous<_NDim, _NDim>(*this);
    }

    NODISCARD CONSTEXPR decltype(auto) Homogeneous() const NOEXCEPT
        requires (_Rows <= 4 and _Cols <= 4) {
        IF_CONSTEXPR(_Rows != _Cols or _Rows < 4)
            return OneExtend<Min(_Rows + 1, _Cols + 1)>();
        else
            return (*this); // noop if already 4x4
    }

    NODISCARD CONSTEXPR decltype(auto) Transpose() const NOEXCEPT {
        return Meta::static_for<u32, _Cols, _Rows>([&](auto... it) CONSTEXPR -> TScalarMatrix<T, _Cols, _Rows> {
            return { Get<it.second, it.first>()... };
        });
    }

    template <u32 _NCols, typename _Other>
    NODISCARD CONSTEXPR decltype(auto) Multiply(const TScalarMatrixExpr<T, _Cols, _NCols, _Other>& other) const NOEXCEPT {
        return Meta::static_for<u32, _Rows, _NCols>([&](auto... it) CONSTEXPR -> TScalarMatrix<T, _Rows, _NCols> {
            return {
                ((void)it, Meta::static_for<u32, _Cols>([&](auto... k) CONSTEXPR {
                    using row = decltype(it.first);
                    using col = decltype(it.second);
                    return (... + (Get<row{}, k>() * other.template Get<k, col{}>()));
                }))...
            };
        });
    }

    template <u32 _NCols, typename _Other>
    NODISCARD CONSTEXPR auto operator *(const TScalarMatrixExpr<T, _Cols, _NCols, _Other>& other) const NOEXCEPT {
        return Multiply(other);
    }

    template <typename _Vec>
    NODISCARD CONSTEXPR TScalarVector<T, _Rows> Multiply(const TScalarVectorExpr<T, _Cols, _Vec>& vec) const NOEXCEPT {
        return Meta::static_for<u32, _Rows>([&](auto... it) CONSTEXPR -> TScalarVector<T, _Rows> {
            return {
                ((void)it, Meta::static_for<u32, _Cols>([&](auto... col) CONSTEXPR {
                    using row = decltype(it);
                    return (... + (Get<row{}, col>() * vec.template Get<col>()));
                }))...
            };
        });
    }

    template <typename _Vec>
    NODISCARD CONSTEXPR auto operator *(const TScalarVectorExpr<T, _Cols, _Vec>& vec) const NOEXCEPT {
        return Multiply(vec);
    }

    template <typename _Vec>
    NODISCARD CONSTEXPR friend auto operator *(const TScalarVectorExpr<T, _Rows, _Vec>& vec, const TScalarMatrixExprBase& mat) NOEXCEPT {
        return Meta::static_for<u32, _Cols>([&](auto... it) CONSTEXPR -> TScalarVector<T, _Cols> {
            return {
                ((void)it, Meta::static_for<u32, _Rows>([&](auto... row) CONSTEXPR {
                    using col = decltype(it);
                    return (... + (mat.Get<row, col{}>() * vec.template Get<row>()));
                }))...
            };
        });
    }

    template <typename _Vec>
    NODISCARD CONSTEXPR TScalarVector<T, _Rows> Multiply_OneExtend(const TScalarVectorExpr<T, _Cols - 1, _Vec>& vec) const NOEXCEPT {
        return Meta::static_for<u32, _Rows>([&](auto... it) CONSTEXPR -> TScalarVector<T, _Rows> {
            return {
                ((void)it, Get<it, _Cols - 1>() + Meta::static_for<u32, _Cols - 1>([&](auto... col) CONSTEXPR {
                    using row = decltype(it);
                    return (... + (Get<row{}, col>() * vec.template Get<col>()));
                }))...
            };
        });
    }

    template <typename _Vec>
    NODISCARD CONSTEXPR TScalarVector<T, _Rows> Multiply_ZeroExtend(const TScalarVectorExpr<T, _Cols - 1, _Vec>& vec) const NOEXCEPT {
        return Meta::static_for<u32, _Rows>([&](auto... it) CONSTEXPR -> TScalarVector<T, _Rows> {
            return {
                ((void)it, Meta::static_for<u32, _Cols - 1>([&](auto... col) CONSTEXPR {
                    using row = decltype(it);
                    return (... + (Get<row{}, col>() * vec.template Get<col>()));
                }))...
            };
        });
    }

    NODISCARD CONSTEXPR row_type operator [](u32 row) const NOEXCEPT {
        return Row(row);
    }

    NODISCARD CONSTEXPR reference_type operator ()(u32 row, u32 col) NOEXCEPT {
        return Get(row, col);
    }

    NODISCARD CONSTEXPR const_reference_type operator ()(u32 row, u32 col) const NOEXCEPT {
        return Get(row, col);
    }

};
//----------------------------------------------------------------------------
// TScalarMatrixExpr
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Expr>
struct TScalarMatrixExpr : TScalarMatrixExprBase<T, _Rows, _Cols, _Expr> {
    using base_type = TScalarMatrixExprBase<T, _Rows, _Cols, _Expr>;
    using base_type::base_type;
    using base_type::Get;
    using base_type::operator [];
    using base_type::operator ();

    CONSTEXPR TScalarMatrixExpr() NOEXCEPT = default;
    CONSTEXPR ~TScalarMatrixExpr() NOEXCEPT = default;

    TScalarMatrixExpr(const TScalarMatrixExpr& ) = delete;
    TScalarMatrixExpr& operator =(const TScalarMatrixExpr& ) = delete;

    CONSTEXPR TScalarMatrixExpr(TScalarMatrixExpr&& ) = default;
    TScalarMatrixExpr& operator =(TScalarMatrixExpr&& ) = delete;
};
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Expr>
struct TScalarMatrixExpr<T, _Rows, _Cols, TScalarMatrixAssignable<T, _Rows, _Cols, _Expr>> : TScalarMatrixExprBase<T, _Rows, _Cols, TScalarMatrixAssignable<T, _Rows, _Cols, _Expr>> {
    using assignable_type = TScalarMatrixAssignable<T, _Rows, _Cols, _Expr>;
    using assignable_type::Assign;

    using base_type = TScalarMatrixExprBase<T, _Rows, _Cols, assignable_type>;
    using base_type::base_type;
    using base_type::Get;
    using base_type::operator [];
    using base_type::operator ();

    CONSTEXPR TScalarMatrixExpr() = default;
    CONSTEXPR ~TScalarMatrixExpr() = default;

    CONSTEXPR TScalarMatrixExpr(const TScalarMatrixExpr& other) {
        Assign(other);
    }

    CONSTEXPR TScalarMatrixExpr& operator =(const TScalarMatrixExpr& other) {
        Assign(other);
        return (*this);
    }

    CONSTEXPR TScalarMatrixExpr(TScalarMatrixExpr&& rvalue) NOEXCEPT {
        Assign(std::move(rvalue));
    }

    CONSTEXPR TScalarMatrixExpr& operator =(TScalarMatrixExpr&& rvalue) NOEXCEPT {
        Assign(std::move(rvalue));
        return (*this);
    }

    template <typename _Other>
    inline CONSTEXPR TScalarMatrixExpr(const TScalarMatrixExpr<T, _Rows, _Cols, _Other>& other) {
        Assign(other);
    }

    template <typename _Other>
    inline CONSTEXPR TScalarMatrixExpr& operator =(const TScalarMatrixExpr<T, _Rows, _Cols, _Other>& other) {
        Assign(other);
        return (*this);
    }

    template <u32 _NRows, u32 _NCols, typename _Other>
    inline CONSTEXPR explicit TScalarMatrixExpr(const TScalarMatrixExpr<T, _NRows, _NCols, _Other>& other) NOEXCEPT
        requires (_NRows >= _Rows and _NCols >= _Cols) {
        Assign(MakeScalarMatrixView<_Rows, _Cols>(other));
    }

    template <typename _Other>
    CONSTEXPR TScalarMatrixExpr& operator *=(const TScalarMatrixExpr<T, _Rows, _Cols, _Other>& other) NOEXCEPT {
        Assign(Multiply(other));
        return (*this);
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs>
NODISCARD inline CONSTEXPR bool operator ==(const TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs, const TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>& rhs) NOEXCEPT {
    return Meta::static_for<u32, _Rows, _Cols>([&](auto... it) -> bool {
        return (... && (lhs.template Get<it.first, it.second>() == rhs.template Get<it.first, it.second>()));
    });
}
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs>
NODISCARD inline CONSTEXPR bool operator !=(const TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs, const TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>& rhs) NOEXCEPT {
    return (not operator ==(lhs, rhs));
}
//----------------------------------------------------------------------------
// TScalarMatrixUnaryOp
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Prm, typename _Op>
struct TScalarMatrixUnaryOp : private _Op {
    const TScalarMatrixExpr<T, _Rows, _Cols, _Prm>& Prm;

    inline CONSTEXPR explicit TScalarMatrixUnaryOp(
        const TScalarMatrixExpr<T, _Rows, _Cols, _Prm>& prm,
        _Op&& unaryOp ) NOEXCEPT
        : _Op(std::move(unaryOp))
        , Prm(prm)
    {}

    NODISCARD inline CONSTEXPR auto Get(u32 row, u32 col) const NOEXCEPT {
        return _Op::operator ()(Prm.Get(row, col));
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Prm, typename _Op>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixOp(
    const TScalarMatrixExpr<T, _Rows, _Cols, _Prm>& prm,
    _Op&& unaryOp) {
    using op_type = TScalarMatrixUnaryOp<T, _Rows, _Cols, _Prm, _Op>;
    using destination_type = decltype(unaryOp(std::declval<T>()));
    return TScalarMatrixExpr<destination_type, _Rows, _Cols, op_type>{ prm, std::forward<_Op>(unaryOp) };
}
//----------------------------------------------------------------------------
#define PPE_SCALARMATRIX_UNARYOP_DECL(_FunctionName, ...) \
    template <typename T, u32 _Rows, u32 _Cols, typename _Prm> \
    NODISCARD inline CONSTEXPR auto _FunctionName(const details::TScalarMatrixExpr<T, _Rows, _Cols, _Prm>& prm) NOEXCEPT { \
        return details::MakeScalarMatrixOp(prm, __VA_ARGS__); \
    }
#define PPE_SCALARMATRIX_UNARYOP_FUNC(_FunctionName) \
    PPE_SCALARMATRIX_UNARYOP_DECL(_FunctionName, [](const T& x) CONSTEXPR NOEXCEPT { return PPE::_FunctionName(x); })
//----------------------------------------------------------------------------
PPE_SCALARMATRIX_UNARYOP_DECL(operator !, std::bit_not<T>{})
PPE_SCALARMATRIX_UNARYOP_DECL(operator -, std::negate<T>{})
PPE_SCALARMATRIX_UNARYOP_DECL(operator ~, Meta::TUnaryComplement<T>{})
//----------------------------------------------------------------------------
//#undef PPE_SCALARMATRIX_UNARYOP_DECL // let it spill for ScalarVectorHelpers.h
//#undef PPE_SCALARMATRIX_UNARYOP_FUNC
//----------------------------------------------------------------------------
// TScalarMatrixBinaryOp
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs, typename _Op>
struct TScalarMatrixBinaryOp : private _Op {
    TScalarMatrixExpr<T, _Rows, _Cols, _Lhs> Lhs;
    TScalarMatrixExpr<T, _Rows, _Cols, _Rhs> Rhs;

    inline CONSTEXPR explicit TScalarMatrixBinaryOp(
        TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>&& __restrict lhs,
        TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>&& __restrict rhs,
        _Op&& binaryOp) NOEXCEPT
        : _Op(std::move(binaryOp))
        , Lhs(std::move(lhs)), Rhs(std::move(rhs))
    {}

    NODISCARD inline CONSTEXPR auto Get(u32 row, u32 col) const NOEXCEPT {
        return _Op::operator ()(Lhs.Get(row, col), Rhs.Get(row, col));
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs, typename _Op>
NODISCARD inline CONSTEXPR auto MakeScalarMatrixOp(
    TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>&& __restrict lhs,
    TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>&& __restrict rhs,
    _Op&& binaryOp) {
    using op_type = TScalarMatrixBinaryOp<T, _Rows, _Cols, _Lhs, _Rhs, _Op>;
    using destination_type = decltype(binaryOp(std::declval<T>(), std::declval<T>()));
    return TScalarMatrixExpr<destination_type, _Rows, _Cols, op_type>{ std::move(lhs), std::move(rhs),std::forward<_Op>(binaryOp) };
}
//----------------------------------------------------------------------------
#define PPE_SCALARMATRIX_BINARYOP_DECL(_FunctionName, ...) \
    template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs> \
    inline CONSTEXPR auto _FunctionName(const details::TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs, const details::TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>& rhs) NOEXCEPT { \
        return details::MakeScalarMatrixOp(details::MakeScalarMatrixRef(lhs), details::MakeScalarMatrixRef(rhs), __VA_ARGS__); \
    } \
    template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs> \
    inline CONSTEXPR auto& CONCAT(_FunctionName, =)(details::TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs, const details::TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>& rhs) NOEXCEPT { \
        lhs = details::MakeScalarMatrixOp(details::MakeScalarMatrixRef(lhs), details::MakeScalarMatrixRef(rhs), __VA_ARGS__); \
        return lhs; \
    }
#define PPE_SCALARMATRIX_BINARYOP_VECTOR(_FunctionName, ...) \
    template <typename T, u32 _Rows, u32 _Cols, typename _Lhs> \
    inline CONSTEXPR auto _FunctionName(const details::TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs, Meta::TDontDeduce<T> rhs) NOEXCEPT { \
        return details::MakeScalarMatrixOp(MakeScalarMatrixRef(lhs), details::MakeScalarMatrixLiteral<_Rows, _Cols>(std::move(rhs)), __VA_ARGS__); \
    } \
    template <typename T, u32 _Rows, u32 _Cols, typename _Rhs> \
    inline CONSTEXPR auto _FunctionName(Meta::TDontDeduce<T> lhs, const details::TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>& rhs) NOEXCEPT { \
        return details::MakeScalarMatrixOp(details::MakeScalarMatrixLiteral<_Rows, _Cols>(std::move(lhs)), details::MakeScalarMatrixRef(rhs), __VA_ARGS__); \
    }
#define PPE_SCALARMATRIX_BINARYOP_OPERATOR(_FunctionName, ...) \
    PPE_SCALARMATRIX_BINARYOP_VECTOR(_FunctionName, __VA_ARGS__) \
    template <typename T, u32 _Rows, u32 _Cols, typename _Lhs> \
    inline CONSTEXPR auto& CONCAT(_FunctionName, =)(details::TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs, Meta::TDontDeduce<T> rhs) NOEXCEPT { \
        lhs = details::MakeScalarMatrixOp(details::MakeScalarMatrixRef(lhs), details::MakeScalarMatrixLiteral<_Rows, _Cols>(std::move(rhs)), __VA_ARGS__); \
        return lhs; \
    }
#define PPE_SCALARMATRIX_BINARYOP_FUNC(_FunctionName) \
    PPE_SCALARMATRIX_BINARYOP_VECTOR(_FunctionName, [](const T& a, const T& b) CONSTEXPR NOEXCEPT { return PPE::_FunctionName(a, b); })
//----------------------------------------------------------------------------
PPE_SCALARMATRIX_BINARYOP_DECL(operator +, std::plus<T>{})
PPE_SCALARMATRIX_BINARYOP_OPERATOR(operator +, std::plus<T>{})
PPE_SCALARMATRIX_BINARYOP_DECL(operator -, std::minus<T>{})
PPE_SCALARMATRIX_BINARYOP_OPERATOR(operator -, std::minus<T>{})
PPE_SCALARMATRIX_BINARYOP_OPERATOR(operator *, std::multiplies<T>{})
PPE_SCALARMATRIX_BINARYOP_OPERATOR(operator /, std::divides<T>{})
PPE_SCALARMATRIX_BINARYOP_OPERATOR(operator &, std::bit_and<T>{})
PPE_SCALARMATRIX_BINARYOP_OPERATOR(operator |, std::bit_or<T>{})
PPE_SCALARMATRIX_BINARYOP_OPERATOR(operator ^, std::bit_xor<T>{})
//----------------------------------------------------------------------------
// Text format
//----------------------------------------------------------------------------
template <typename _Char, typename T, u32 _Rows, u32 _Cols, typename _Expr>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TScalarMatrixExpr<T, _Rows, _Cols, _Expr>& m) {
    oss << STRING_LITERAL(_Char, '{');
    forrange(row, 0, _Rows) {
        oss << (row > 0 ? STRING_LITERAL(_Char, ",[") : STRING_LITERAL(_Char, '['))
            << m.Get(row, 0);
        forrange(col, 1, _Cols)
            oss << STRING_LITERAL(_Char, ", ") << m.Get(row, col);
        oss << STRING_LITERAL(_Char, ']');
    }
    oss << STRING_LITERAL(_Char, '}');
    return oss;
}
//----------------------------------------------------------------------------
// hash_value
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Expr>
NODISCARD hash_t hash_value(const TScalarMatrixExpr<T, _Rows, _Cols, _Expr>& m) NOEXCEPT {
    return Meta::static_for<u32, _Rows, _Cols>([&](auto... it) NOEXCEPT -> hash_t {
        return hash_tuple(m.template Get<it.first, it.second>()...);
    });
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
// All scalar matrices are considered as pods
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TScalarMatrix<T COMMA _Rows COMMA _Cols>, typename T, u32 _Rows, u32 _Cols)
//----------------------------------------------------------------------------
// Matrix cast
//----------------------------------------------------------------------------
template <typename _Dst, typename T, u32 _Rows, u32 _Cols, typename _Expr>
NODISCARD inline CONSTEXPR TScalarMatrix<_Dst, _Rows, _Cols> bit_cast(const details::TScalarMatrixExpr<T, _Rows, _Cols, _Expr>& v) NOEXCEPT {
    return details::MakeScalarMatrixOp(v, [](const T& x) CONSTEXPR { return PPE::bit_cast<_Dst>(x); });
}
//----------------------------------------------------------------------------
template <typename _Dst, typename T, u32 _Rows, u32 _Cols, typename _Expr>
NODISCARD inline CONSTEXPR TScalarMatrix<_Dst, _Rows, _Cols> checked_cast(const details::TScalarMatrixExpr<T, _Rows, _Cols, _Expr>& v) NOEXCEPT {
    return details::MakeScalarMatrixOp(v, [](const T& x) CONSTEXPR { return PPE::checked_cast<_Dst>(x); });
}
//----------------------------------------------------------------------------
// Numeric limits
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Expr>
struct TNumericLimits< details::TScalarMatrixExpr<T, _Rows, _Cols, _Expr> > {
    typedef details::TScalarMatrixExpr<T, _Rows, _Cols, _Expr> value_type;
    typedef TNumericLimits<T> scalar_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, scalar_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  scalar_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  scalar_type::is_signed);

    NODISCARD inline static CONSTEXPR value_type DefaultValue() NOEXCEPT { return value_type( scalar_type::DefaultValue() ); }
    NODISCARD inline static CONSTEXPR value_type Epsilon() NOEXCEPT { return value_type( scalar_type::Epsilon() ); }
    NODISCARD inline static CONSTEXPR value_type MaxValue() NOEXCEPT { return value_type( scalar_type::MaxValue() ); }
    NODISCARD inline static CONSTEXPR value_type MinValue() NOEXCEPT { return value_type( scalar_type::MinValue() ); }
    NODISCARD inline static CONSTEXPR value_type LowestValue() NOEXCEPT { return value_type(scalar_type::LowestValue()); }
    NODISCARD inline static CONSTEXPR value_type Nan() NOEXCEPT { return value_type( scalar_type::Nan() ); }
    NODISCARD inline static CONSTEXPR value_type Zero() NOEXCEPT { return value_type( scalar_type::Zero() ); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#if defined(BUILD_LINK_DYNAMIC)
#   ifdef __clang__
#       define EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(T, W, H) template struct details::TScalarMatrixExpr<T, W, H, details::TScalarMatrixAssignable<T, W, H, details::TScalarMatrixStorage<T, W, H> >>
#       define EXTERN_RUNTIME_CORE_SCALARMATRIX_DEF(T, W, H)
#   else
#       define EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(T, W, H) EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) details::TScalarMatrixExpr<T, W, H, details::TScalarMatrixAssignable<T, W, H, details::TScalarMatrixStorage<T, W, H> >>
#       define EXTERN_RUNTIME_CORE_SCALARMATRIX_DEF(T, W, H)
#   endif
#else
#   define EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(T, W, H) EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) details::TScalarMatrixExpr<T, W, H, details::TScalarMatrixAssignable<T, W, H, details::TScalarMatrixStorage<T, W, H> >>
#   define EXTERN_RUNTIME_CORE_SCALARMATRIX_DEF(T, W, H) EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) details::TScalarMatrixExpr<T, W, H, details::TScalarMatrixAssignable<T, W, H, details::TScalarMatrixStorage<T, W, H> >>
#endif

#ifndef EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL
#   define EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(T, W, H) EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) details::TScalarMatrixExpr<T, W, H, details::TScalarMatrixAssignable<T, W, H, details::TScalarMatrixStorage<T, W, H> >>
#endif
#ifndef EXTERN_RUNTIME_CORE_SCALARMATRIX_DEF
#   define EXTERN_RUNTIME_CORE_SCALARMATRIX_DEF(T, W, H) EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) details::TScalarMatrixExpr<T, W, H, details::TScalarMatrixAssignable<T, W, H, details::TScalarMatrixStorage<T, W, H> >>
#endif

#ifndef EXPORT_PPE_RUNTIME_CORE_SCALARMATRIX
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(int, 2, 2);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(int, 3, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(int, 4, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(int, 3, 4);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(int, 4, 4);
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(unsigned int, 2, 2);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(unsigned int, 3, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(unsigned int, 4, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(unsigned int, 3, 4);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(unsigned int, 4, 4);
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(float, 2, 2);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(float, 3, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(float, 4, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(float, 3, 4);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(float, 4, 4);
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(double, 2, 2);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(double, 3, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(double, 4, 3);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(double, 3, 4);
EXTERN_RUNTIME_CORE_SCALARMATRIX_DECL(double, 4, 4);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!EXPORT_PPE_RUNTIME_CORE_SCALARMATRIX

PRAGMA_RESTORE_RUNTIMECHECKS
