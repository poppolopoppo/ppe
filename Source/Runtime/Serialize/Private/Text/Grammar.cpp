#include "stdafx.h"

#include "Text/Grammar.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/ParseExpression.h"
#include "Parser/ParseProduction.h"
#include "Parser/ParseStatement.h"
#include "Parser/StringExpansion.h"

#include "MetaObject.h"
#include "RTTI/AtomHeap.h"
#include "RTTI/Macros-impl.h"

#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Thread/ThreadContext.h"

#include "HAL/PlatformMaths.h"

// workaround a bug in Profiling : c4702 unreachable code - #TODO test after a few updates if it's still necessary
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4702)

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
typedef bool                FParseBool;
typedef i64                 FParseInteger;
typedef u64                 FParseUnsigned;
typedef double              FParseFloat;
typedef PPE::FString        FParseString; // TODO - FWString, change FLexer to wchar_t
typedef RTTI::FAtom         FParseAtom;
typedef RTTI::PMetaObject   FParseObject;
//----------------------------------------------------------------------------
enum EParseTypeId : RTTI::FTypeId {
    PARSEID_BOOL        = RTTI::NativeTypeId<FParseBool     >(),
    PARSEID_INTEGER     = RTTI::NativeTypeId<FParseInteger  >(),
    PARSEID_UNSIGNED    = RTTI::NativeTypeId<FParseUnsigned >(),
    PARSEID_FLOAT       = RTTI::NativeTypeId<FParseFloat    >(),
    PARSEID_STRING      = RTTI::NativeTypeId<FParseString   >(),
    PARSEID_OBJECT      = RTTI::NativeTypeId<FParseObject   >()
};
//----------------------------------------------------------------------------
#define DEF_UNARYOPERATOR_FUNCTOR(_Name, _Op) template <typename T> \
    struct CONCAT(TUnOp_, _Name) { \
        T operator ()(const Parser::FParseExpression *, const T& value) const { \
            return (_Op value); \
        } \
    }

DEF_UNARYOPERATOR_FUNCTOR(Add,  +);
DEF_UNARYOPERATOR_FUNCTOR(Sub,  -);
DEF_UNARYOPERATOR_FUNCTOR(Not,  !);
DEF_UNARYOPERATOR_FUNCTOR(Cpl,  ~);
//----------------------------------------------------------------------------
#define FORBID_UNARYOPERATOR_FUNCTOR(_Name, _Op, _Type) template <> \
    struct CONCAT(TUnOp_, _Name)< _Type > { \
        _Type operator ()(const Parser::FParseExpression *expr, const _Type& ) const { \
            PPE_THROW_IT(Parser::FParserException("unary operator " STRINGIZE(_Op) " is not available for <" STRINGIZE(_Type) ">", expr->Site(), expr)); \
        } \
    }

FORBID_UNARYOPERATOR_FUNCTOR(Add, +, FParseBool);
FORBID_UNARYOPERATOR_FUNCTOR(Sub, -, FParseBool);
FORBID_UNARYOPERATOR_FUNCTOR(Cpl, ~, FParseBool);

FORBID_UNARYOPERATOR_FUNCTOR(Sub, -, FParseUnsigned);

FORBID_UNARYOPERATOR_FUNCTOR(Not, !, FParseFloat);
FORBID_UNARYOPERATOR_FUNCTOR(Cpl, ~, FParseFloat);

FORBID_UNARYOPERATOR_FUNCTOR(Add, +, FParseString);
FORBID_UNARYOPERATOR_FUNCTOR(Sub, -, FParseString);
FORBID_UNARYOPERATOR_FUNCTOR(Not, !, FParseString);
FORBID_UNARYOPERATOR_FUNCTOR(Cpl, ~, FParseString);
//----------------------------------------------------------------------------
#define DEF_BINARYOPERATOR_FUNCTOR(_Name, _Op) template <typename T> \
    struct CONCAT(TBinOp_, _Name) { \
        T operator ()(const Parser::FParseExpression *, const T& lhs, const T& rhs) const { \
            return (lhs _Op rhs); \
        } \
    }

DEF_BINARYOPERATOR_FUNCTOR(Mul,   *);
DEF_BINARYOPERATOR_FUNCTOR(Div,   /);
DEF_BINARYOPERATOR_FUNCTOR(Mod,   %);

DEF_BINARYOPERATOR_FUNCTOR(Add,   +);
DEF_BINARYOPERATOR_FUNCTOR(Sub,   -);

DEF_BINARYOPERATOR_FUNCTOR(Lsh,  <<);
DEF_BINARYOPERATOR_FUNCTOR(Rsh,  >>);

DEF_BINARYOPERATOR_FUNCTOR(And,   &);
DEF_BINARYOPERATOR_FUNCTOR(Or,    |);
DEF_BINARYOPERATOR_FUNCTOR(Xor,   ^);
//----------------------------------------------------------------------------
template <typename T>
struct TBinOp_Pow {
    T operator ()(const Parser::FParseExpression *, const T& lhs, const T& rhs) const {
        return T(std::pow(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
#define FORBID_BINARYOPERATOR_FUNCTOR(_Name, _Op, _Type) template <> \
    struct CONCAT(TBinOp_, _Name)< _Type > { \
        _Type operator ()(const Parser::FParseExpression *expr, const _Type& , const _Type& ) const { \
            PPE_THROW_IT(Parser::FParserException("binary operator " STRINGIZE(_Op) " is not available for <" STRINGIZE(_Type) ">", expr->Site(), expr)); \
        } \
    }

FORBID_BINARYOPERATOR_FUNCTOR(Sub,  -, FParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Div,  /, FParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Mod,  %, FParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Lsh, <<, FParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Rsh, >>, FParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Pow, **, FParseBool);

FORBID_BINARYOPERATOR_FUNCTOR(And,  &, FParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Or,   |, FParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Xor,  ^, FParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Lsh, <<, FParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Rsh, >>, FParseFloat);

FORBID_BINARYOPERATOR_FUNCTOR(Sub,  -, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Mul,  *, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Div,  /, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Mod,  %, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Pow, **, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(And,  &, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Or,   |, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Xor,  ^, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Lsh, <<, FParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Rsh, >>, FParseString);
//----------------------------------------------------------------------------
template <>
struct TBinOp_Add<FParseBool> {
    FParseBool operator ()(const Parser::FParseExpression *, FParseBool lhs, FParseBool rhs) const {
        return lhs || rhs;
    }
};
//----------------------------------------------------------------------------
template <>
struct TBinOp_Mul<FParseBool> {
    FParseBool operator ()(const Parser::FParseExpression *, FParseBool lhs, FParseBool rhs) const {
        return lhs && rhs;
    }
};
//----------------------------------------------------------------------------
template <>
struct TBinOp_Mod<FParseFloat> {
    FParseFloat operator ()(const Parser::FParseExpression *, FParseFloat lhs, FParseFloat rhs) const {
        return std::fmod(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
#define DEF_BINARYOPERATOR_COMPARATOR(_Name, _Op) template <typename T> \
    struct CONCAT(FCmpOp_, _Name) { \
        bool operator ()(const Parser::FParseExpression *, const T& lhs, const T& rhs) const { \
            return (lhs _Op rhs); \
        } \
    }

DEF_BINARYOPERATOR_COMPARATOR(Less, <);
DEF_BINARYOPERATOR_COMPARATOR(LessOrEqual, <=);

DEF_BINARYOPERATOR_COMPARATOR(Greater, >);
DEF_BINARYOPERATOR_COMPARATOR(GreaterOrEqual, >=);

DEF_BINARYOPERATOR_COMPARATOR(Equals, ==);
DEF_BINARYOPERATOR_COMPARATOR(NotEquals, !=);
//----------------------------------------------------------------------------
template <typename T, template <typename > class _Op>
static bool TryAssignCopy_(Parser::FParseContext* context, const RTTI::FAtom& value, const Parser::FParseExpression *expr, RTTI::FAtom& result) {
    Assert(!result);

    T tmp;
    if (value.PromoteCopy(RTTI::MakeAtom(&tmp))) {
        result = context->CreateAtomFrom( _Op<T>()(expr, tmp) );
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <template <typename > class _Op>
struct TUnaryOp {
    RTTI::FAtom operator ()(Parser::FParseContext* context, const Parser::FParseExpression *expr) const {
        const RTTI::FAtom value = expr->Eval(context);
        Assert(value);

        switch (value.TypeId()) {
        case PARSEID_BOOL:
            return context->CreateAtomFrom( _Op< FParseBool     >()(expr, value.FlatData<FParseBool       >()) );
        case PARSEID_INTEGER:
            return context->CreateAtomFrom( _Op< FParseInteger  >()(expr, value.FlatData<FParseInteger    >()) );
        case PARSEID_UNSIGNED:
            return context->CreateAtomFrom( _Op< FParseUnsigned >()(expr, value.FlatData<FParseUnsigned   >()) );
        case PARSEID_FLOAT:
            return context->CreateAtomFrom( _Op< FParseFloat    >()(expr, value.FlatData<FParseFloat      >()) );
        case PARSEID_STRING:
            return context->CreateAtomFrom( _Op< FParseString   >()(expr, value.FlatData<FParseString     >()) );

        default:
            // now try to cast, i.e not the current type but accessible through meta cast
            {
                RTTI::FAtom result;
                if (TryAssignCopy_<FParseString, _Op>(context, value, expr, result) ||
                    TryAssignCopy_<FParseFloat, _Op>(context, value, expr, result) ||
                    TryAssignCopy_<FParseInteger    , _Op>(context, value, expr, result) ||
                    TryAssignCopy_<FParseUnsigned, _Op>(context, value, expr, result) ||
                    TryAssignCopy_<FParseBool, _Op>(context, value, expr, result) ) {
                    Assert(result);
                    return result;
                }
            }
        }

        PPE_THROW_IT(Parser::FParserException("invalid atom type", expr));
    }
};
//----------------------------------------------------------------------------
template <template <typename > class _Op>
struct TBinaryOp {
    static RTTI::FAtom BooleanOp_(
        Parser::FParseContext* context,
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FAtom& lhs_value,
        const RTTI::FAtom& rhs_value) {
        const RTTI::FTypeId rhs_type_id = rhs_value.TypeId();

        if (rhs_type_id == PARSEID_BOOL) {
            return context->CreateAtomFrom( _Op< FParseBool >()(
                lhs,
                lhs_value.FlatData<FParseBool>(),
                rhs_value.FlatData<FParseBool>() ));
        }
        else {
            FParseBool b{ false };

            if (rhs_type_id == PARSEID_INTEGER)
                b = (FParseInteger(0) != rhs_value.FlatData<FParseInteger>());
            else if (rhs_type_id == PARSEID_UNSIGNED)
                b = (FParseUnsigned(0) != rhs_value.FlatData<FParseUnsigned>());
            else if (rhs_type_id == PARSEID_FLOAT)
                b = (FParseFloat(0) != rhs_value.FlatData<FParseFloat>());
            else {

                if (is_boolean_v(rhs_value)) {
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&b)))
                        NOOP(); // b was already assigned
                }
                else if (is_signed_integral_v(rhs_value)) {
                    FParseInteger i;
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&i)))
                        b = (FParseInteger(0) != i);
                }
                else if (is_unsigned_integral_v(rhs_value)) {
                    FParseUnsigned u;
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&u)))
                        b = (FParseUnsigned(0) != u);
                }
                else if (is_floating_point_v(rhs_value)) {
                    FParseFloat fp;
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                        b = (FParseFloat(0) != fp);
                }
                else if (is_string_v(rhs_value)) {
                    FParseString s;
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&s)))
                        b = (not s.empty());
                }
                else {
                    PPE_THROW_IT(Parser::FParserException("could not convert to boolean", rhs));
                }
            }

            return context->CreateAtomFrom( _Op< FParseBool >()(
                lhs,
                lhs_value.FlatData<FParseBool>(),
                b ));
        }
    }

    static RTTI::FAtom IntegerOp_(
        Parser::FParseContext* context,
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FAtom& lhs_value,
        const RTTI::FAtom& rhs_value) {
        const RTTI::FTypeId rhs_type_id = rhs_value.TypeId();

        FParseInteger integer{ 0 };

        if (rhs_type_id == PARSEID_BOOL)
            return context->CreateAtomFrom(
                _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseInteger>(), rhs_value.FlatData<FParseBool>() ? FParseInteger(1) : FParseInteger(0)) );
        else if (rhs_type_id == PARSEID_INTEGER)
            return context->CreateAtomFrom(
                _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseInteger>(), rhs_value.FlatData<FParseInteger>()) );
        else if (rhs_type_id == PARSEID_UNSIGNED)
            return context->CreateAtomFrom(
                _Op< FParseUnsigned >()(lhs, lhs_value.FlatData<FParseInteger>(), rhs_value.FlatData<FParseUnsigned>()));
        else if (rhs_type_id == PARSEID_FLOAT)
            return context->CreateAtomFrom(
                _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value.FlatData<FParseInteger>()), rhs_value.FlatData<FParseFloat>()) );
        else {

            if (is_signed_integral_v(rhs_value)) {
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&integer)))
                    return context->CreateAtomFrom(
                        _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseInteger>(), integer));
            }
            else if (is_unsigned_integral_v(rhs_value)) {
                FParseUnsigned u;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&u)))
                    return context->CreateAtomFrom(
                        _Op< FParseUnsigned >()(lhs, lhs_value.FlatData<FParseInteger>(), u));
            }
            else if (is_floating_point_v(rhs_value)) {
                FParseFloat fp;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                    return context->CreateAtomFrom(
                        _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value.FlatData<FParseInteger>()), fp));
            }
            else if (is_boolean_v(rhs_value)) {
                FParseBool b;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&b)))
                    return context->CreateAtomFrom(
                        _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseInteger>(), b ? FParseInteger(1) : FParseInteger(0)));
            }

            PPE_THROW_IT(Parser::FParserException("could not convert to integer", rhs));
        }
    }

    static RTTI::FAtom UnsignedOp_(
        Parser::FParseContext* context,
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FAtom& lhs_value,
        const RTTI::FAtom& rhs_value) {
        const RTTI::FTypeId rhs_type_id = rhs_value.TypeId();

        FParseUnsigned unsign_d{ 0 };

        if (rhs_type_id == PARSEID_BOOL)
            return context->CreateAtomFrom(
                _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseUnsigned>(), rhs_value.FlatData<FParseBool>() ? FParseUnsigned(1) : FParseUnsigned(0)));
        else if (rhs_type_id == PARSEID_INTEGER)
            return context->CreateAtomFrom(
                _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseUnsigned>(), rhs_value.FlatData<FParseInteger>()));
        else if (rhs_type_id == PARSEID_UNSIGNED)
            return context->CreateAtomFrom(
                _Op< FParseUnsigned >()(lhs, lhs_value.FlatData<FParseUnsigned>(), rhs_value.FlatData<FParseUnsigned>()));
        else if (rhs_type_id == PARSEID_FLOAT)
            return context->CreateAtomFrom(
                _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value.FlatData<FParseUnsigned>()), rhs_value.FlatData<FParseFloat>()));
        else {

            if (is_unsigned_integral_v(rhs_value)) {
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&unsign_d)))
                    return context->CreateAtomFrom(
                        _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseUnsigned>(), unsign_d));
            }
            else if (is_unsigned_integral_v(rhs_value)) {
                FParseInteger i;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&i)))
                    return context->CreateAtomFrom(
                        _Op< FParseUnsigned >()(lhs, lhs_value.FlatData<FParseUnsigned>(), i));
            }
            else if (is_floating_point_v(rhs_value)) {
                FParseFloat fp;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                    return context->CreateAtomFrom(
                        _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value.FlatData<FParseUnsigned>()), fp));
            }
            else if (is_boolean_v(rhs_value)) {
                FParseBool b;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&b)))
                    return context->CreateAtomFrom(
                        _Op< FParseUnsigned >()(lhs, lhs_value.FlatData<FParseUnsigned>(), b ? FParseUnsigned(1) : FParseUnsigned(0)));
            }

            PPE_THROW_IT(Parser::FParserException("could not convert to integer", rhs));
        }
    }

    static RTTI::FAtom FloatOp_(
        Parser::FParseContext* context,
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FAtom& lhs_value,
        const RTTI::FAtom& rhs_value) {
        const RTTI::FTypeId rhs_type_id = rhs_value.TypeId();

        FParseFloat f{ 0 };

        if (rhs_type_id == PARSEID_FLOAT)
            f = rhs_value.FlatData<FParseFloat>();
        else if (rhs_type_id == PARSEID_INTEGER)
            f = static_cast<FParseFloat>(rhs_value.FlatData<FParseInteger>());
        else if (rhs_type_id == PARSEID_UNSIGNED)
            f = static_cast<FParseFloat>(rhs_value.FlatData<FParseUnsigned>());
        else if (rhs_type_id == PARSEID_BOOL)
            f = rhs_value.FlatData<FParseBool>() ? FParseFloat(1) : FParseFloat(0);
        else {

            if (is_floating_point_v(rhs_value)) {
                FParseFloat fp;
                Verify(rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)));
                f = fp;
            }
            else if (is_signed_integral_v(rhs_value)) {
                FParseInteger i;
                Verify(rhs_value.PromoteCopy(RTTI::MakeAtom(&i)));
                f = static_cast<FParseFloat>(i);
            }
            else if (is_unsigned_integral_v(rhs_value)) {
                FParseUnsigned u;
                Verify(rhs_value.PromoteCopy(RTTI::MakeAtom(&u)));
                f = static_cast<FParseFloat>(u);
            }
            else {
                PPE_THROW_IT(Parser::FParserException("could not convert to float", rhs));
            }
        }

        return context->CreateAtomFrom(
            _Op< FParseFloat >()(lhs, lhs_value.FlatData<FParseFloat>(), f)
            );
    }

    static RTTI::FAtom StringOp_(
        Parser::FParseContext* context,
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FAtom& lhs_value,
        const RTTI::FAtom& rhs_value) {
        const RTTI::FTypeId rhs_type_id = rhs_value.TypeId();

        if (rhs_type_id == PARSEID_STRING)
            return context->CreateAtomFrom(
                _Op< FParseString >()(lhs, lhs_value.FlatData<FParseString>(), rhs_value.FlatData<FParseString>())
                );

        FStringBuilder oss;

        if (rhs_type_id == PARSEID_BOOL)
            oss << rhs_value.FlatData<FParseBool>();
        else if (rhs_type_id == PARSEID_INTEGER)
            oss << rhs_value.FlatData<FParseInteger>();
        else if (rhs_type_id == PARSEID_UNSIGNED)
            oss << rhs_value.FlatData<FParseUnsigned>();
        else if (rhs_type_id == PARSEID_FLOAT)
            oss << rhs_value.FlatData<FParseFloat>();
        else {
            FParseString s;
            FParseInteger i;
            FParseUnsigned u;
            FParseFloat fp;
            FParseBool b;

            if (is_string_v(rhs_value) && rhs_value.PromoteCopy(RTTI::MakeAtom(&s)))
                oss << s;
            else if (is_signed_integral_v(rhs_value) && rhs_value.PromoteCopy(RTTI::MakeAtom(&i)))
                oss << i;
            else if (is_unsigned_integral_v(rhs_value) && rhs_value.PromoteCopy(RTTI::MakeAtom(&u)))
                oss << u;
            else if (is_floating_point_v(rhs_value) && rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                oss << fp;
            else if (is_boolean_v(rhs_value) && rhs_value.PromoteCopy(RTTI::MakeAtom(&b)))
                oss << b;
            else
                PPE_THROW_IT(Parser::FParserException("could not convert to string", rhs));
        }

        return context->CreateAtomFrom(
            _Op< FParseString >()(lhs, lhs_value.FlatData<FParseString>(), oss.ToString())
            );
    }

    RTTI::FAtom operator ()(Parser::FParseContext* context, const Parser::FParseExpression *lhs, const Parser::FParseExpression *rhs) const {
        const RTTI::FAtom lhs_value = lhs->Eval(context);
        const RTTI::FAtom rhs_value = rhs->Eval(context);

        Assert(lhs_value);
        Assert(rhs_value);

        switch (lhs_value.TypeId()) {
        case PARSEID_BOOL:
            return BooleanOp_(context, lhs, rhs, lhs_value, rhs_value);
        case PARSEID_INTEGER:
            return IntegerOp_(context, lhs, rhs, lhs_value, rhs_value);
        case PARSEID_UNSIGNED:
            return UnsignedOp_(context, lhs, rhs, lhs_value, rhs_value);
        case PARSEID_FLOAT:
            return FloatOp_(context, lhs, rhs, lhs_value, rhs_value);
        case PARSEID_STRING:
            return StringOp_(context, lhs, rhs, lhs_value, rhs_value);

        default:
            // now try to cast, i.e not the current type but accessible through copy promotion
            if (is_signed_integral_v(lhs_value)) {
                FParseInteger i;
                RTTI::FAtom lhs_integer(RTTI::MakeAtom(&i));
                if (lhs_value.PromoteCopy(lhs_integer))
                    return IntegerOp_(context, lhs, rhs, lhs_integer, rhs_value);
            }
            else if (is_unsigned_integral_v(lhs_value)) {
                FParseUnsigned u;
                RTTI::FAtom lhs_unsigned(RTTI::MakeAtom(&u));
                if (lhs_value.PromoteCopy(lhs_unsigned))
                    return UnsignedOp_(context, lhs, rhs, lhs_unsigned, rhs_value);
            }
            else if (is_floating_point_v(lhs_value)) {
                FParseFloat fp;
                RTTI::FAtom lhs_fp(RTTI::MakeAtom(&fp));
                if (lhs_value.PromoteCopy(lhs_fp))
                    return FloatOp_(context, lhs, rhs, lhs_fp, rhs_value);
            }
            else if (is_boolean_v(lhs_value)) {
                FParseBool b;
                RTTI::FAtom lhs_bool(RTTI::MakeAtom(&b));
                if (lhs_value.PromoteCopy(lhs_bool))
                    return BooleanOp_(context, lhs, rhs, lhs_bool, rhs_value);
            }
            else if (is_string_v(lhs_value)) {
                FParseString str;
                RTTI::FAtom lhs_str(RTTI::MakeAtom(&str));
                if (lhs_value.PromoteCopy(lhs_str))
                    return StringOp_(context, lhs, rhs, lhs_str, rhs_value);
            }
        }

        PPE_THROW_IT(Parser::FParserException("invalid atom type for binary operator", lhs));
    }
};
//----------------------------------------------------------------------------
// Special behavior for <String> % <Any> which can be used for string interpolation
template <>
RTTI::FAtom TBinaryOp<TBinOp_Mod>::StringOp_(
    Parser::FParseContext* context,
    const Parser::FParseExpression* lhs,
    const Parser::FParseExpression*,
    const RTTI::FAtom& lhs_value, const RTTI::FAtom& rhs_value) {

    FString formated;

    const RTTI::ETypeFlags typeFlags = rhs_value.TypeFlags();
    if (typeFlags ^ RTTI::ETypeFlags::Tuple)
        formated = PerformStringExpansion(lhs_value.FlatData<FString>(), rhs_value, rhs_value.Traits()->ToTuple(), lhs->Site());
    else if (typeFlags ^ RTTI::ETypeFlags::List)
        formated = PerformStringExpansion(lhs_value.FlatData<FString>(), rhs_value, rhs_value.Traits()->ToList(), lhs->Site());
#if 0 // #TODO : named format instead of indexes when used against a dico
    else if (typeFlags ^ RTTI::ETypeFlags::Dico)
        formated = PerformStringExpansion(lhs_value.FlatData<FString>(), rhs_value, rhs_value.Traits()->ToDico(), lhs->Site());
#endif
    else {
        Assert_NoAssume(typeFlags ^ RTTI::ETypeFlags::Scalar);
        formated = PerformStringExpansion(lhs_value.FlatData<FString>(), rhs_value, rhs_value.Traits()->ToScalar(), lhs->Site());
    }

    return context->CreateAtomFrom(std::move(formated));
}
//----------------------------------------------------------------------------
struct FTernaryOp {
    bool operator ()(Parser::FParseContext* context, const Parser::FParseExpression *expr) const {
        const RTTI::FAtom value = expr->Eval(context);

        Assert(value);
        FParseBool b = false;

        switch (value.TypeId()) {
        case PARSEID_BOOL:
            return (value.FlatData<FParseBool>() != 0);
        case PARSEID_INTEGER:
            return (value.FlatData<FParseInteger>() != 0);
        case PARSEID_UNSIGNED:
            return (value.FlatData<FParseUnsigned>() != 0);
        case PARSEID_FLOAT:
            return (value.FlatData<FParseFloat>() != 0);
        case PARSEID_STRING:
            return (value.FlatData<FParseString>().empty() != false);

        default:
            if (value.PromoteCopy(RTTI::MakeAtom(&b)))
                return b;
        }

        PPE_THROW_IT(Parser::FParserException("invalid atom type for ternary operator", expr));
    }
};
//----------------------------------------------------------------------------
static Parser::TProduction<RTTI::PTypeTraits> ExpectTypenameRTTI() {
    return Parser::TProduction<RTTI::PTypeTraits>{
        [](Parser::FParseList& input, RTTI::PTypeTraits* value) -> Parser::FParseResult {
            const Parser::FParseMatch *match = input.Read();

            if (match) {
                if (match->Symbol()->Type() == Lexer::FSymbol::Typename)
                    *value = RTTI::MakeTraits(RTTI::ENativeType(match->Symbol()->Ord()));
                else if (match->Symbol()->Type() == Lexer::FSymbol::Identifier)
                    *value = RTTI::MakeTraitsFromTypename(match->Value());

                if (*value)
                    return Parser::FParseResult::Success(match->Site());
            }

            return Parser::FParseResult::Unexpected(Lexer::FSymbol::Typename, match, input);
        }};
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGrammarImpl {
public:

#define USE_GRAMMAR_WORKAROUND_VS2019 1

    FGrammarImpl() NOEXCEPT;
    ~FGrammarImpl();

    Parser::PCParseExpression ParseExpression(Parser::FParseList& input) const;
    Parser::PCParseStatement ParseStatement(Parser::FParseList& input) const;

private:
    using site_t = Lexer::FSpan;
    using symbol_t = Lexer::FSymbol;

    using expr_t = Parser::PCParseExpression;
    using statement_t = Parser::PCParseStatement;

    using many_expr_t = Parser::TEnumerable<expr_t>;
    using many_statement_t = Parser::TEnumerable<statement_t>;

    using match_p = const Parser::FParseMatch*;
    using parse_t = Parser::FParseResult;

    template <typename T>
    using TEnumerable = Parser::TEnumerable<T>;

    const Parser::TProduction< expr_t >& _lvalue; // only an alias
    Parser::TProduction< expr_t > _export;
    Parser::TProduction< expr_t > _expr;
    Parser::TProduction< many_expr_t > _exprSeq;
    Parser::TProduction< statement_t > _statement;

    Parser::TProduction< expr_t > _literal;

    Parser::TProduction< statement_t > _property;
#if !USE_GRAMMAR_WORKAROUND_VS2019
    Parser::TProduction< many_statement_t > _objectInner;
#endif
    Parser::TProduction< expr_t > _object;

    Parser::TProduction< expr_t > _tuple;
    Parser::TProduction< expr_t > _array;

    Parser::TProduction< TTuple<expr_t, match_p, expr_t> > _dictionaryItem;
    Parser::TProduction< TEnumerable<TTuple<expr_t, match_p, expr_t>> > _dictionaryInner;
    Parser::TProduction< expr_t > _dictionary;

    Parser::TProduction< expr_t > _variable;
    Parser::TProduction< expr_t > _reference;
    Parser::TProduction< expr_t > _accessor;
    Parser::TProduction< expr_t > _rvalue;

    Parser::TProduction< expr_t > _pow;
    Parser::TProduction< expr_t > _unary;
    Parser::TProduction< expr_t > _mulDivMod;
    Parser::TProduction< expr_t > _addSub;
    Parser::TProduction< expr_t > _lshRsh;
    Parser::TProduction< expr_t > _compare;
    Parser::TProduction< expr_t > _equalsNotEquals;
    Parser::TProduction< expr_t > _and;
    Parser::TProduction< expr_t > _xor;
    Parser::TProduction< expr_t > _or;
    Parser::TProduction< expr_t > _ternary;

};
//----------------------------------------------------------------------------
FGrammarImpl::FGrammarImpl() NOEXCEPT
:   _lvalue(_ternary)

#if !USE_GRAMMAR_WORKAROUND_VS2019
,   _export(Parser::And(
        Parser::Optional<symbol_t::Export>(),
        Parser::Sequence<symbol_t::Identifier, symbol_t::Is>(),
        _lvalue.Ref() )
    .Select<expr_t>([](expr_t* dst, const site_t& site, const TTuple<match_p, TTuple<match_p, match_p>, expr_t>&& src) {
        match_p const exportIFP = std::get<0>(src);
        match_p const name = std::get<0>(std::get<1>(src));
        const expr_t& value = std::get<2>(src);

        const Parser::FVariableExport::EFlags scope = (exportIFP)
            ? Parser::FVariableExport::Global
            : Parser::FVariableExport::Public;

        *dst = Parser::MakeVariableExport(RTTI::FName(name->Value()), value, scope, site);
    }))
#else
,   _export([this](Parser::FParseList& input, expr_t* dst) -> Parser::FParseResult {
        match_p export_ = nullptr, id_ = nullptr, is_ = nullptr;
        auto scope = Parser::FVariableExport::Public;

        if (input.TryRead<symbol_t::Export>(&export_))
            scope = Parser::FVariableExport::Global;
        if (not input.TryRead<symbol_t::Identifier>(&id_))
            return Parser::FParseResult::Unexpected(symbol_t::Identifier, id_, input);
        if (not input.TryRead<symbol_t::Is>(&is_))
            return Parser::FParseResult::Unexpected(symbol_t::Is, is_, input);

        expr_t value;
        const Parser::FParseResult result = _lvalue(input, &value);
        if (not result)
            return result;

        const site_t site{ Lexer::FSpan::FromSite(export_ ? export_->Site() : id_->Site(), value->Site()) };
        *dst = Parser::MakeVariableExport(RTTI::FName(id_->Value()), value, scope, site);

        return Parser::FParseResult::Success(site);
    })
#endif

,   _expr(Parser::Switch(
        _export,
        _lvalue
    ))

,   _exprSeq(_expr.Join(Parser::Expect<symbol_t::Comma>()))

,   _statement(_expr.Select<statement_t>([](statement_t* dst, const site_t& site, expr_t&& expr){
        UNUSED(site);
        *dst = Parser::MakeEvalExpr(expr);
    }))

#if !USE_GRAMMAR_WORKAROUND_VS2019
,   _literal(Parser::Or(
        Parser::Expect<symbol_t::Null, expr_t>([](match_p src) -> expr_t {
            return Parser::MakeLiteral(FParseObject(), src->Site());
        }),
        Parser::Expect<symbol_t::True, expr_t>([](match_p src) -> expr_t {
            return Parser::MakeLiteral(true, src->Site());
        }),
        Parser::Expect<symbol_t::False, expr_t>([](match_p src) -> expr_t {
            return Parser::MakeLiteral(false, src->Site());
        }),
        Parser::Expect<symbol_t::Integer, expr_t>([](match_p src ) -> expr_t {
            i64 i;
            Verify(Atoi(&i, src->Value(), 10));
            return Parser::MakeLiteral(i, src->Site());
        }),
        Parser::Expect<symbol_t::Unsigned, expr_t>([](match_p src) -> expr_t {
            u64 u;
            Verify(Atoi(&u, src->Value(), 10));
            return Parser::MakeLiteral(u, src->Site());
        }),
        Parser::Expect<symbol_t::Float, expr_t>([](match_p src) -> expr_t {
            double d;
            Verify(Atod(&d, src->Value()));
            return Parser::MakeLiteral(d, src->Site());
        }),
        Parser::Expect<symbol_t::String, expr_t>([](match_p src)  -> expr_t {
            return Parser::MakeLiteral(FString(src->Value()), src->Site());
        })
    ))
#else
,   _literal([](Parser::FParseList& input, expr_t* dst) -> Parser::FParseResult {
        match_p m = nullptr;

        if (input.TryRead<symbol_t::Null>(&m))
            *dst = Parser::MakeLiteral(FParseObject(), m->Site());
        else if (input.TryRead<symbol_t::True>(&m))
            *dst = Parser::MakeLiteral(true, m->Site());
        else if (input.TryRead<symbol_t::False>(&m))
            *dst = Parser::MakeLiteral(false, m->Site());
        else if (input.TryRead<symbol_t::String>(&m))
            *dst = Parser::MakeLiteral(FString(m->Value()), m->Site());
        else if (input.TryRead<symbol_t::Integer>(&m)) {
            i64 i;
            Verify(Atoi(&i, m->Value(), 10));
            *dst = Parser::MakeLiteral(i, m->Site());
        }
        else if (input.TryRead<symbol_t::Unsigned>(&m)) {
            u64 u;
            Verify(Atoi(&u, m->Value(), 10));
            *dst = Parser::MakeLiteral(u, m->Site());
        }
        else if (input.TryRead<symbol_t::Float>(&m)) {
            double d;
            Verify(Atod(&d, m->Value()));
            *dst = Parser::MakeLiteral(d, m->Site());
        }
        else
            return Parser::FParseResult::Failure(input.Site());

        Assert(*dst);
        return Parser::FParseResult::Success(m->Site());
    })
#endif

#if !USE_GRAMMAR_WORKAROUND_VS2019
,   _property(Parser::And(
        Parser::Sequence<symbol_t::Identifier, symbol_t::Assignment>(),
        _expr.Ref() )
    .Select<statement_t>([](statement_t* dst, const site_t& site, TTuple<TTuple<match_p, match_p>, expr_t>&& src) {
        match_p name = std::get<0>(std::get<0>(src));
        expr_t& value = std::get<1>(src);

        *dst = Parser::MakePropertyAssignment(RTTI::FName(name->Value()), value, site);
    }))
#else
,   _property([this](Parser::FParseList& input, statement_t* dst) -> Parser::FParseResult {
        match_p id_ = nullptr, assign_ = nullptr;
        if (not input.Expect<symbol_t::Identifier>(&id_))
            return Parser::FParseResult::Unexpected(symbol_t::Identifier, id_, input);
        if (not input.Expect<symbol_t::Assignment>(&assign_))
            return Parser::FParseResult::Unexpected(symbol_t::Assignment, assign_, input);

        expr_t value;
        const Parser::FParseResult result = _expr(input, &value);
        if (not result)
            return result;

        const site_t site{ Lexer::FSpan::FromSite(id_->Site(), value->Site()) };
        *dst = Parser::MakePropertyAssignment(RTTI::FName(id_->Value()), value, site);

        return Parser::FParseResult::Success(site);
    })
#endif

#if !USE_GRAMMAR_WORKAROUND_VS2019
,   _objectInner(_property.Many())
,   _object(Parser::And(
        Parser::Expect<symbol_t::Identifier>(),
        Parser::Closure<
            symbol_t::LBrace,
            symbol_t::RBrace >(_objectInner) )
    .Select<expr_t>([](
        expr_t* dst,
        const site_t& site,
        TTuple<match_p, many_statement_t>&& src) {
        match_p name = std::get<0>(src);
        many_statement_t& statements = std::get<1>(src);
        *dst = Parser::MakeObjectDefinition(
            RTTI::FName(name->Value()), site,
            MakeMoveIterator(statements.begin()),
            MakeMoveIterator(statements.end()) );
    }))
#else
,   _object([this](Parser::FParseList& input, expr_t* dst) -> Parser::FParseResult {
        match_p class_ = nullptr, brace_ = nullptr;
        if (not input.Expect<symbol_t::Identifier>(&class_))
            return Parser::FParseResult::Unexpected(symbol_t::Identifier, class_, input);
        if (not input.Expect<symbol_t::LBrace>(&brace_))
            return Parser::FParseResult::Unexpected(symbol_t::LBrace, brace_, input);

        many_statement_t inner;
        const Parser::FParseResult result = _property.Many()(input, &inner);
        if (not result)
            return result;

        if (not input.Expect<symbol_t::RBrace>(&brace_))
            return Parser::FParseResult::Unexpected(symbol_t::RBrace, brace_, input);

        const site_t site{ Lexer::FSpan::FromSite(class_->Site(), brace_->Site()) };
        *dst = Parser::MakeObjectDefinition(
            RTTI::FName(class_->Value()), site,
            MakeMoveIterator(inner.begin()),
            MakeMoveIterator(inner.end()) );

        return Parser::FParseResult::Success(site);
    })
#endif

,   _tuple(Parser::Closure<
        symbol_t::LParenthese,
        symbol_t::RParenthese >(_exprSeq)
    .Select<expr_t>([](expr_t* dst, const site_t& site, many_expr_t&& src) {
        Assert(not src.empty());
        *dst = (src.size() > 1
            ? expr_t{ Parser::MakeTupleExpr(src.MakeView(), site) }
            : src.front() /* tuple of 1 elt aren't wrapped inside a tuple expr, act as a parenthesized expr */ );
    }))

,   _array(Parser::Closure<
        symbol_t::LBracket,
        symbol_t::RBracket >(_exprSeq)
    .Select<expr_t>([](expr_t* dst, const site_t& site, many_expr_t&& src) {
        *dst = Parser::MakeArrayExpr(src.MakeView(), site);
    }))

#if !USE_GRAMMAR_WORKAROUND_VS2019
,   _dictionaryItem(Parser::And(
        _expr.Ref(),
        Parser::Expect<symbol_t::Comma>(),
        _expr.Ref() ))
#else
,   _dictionaryItem([this](Parser::FParseList& input, TTuple<expr_t, match_p, expr_t>* dst) -> Parser::FParseResult {
        expr_t key;
        Parser::FParseResult result = _expr(input, &key);
        if (not result)
            return result;

        match_p comma = nullptr;
        if (not input.Expect<symbol_t::Comma>(&comma))
            return Parser::FParseResult::Unexpected(symbol_t::Comma, comma, input);

        expr_t value;
        result = _expr(input, &value);
        if (not result)
            return result;

        const site_t site{ Lexer::FSpan::FromSpan(key->Site(), value->Site()) };
        *dst = std::make_tuple(std::move(key), comma, std::move(value));

        return Parser::FParseResult::Success(site);
    })
#endif
,   _dictionaryInner(Parser::Closure<
        symbol_t::LParenthese,
        symbol_t::RParenthese >(_dictionaryItem)
    .Join(Parser::Expect<symbol_t::Comma>() ))
,   _dictionary(Parser::Closure<
        symbol_t::LBrace,
        symbol_t::RBrace >(_dictionaryInner)
    .Select<expr_t>([](expr_t* dst, const site_t& site, TEnumerable<TTuple<expr_t, match_p, expr_t>>&& src) {
        Parser::FDictionaryExpr::dico_type dico;
        dico.reserve(src.size());

        for (auto& it : src)
            dico.Add(std::move(std::get<0>(it))) = std::move(std::get<2>(it));

        *dst = Parser::MakeDictionaryExpr(std::move(dico), site);
    }))

#if !USE_GRAMMAR_WORKAROUND_VS2019
,   _variable(Parser::Or(
        Parser::Expect<symbol_t::Identifier>()
            .Select<expr_t>([](expr_t* dst, const site_t& site, match_p&& src) {
                RTTI::FPathName pathName;
                pathName.Identifier = RTTI::FName(src->Value());

                *dst = Parser::MakeVariableReference(pathName, site);
            }),
        Parser::Sequence<
            symbol_t::Dollar,
            symbol_t::Div,
            symbol_t::Identifier,
            symbol_t::Div,
            symbol_t::Identifier >()
        .Select<expr_t>([](
            expr_t* dst,
            const site_t& site,
            TTuple<match_p, match_p, match_p, match_p, match_p>&& src) {
            match_p transaction = std::get<2>(src);
            match_p identifier = std::get<4>(src);

            RTTI::FPathName pathName;
            pathName.Transaction = RTTI::FName(transaction->Value());
            pathName.Identifier = RTTI::FName(identifier->Value());

            *dst = Parser::MakeVariableReference(pathName, site);
        }),
        Parser::Sequence<
            symbol_t::Complement,
            symbol_t::Div,
            symbol_t::Identifier>()
        .Select<expr_t>([](expr_t* dst, const site_t& site, TTuple<match_p, match_p, match_p>&& src) {
            match_p identifier = std::get<2>(src);

            RTTI::FPathName pathName;
            pathName.Identifier = RTTI::FName(identifier->Value());

            *dst = Parser::MakeVariableReference(pathName, site);
        })
    ))
#else
,   _variable([](Parser::FParseList& input, expr_t* dst) -> Parser::FParseResult {
        site_t site;
        RTTI::FPathName pathName;

        match_p id_ = nullptr, scope_ = nullptr;
        if      (input.TryRead<symbol_t::Identifier>(&id_)) {
            site = id_->Site();
            pathName.Identifier = RTTI::FName(id_->Value());
        }
        else if (input.TryRead<symbol_t::Dollar>(&scope_)) {
            match_p transaction, sep;
            if (not input.Expect<symbol_t::Div>(&sep))
                return Parser::FParseResult::Unexpected(symbol_t::Div, sep, input);
            if (not input.Expect<symbol_t::Identifier>(&transaction))
                return Parser::FParseResult::Unexpected(symbol_t::Identifier, transaction, input);
            if (not input.Expect<symbol_t::Div>(&sep))
                return Parser::FParseResult::Unexpected(symbol_t::Div, sep, input);
            if (not input.Expect<symbol_t::Identifier>(&id_))
                return Parser::FParseResult::Unexpected(symbol_t::Identifier, id_, input);

            site = Lexer::FSpan::FromSite(scope_->Site(), id_->Site());
            pathName.Namespace = RTTI::FName(transaction->Value());
            pathName.Identifier = RTTI::FName(id_->Value());
        }
        else if (input.TryRead<symbol_t::Complement>(&scope_)) {
            match_p sep;
            if (not input.Expect<symbol_t::Div>(&sep))
                return Parser::FParseResult::Unexpected(symbol_t::Div, sep, input);
            if (not input.Expect<symbol_t::Identifier>(&id_))
                return Parser::FParseResult::Unexpected(symbol_t::Identifier, id_, input);

            site = Lexer::FSpan::FromSite(scope_->Site(), id_->Site());
            pathName.Identifier = RTTI::FName(id_->Value());
        }
        else {
            return Parser::FParseResult::Failure(input.Site());
        }

        *dst = Parser::MakeVariableReference(pathName, site);

        return Parser::FParseResult::Success(site);
    })
#endif

,   _reference(Parser::Switch(
        _literal,
        _tuple,
        _array,
        _dictionary,
        _object,
        _variable
    ))

,   _accessor([this](Parser::FParseList& input, expr_t* value) -> Parser::FParseResult {
        Parser::FParseResult result = _reference(input, value);

        while (result) {
            if (Unlikely(input.PeekType() == symbol_t::Dot)) {
                const match_p dot = input.Read(); // skip the dot

                match_p id;
                if (not input.Expect<symbol_t::Identifier>(&id))
                    input.Error("expected an identifier", input.Site());

                if (Unlikely(input.PeekType() == symbol_t::LParenthese)) {
                    Verify(input.Read()); // skip the lparen

                    Parser::TEnumerable<expr_t> args;

                    for (;;) {
                        expr_t arg;
                        const Parser::FParseResult r = _expr.TryParse(input, &arg);
                        if (not r.Succeed())
                            break;

                        args.push_back(std::move(arg));

                        if (input.PeekType() != symbol_t::Comma)
                            break;

                        Verify(input.Read()); // skip the comma separator
                    }

                    match_p rparen;
                    if (not input.Expect<symbol_t::RParenthese>(&rparen))
                        input.Error("expected a closing ')' for function call", input.Site());

                    *value = Parser::MakeFunctionCall(std::move(*value), RTTI::FName(id->Value()), args.MakeView(),
                        site_t::FromSite(dot->Site(), input.Site()) );

                }
                else {
                    *value = Parser::MakePropertyReference(std::move(*value), RTTI::FName(id->Value()),
                        site_t::FromSite(dot->Site(), id->Site()) );
                }
            }
            else if (Unlikely(input.PeekType() == symbol_t::LBracket)) {
                const match_p lbracket = input.Read(); // skip the [

                expr_t subscript;
                const Parser::FParseResult r = _expr.TryParse(input, &subscript);
                if (not r.Succeed())
                    break;

                match_p rbracket;
                if (not input.Expect<symbol_t::RBracket>(&rbracket))
                    input.Error("expected a closing ']' for subscript operator", input.Site());

                *value = Parser::MakeSubscriptOperator(std::move(*value), std::move(subscript),
                    site_t::FromSite(lbracket->Site(), input.Site()) );
            }
            else {
                break; // nothing matching, stop chain parsing
            }

            result = Parser::FParseResult::Success(result.Site, input.Site());
        }

        return result;
    })

,   _rvalue([this](Parser::FParseList& input, expr_t* value) -> Parser::FParseResult {
        RTTI::PTypeTraits traits; // cast with `<TYPENAME>:x`
        const Parser::FParseMatch* const start = input.Peek();
        if (Unlikely(const Parser::FParseResult cast = ExpectTypenameRTTI().TryParse(input, &traits))) {
            match_p colon = nullptr;
            if (input.TryRead<symbol_t::Colon>(&colon)) {
                expr_t expr;
                const Parser::FParseResult inner = _unary(input, &expr);
                if (Unlikely(not inner))
                    return inner;

                const site_t site{ Lexer::FSpan::FromSite(cast.Site, expr->Site()) };
                *value = Parser::MakeCastExpr(traits, expr.get(), site);

                return Parser::FParseResult::Success(site);
            }

            // restore input before fall-backing to accessor
            input.Seek(start);
        }

        return _accessor(input, value);
    })

,   _pow(Parser::BinaryOp<symbol_t::Pow, expr_t>(_rvalue,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Pow>(), lhs.get(), rhs.get(), op->Site());
        }))

,   _unary(Parser::UnaryOp<
        symbol_t::Add |
        symbol_t::Sub |
        symbol_t::Not |
        symbol_t::Complement, expr_t >(_pow,
        [](match_p op, const expr_t& rhs) -> expr_t {
            switch (op->Symbol()->Type()) {
            case symbol_t::Add:
                return rhs; // +1 <=> 1 : nothing to do
            case symbol_t::Sub:
                return Parser::MakeUnaryFunction(TUnaryOp<TUnOp_Sub>(), rhs.get(), op->Site());
            case symbol_t::Not:
                return Parser::MakeUnaryFunction(TUnaryOp<TUnOp_Not>(), rhs.get(), op->Site());
            case symbol_t::Complement:
                return Parser::MakeUnaryFunction(TUnaryOp<TUnOp_Cpl>(), rhs.get(), op->Site());
            default:
                AssertNotReached();
            }
        }))

,   _mulDivMod(Parser::BinaryOp<
        symbol_t::Mul |
        symbol_t::Div |
        symbol_t::Mod, expr_t >(_unary,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            switch (op->Symbol()->Type()) {
            case symbol_t::Mul:
                return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Mul>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::Div:
                return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Div>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::Mod:
                return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Mod>(), lhs.get(), rhs.get(), op->Site());
            default:
                AssertNotReached();
            }
        }))

,   _addSub(Parser::BinaryOp<
        symbol_t::Add |
        symbol_t::Sub, expr_t>(_mulDivMod,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            switch (op->Symbol()->Type()) {
            case symbol_t::Add:
                return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Add>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::Sub:
                return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Sub>(), lhs.get(), rhs.get(), op->Site());
            default:
                AssertNotReached();
            }
        }))

,   _lshRsh(Parser::BinaryOp<
        symbol_t::LShift |
        symbol_t::RShift, expr_t>(_addSub,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            switch (op->Symbol()->Type()) {
            case symbol_t::LShift:
                return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Lsh>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::RShift:
                return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Rsh>(), lhs.get(), rhs.get(), op->Site());
            default:
                AssertNotReached();
            }
        }))

,   _compare(Parser::BinaryOp<
        symbol_t::Less |
        symbol_t::LessOrEqual |
        symbol_t::Greater |
        symbol_t::GreaterOrEqual, expr_t>(_lshRsh,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            switch (op->Symbol()->Type()) {
            case symbol_t::Less:
                return Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_Less>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::LessOrEqual:
                return Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_LessOrEqual>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::Greater:
                return Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_Greater>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::GreaterOrEqual:
                return Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_GreaterOrEqual>(), lhs.get(), rhs.get(), op->Site());
            default:
                AssertNotReached();
            }
        }))

,   _equalsNotEquals(Parser::BinaryOp<
        symbol_t::Equals |
        symbol_t::NotEquals, expr_t>(_compare,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            switch (op->Symbol()->Type()) {
            case symbol_t::Equals:
                return Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_Equals>(), lhs.get(), rhs.get(), op->Site());
            case symbol_t::NotEquals:
                return Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_NotEquals>(), lhs.get(), rhs.get(), op->Site());
            default:
                AssertNotReached();
            }
        }))

,   _and(Parser::BinaryOp<symbol_t::And, expr_t>(_equalsNotEquals,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_And>(), lhs.get(), rhs.get(), op->Site());
        }))

,   _xor(Parser::BinaryOp<symbol_t::Xor, expr_t>(_and,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Xor>(), lhs.get(), rhs.get(), op->Site());
        }))

,   _or(Parser::BinaryOp<symbol_t::Or, expr_t>(_xor,
        [](const expr_t& lhs, match_p op, const expr_t& rhs) -> expr_t {
            return Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Or>(), lhs.get(), rhs.get(), op->Site());
        }))

,   _ternary(Parser::TernaryOp<symbol_t::Question, symbol_t::Colon, expr_t>(_or,
        [](const expr_t& cond, match_p question, const expr_t& ifTrue, match_p colon, const expr_t& ifFalse) -> expr_t {
            UNUSED(question);
            UNUSED(colon);
            return Parser::MakeTernary(FTernaryOp(), cond.get(), ifTrue.get(), ifFalse.get(),
                site_t::FromSite(cond->Site(), ifFalse->Site()) );
        }))

{}
//----------------------------------------------------------------------------
FGrammarImpl::~FGrammarImpl() = default;
//----------------------------------------------------------------------------
Parser::PCParseExpression FGrammarImpl::ParseExpression(Parser::FParseList& input) const {
    Parser::PCParseExpression result; // exception safety
    if (input.Peek())
        result = _expr.Parse(input);
    return result;
}
//----------------------------------------------------------------------------
Parser::PCParseStatement FGrammarImpl::ParseStatement(Parser::FParseList& input) const {
    Parser::PCParseStatement result; // exception safety
    if (input.Peek())
        result = _statement.Parse(input);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static const FGrammarImpl *sGrammarImpl = nullptr;
//----------------------------------------------------------------------------
void FGrammarStartup::Start() {
    AssertIsMainThread();
    AssertRelease(nullptr == sGrammarImpl);

    sGrammarImpl = new FGrammarImpl();
}
//----------------------------------------------------------------------------
void FGrammarStartup::Shutdown() {
    AssertIsMainThread();
    AssertRelease(nullptr != sGrammarImpl);

    checked_delete_ref(sGrammarImpl);
}
//----------------------------------------------------------------------------
void FGrammarStartup::ClearAll_UnusedMemory()
{}
//----------------------------------------------------------------------------
Parser::PCParseExpression FGrammarStartup::ParseExpression(Parser::FParseList& input) {
    Assert(sGrammarImpl);
    return sGrammarImpl->ParseExpression(input);
}
//----------------------------------------------------------------------------
Parser::PCParseStatement FGrammarStartup::ParseStatement(Parser::FParseList& input) {
    Assert(sGrammarImpl);
    return sGrammarImpl->ParseStatement(input);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE

PRAGMA_MSVC_WARNING_POP()
