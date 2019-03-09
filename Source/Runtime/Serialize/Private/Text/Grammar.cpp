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

// workaround a bug in Profiling : c4702 unreachable code - #TODO test after a few updates that this is no more necessary
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

                if (RTTI::is_boolean(rhs_type_id)) {
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&b)))
                        NOOP(); // b was already assigned
                }
                else if (RTTI::is_signed_integral(rhs_type_id)) {
                    FParseInteger i;
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&i)))
                        b = (FParseInteger(0) != i);
                }
                else if (RTTI::is_unsigned_integral(rhs_type_id)) {
                    FParseUnsigned u;
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&u)))
                        b = (FParseUnsigned(0) != u);
                }
                else if (RTTI::is_floating_point(rhs_type_id)) {
                    FParseFloat fp;
                    if (rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                        b = (FParseFloat(0) != fp);
                }
                else if (RTTI::is_string(rhs_type_id)) {
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

            if (RTTI::is_signed_integral(rhs_type_id)) {
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&integer)))
                    return context->CreateAtomFrom(
                        _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseInteger>(), integer));
            }
            else if (RTTI::is_unsigned_integral(rhs_type_id)) {
                FParseUnsigned u;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&u)))
                    return context->CreateAtomFrom(
                        _Op< FParseUnsigned >()(lhs, lhs_value.FlatData<FParseInteger>(), u));
            }
            else if (RTTI::is_floating_point(rhs_type_id)) {
                FParseFloat fp;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                    return context->CreateAtomFrom(
                        _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value.FlatData<FParseInteger>()), fp));
            }
            else if (RTTI::is_boolean(rhs_type_id)) {
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

            if (RTTI::is_unsigned_integral(rhs_type_id)) {
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&unsign_d)))
                    return context->CreateAtomFrom(
                        _Op< FParseInteger >()(lhs, lhs_value.FlatData<FParseUnsigned>(), unsign_d));
            }
            else if (RTTI::is_unsigned_integral(rhs_type_id)) {
                FParseInteger i;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&i)))
                    return context->CreateAtomFrom(
                        _Op< FParseUnsigned >()(lhs, lhs_value.FlatData<FParseUnsigned>(), i));
            }
            else if (RTTI::is_floating_point(rhs_type_id)) {
                FParseFloat fp;
                if (rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                    return context->CreateAtomFrom(
                        _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value.FlatData<FParseUnsigned>()), fp));
            }
            else if (RTTI::is_boolean(rhs_type_id)) {
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

            if (RTTI::is_floating_point(rhs_type_id)) {
                FParseFloat fp;
                Verify(rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)));
                f = fp;
            }
            else if (RTTI::is_signed_integral(rhs_type_id)) {
                FParseInteger i;
                Verify(rhs_value.PromoteCopy(RTTI::MakeAtom(&i)));
                f = static_cast<FParseFloat>(i);
            }
            else if (RTTI::is_unsigned_integral(rhs_type_id)) {
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

            if (RTTI::is_string(rhs_type_id) && rhs_value.PromoteCopy(RTTI::MakeAtom(&s)))
                oss << s;
            else if (RTTI::is_signed_integral(rhs_type_id) && rhs_value.PromoteCopy(RTTI::MakeAtom(&i)))
                oss << i;
            else if (RTTI::is_unsigned_integral(rhs_type_id) && rhs_value.PromoteCopy(RTTI::MakeAtom(&u)))
                oss << u;
            else if (RTTI::is_floating_point(rhs_type_id) && rhs_value.PromoteCopy(RTTI::MakeAtom(&fp)))
                oss << fp;
            else if (RTTI::is_boolean(rhs_type_id) && rhs_value.PromoteCopy(RTTI::MakeAtom(&b)))
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
            const RTTI::FTypeId typeId = lhs_value.TypeId();

            if (RTTI::is_signed_integral(typeId)) {
                FParseInteger i;
                RTTI::FAtom lhs_integer(RTTI::MakeAtom(&i));
                if (lhs_value.PromoteCopy(lhs_integer))
                    return IntegerOp_(context, lhs, rhs, lhs_integer, rhs_value);
            }
            else if (RTTI::is_unsigned_integral(typeId)) {
                FParseUnsigned u;
                RTTI::FAtom lhs_unsigned(RTTI::MakeAtom(&u));
                if (lhs_value.PromoteCopy(lhs_unsigned))
                    return IntegerOp_(context, lhs, rhs, lhs_unsigned, rhs_value);
            }
            else if (RTTI::is_floating_point(typeId)) {
                FParseFloat fp;
                RTTI::FAtom lhs_fp(RTTI::MakeAtom(&fp));
                if (lhs_value.PromoteCopy(lhs_fp))
                    return FloatOp_(context, lhs, rhs, lhs_fp, rhs_value);
            }
            else if (RTTI::is_boolean(typeId)) {
                FParseBool b;
                RTTI::FAtom lhs_bool(RTTI::MakeAtom(&b));
                if (lhs_value.PromoteCopy(lhs_bool))
                    return BooleanOp_(context, lhs, rhs, lhs_bool, rhs_value);
            }
            else if (RTTI::is_string(typeId)) {
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
using PTypeTraitsWithSite = TPair<RTTI::PTypeTraits, Lexer::FSpan>;
static Parser::TProduction<PTypeTraitsWithSite> ExpectTypenameRTTI() {
    return Parser::TProduction<PTypeTraitsWithSite>{
        [](Parser::FParseList& input) -> Parser::TParseResult<PTypeTraitsWithSite> {
            const Lexer::FMatch *match = input.Read();

            RTTI::PTypeTraits traits;

            if (match) {
                if (match->Symbol()->Type() == Lexer::FSymbol::Typename)
                    traits = RTTI::MakeTraits(RTTI::ENativeType(match->Symbol()->Ord()));
                else if (match->Symbol()->Type() == Lexer::FSymbol::Identifier)
                    traits = RTTI::MakeTraitsFromTypename(match->Value().MakeView());
            }

            return (traits
                ? Parser::TParseResult<PTypeTraitsWithSite>::Success(MakePair(traits, match->Site()), match->Site())
                : Parser::TParseResult<PTypeTraitsWithSite>::Unexpected(Lexer::FSymbol::Typename, match, input) );
        }};
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGrammarImpl {
public:
    FGrammarImpl();
    ~FGrammarImpl();

    Parser::PCParseExpression ParseExpression(Parser::FParseList& input) const;
    Parser::PCParseStatement ParseStatement(Parser::FParseList& input) const;

private:
    using match_p = const Lexer::FMatch*;

    Parser::TProduction< Parser::PCParseExpression > _literal;
    Parser::TProduction< Parser::PCParseExpression > _object;
    Parser::TProduction< Parser::PCParseExpression > _cast;
    Parser::TProduction< Parser::PCParseExpression > _variable;
    Parser::TProduction< Parser::PCParseExpression > _reference;
    Parser::TProduction< Parser::PCParseExpression > _rvalue;

    Parser::TProduction< Parser::PCParseExpression > _pow;
    Parser::TProduction< Parser::PCParseExpression > _unary;
    Parser::TProduction< Parser::PCParseExpression > _mulDivMod;
    Parser::TProduction< Parser::PCParseExpression > _addSub;
    Parser::TProduction< Parser::PCParseExpression > _lshRsh;
    Parser::TProduction< Parser::PCParseExpression > _compare;
    Parser::TProduction< Parser::PCParseExpression > _equalsNotEquals;
    Parser::TProduction< Parser::PCParseExpression > _and;
    Parser::TProduction< Parser::PCParseExpression > _xor;
    Parser::TProduction< Parser::PCParseExpression > _or;
    Parser::TProduction< Parser::PCParseExpression > _ternary;

    Parser::TProduction< Parser::PCParseExpression > _tuple;
    Parser::TProduction< Parser::PCParseExpression > _array;
    Parser::TProduction< Parser::PCParseExpression > _dictionary;

    Parser::TProduction< Parser::PCParseExpression > _lvalue;
    Parser::TProduction< Parser::PCParseExpression > _export;

    Parser::TProduction< Parser::PCParseExpression > _expr;

    Parser::TProduction< Parser::PCParseStatement >  _property;
    Parser::TProduction< Parser::PCParseStatement >  _statement;
};
//----------------------------------------------------------------------------
FGrammarImpl::FGrammarImpl()
:   _literal(
        Parser::Expect(Lexer::FSymbol::Null).Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(FParseObject(), m->Site());
    })
    .Or(Parser::Expect(Lexer::FSymbol::True).Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(true, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::False).Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(false, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::Integer).Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        i64 i;
        Verify(Atoi(&i, m->Value().MakeView(), 10));
        return Parser::MakeLiteral(i, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::Unsigned).Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        u64 u;
        Verify(Atoi(&u, m->Value().MakeView(), 10));
        return Parser::MakeLiteral(u, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::Float).Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        double d;
        Verify(Atod(&d, m->Value().MakeView()));
        return Parser::MakeLiteral(d, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::String).Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m/* && m->Value().size()*//* strings can be empty */);
        return Parser::MakeLiteral(m->Value(), m->Site());
    }))
    )

,   _object(
            Parser::Expect(Lexer::FSymbol::Identifier)
    .And(   Parser::Expect(Lexer::FSymbol::LBrace))
    .And(   _property.Ref().Many() )
    .And(   Parser::Expect(Lexer::FSymbol::RBrace))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, match_p, const Parser::TEnumerable<Parser::PCParseStatement>, match_p>& args) -> Parser::PCParseExpression {
        match_p name = std::get<0>(args);
        const Parser::TEnumerable<Parser::PCParseStatement>& statements = std::get<2>(args);

        Assert(name);

        return Parser::MakeObjectDefinition(RTTI::FName(name->Value()), name->Site(), statements.begin(), statements.end());
    }))

,   _cast(
            ExpectTypenameRTTI()
    .And(   Parser::Expect(Lexer::FSymbol::Colon))
#if 0
    .And(   _rvalue.Ref()   )
#else
    .And(   _unary.Ref()    ) // must allow '-123' which uses the unary operator '-' for instance
#endif
        .Select<Parser::PCParseExpression>([](const TTuple<PTypeTraitsWithSite, match_p, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            const PTypeTraitsWithSite& traitsWSite = std::get<0>(args);
            const Parser::PCParseExpression& rvalue = std::get<2>(args);

            const Lexer::FSpan site = Lexer::FSpan::FromSpan(
                traitsWSite.second,
                std::get<1>(args)->Site()
            );

            return Parser::MakeCastExpr(traitsWSite.first, rvalue.get(), site);
        })
    )

,   _variable(
        Parser::Expect(Lexer::FSymbol::Identifier)
    .Select<Parser::PCParseExpression>([](match_p&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());

        RTTI::FPathName pathName;
        pathName.Identifier = RTTI::FName(m->Value());

        return Parser::MakeVariableReference(pathName, m->Site());
    })
    .Or(        Parser::Expect(Lexer::FSymbol::Dollar)
        .And(   Parser::Expect(Lexer::FSymbol::Div))
        .And(   Parser::Expect(Lexer::FSymbol::Identifier))
        .And(   Parser::Expect(Lexer::FSymbol::Div))
        .And(   Parser::Expect(Lexer::FSymbol::Identifier))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, match_p, match_p, match_p, match_p>& args) -> Parser::PCParseExpression {
        match_p transaction = std::get<2>(args);
        match_p identifier = std::get<4>(args);

        RTTI::FPathName pathName;
        pathName.Transaction = RTTI::FName(transaction->Value());
        pathName.Identifier = RTTI::FName(identifier->Value());

        Assert(not pathName.Transaction.empty());

        const Lexer::FSpan site = Lexer::FSpan::FromSpan(
            std::get<0>(args)->Site(),
            std::get<4>(args)->Site()
        );

        return Parser::MakeVariableReference(pathName, site);
    }))
    .Or(        Parser::Expect(Lexer::FSymbol::Complement)
        .And(   Parser::Expect(Lexer::FSymbol::Div))
        .And(   Parser::Expect(Lexer::FSymbol::Identifier))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, match_p, match_p>& args) -> Parser::PCParseExpression {
        match_p identifier = std::get<2>(args);

        RTTI::FPathName pathName;
        pathName.Identifier = RTTI::FName(identifier->Value());

        const Lexer::FSpan site = Lexer::FSpan::FromSpan(
            std::get<0>(args)->Site(),
            std::get<2>(args)->Site()
        );

        return Parser::MakeVariableReference(pathName, site);
    })))

,   _reference(
        _literal.Ref()
    .Or(_tuple.Ref())
    .Or(_array.Ref())
    .Or(_dictionary.Ref())
    .Or(_object.Ref())
    .Or(_cast.Ref())
    .Or(_variable.Ref())
    .Or(Parser::Expect(Lexer::FSymbol::LParenthese).And(_expr.Ref()).And(Parser::Expect(Lexer::FSymbol::RParenthese))
        .Select<Parser::PCParseExpression>([](const TTuple<match_p, Parser::PCParseExpression, match_p>& args) -> Parser::PCParseExpression {
            return std::get<1>(args);
        })))

,   _rvalue(
        _reference.Ref()
    .Optional(  Parser::Expect(Lexer::FSymbol::Dot)
        .And(   Parser::Expect(Lexer::FSymbol::Identifier))
        .AtLeastOnce()
        .Optional(  Parser::Expect(Lexer::FSymbol::LParenthese)
            .And(   _expr.Ref().Join(Lexer::FSymbol::Comma))
            .And(   Parser::Expect(Lexer::FSymbol::RParenthese))))
    .Select<Parser::PCParseExpression>([](const TTuple<
        Parser::PCParseExpression,
        Meta::TOptional<TTuple<
            Parser::TEnumerable< TTuple<match_p, match_p> >,
            Meta::TOptional< TTuple<match_p, Parser::TEnumerable<Parser::PCParseExpression>, match_p> >
        >>>& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& rvalue = std::get<0>(args);
        const auto& member = std::get<1>(args);

        Assert(rvalue);
        if (not member.has_value())
            return rvalue; // this is a simple reference

        const Parser::TEnumerable< TTuple<match_p, match_p> >& indirection = std::get<0>(*member);
        Assert_NoAssume(not indirection.empty());

        const Meta::TOptional< TTuple<match_p, Parser::TEnumerable<Parser::PCParseExpression>, match_p> >& funcall = std::get<1>(*member);

        if (funcall.has_value()) { // this is a function call

            Parser::PCParseExpression target = rvalue;
            for (const auto& it : indirection.MakeConstView().ShiftBack()) {
                match_p const name = std::get<1>(it);
                target = Parser::MakePropertyReference(target, RTTI::FName(name->Value()), name->Site());
            }

            match_p const funname = std::get<1>(indirection.back()); // the last id is the function name
            match_p const lparen = std::get<0>(*funcall);
            const Parser::TEnumerable<Parser::PCParseExpression>& funargs = std::get<1>(*funcall);

            const Lexer::FSpan site = Lexer::FSpan::FromSpan(
                std::get<0>(args)->Site(),
                lparen->Site()
            );

            return Parser::MakeFunctionCall(std::move(target), RTTI::FName(funname->Value()), funargs, site);
        }
        else { // this is a property reference

            Parser::PCParseExpression target = rvalue;
            for (const auto& it : indirection) {
                match_p const name = std::get<1>(it);

                const Lexer::FSpan site = Lexer::FSpan::FromSpan(
                    std::get<0>(it)->Site(),
                    name->Site()
                );

                target = Parser::MakePropertyReference(target, RTTI::FName(name->Value()), site);
            }

            return target;
        }
    }))

,   _pow(
    _rvalue.Ref().And(Parser::Expect(Lexer::FSymbol::Pow).And(_rvalue.Ref()).Many())
        .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
            const Parser::PCParseExpression& lhs = std::get<0>(args);
            const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

            Parser::PCParseExpression result = lhs;

            for (const auto& it : ops) {
                match_p op = std::get<0>(it);
                const Parser::PCParseExpression& rhs = std::get<1>(it);

                Assert(Lexer::FSymbol::Pow == op->Symbol()->Type());
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Pow>(), result.get(), rhs.get(), op->Site());
            }

            return result;
        })
    )

,   _unary(
    Parser::ExpectMask(
        Lexer::FSymbol::Add
    |   Lexer::FSymbol::Sub
    |   Lexer::FSymbol::Not
    |   Lexer::FSymbol::Complement)
    .Many()
    .And(_pow.Ref())
        .Select<Parser::PCParseExpression>([](const TTuple<Parser::TEnumerable<match_p>, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            const Parser::TEnumerable<match_p>& ops = std::get<0>(args);
            const Parser::PCParseExpression& expr = std::get<1>(args);

            Parser::PCParseExpression result = expr;

            for (match_p op : ops) {
                switch (op->Symbol()->Type()) {
                case Lexer::FSymbol::Add:
                    return expr;

                case Lexer::FSymbol::Sub:
                    result = Parser::MakeUnaryFunction(TUnaryOp<TUnOp_Sub>(), result.get(), op->Site());
                    break;

                case Lexer::FSymbol::Not:
                    result = Parser::MakeUnaryFunction(TUnaryOp<TUnOp_Not>(), result.get(), op->Site());
                    break;

                default:
                    Assert(Lexer::FSymbol::Complement == op->Symbol()->Type());
                    result = Parser::MakeUnaryFunction(TUnaryOp<TUnOp_Cpl>(), result.get(), op->Site());
                    break;
                }
            }

            return result;
        })
    )

,   _mulDivMod(
    _unary.Ref().And(Parser::ExpectMask(
        Lexer::FSymbol::Mul
    |   Lexer::FSymbol::Div
    |   Lexer::FSymbol::Mod).And(_unary.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            switch (op->Symbol()->Type()) {
            case Lexer::FSymbol::Mul:
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Mul>(), result.get(), rhs.get(), op->Site());
                break;

            case Lexer::FSymbol::Div:
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Div>(), result.get(), rhs.get(), op->Site());
                break;

            default:
                Assert(Lexer::FSymbol::Mod == op->Symbol()->Type());
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Mod>(), result.get(), rhs.get(), op->Site());
                break;
            }
        }

        return result;
    }))

,   _addSub(
    _mulDivMod.Ref().And(Parser::ExpectMask(
        Lexer::FSymbol::Add
    |   Lexer::FSymbol::Sub).And(_mulDivMod.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            if (op->Symbol()->Type() == Lexer::FSymbol::Add) {
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Add>(), result.get(), rhs.get(), op->Site());
            }
            else {
                Assert(op->Symbol()->Type() == Lexer::FSymbol::Sub);
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Sub>(), result.get(), rhs.get(), op->Site());
            }
        }

        return result;
    }))

,   _lshRsh(
    _addSub.Ref().And(Parser::ExpectMask(
        Lexer::FSymbol::LShift
    |   Lexer::FSymbol::RShift).And(_addSub.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            if (op->Symbol()->Type() == Lexer::FSymbol::LShift) {
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Lsh>(), result.get(), rhs.get(), op->Site());
            }
            else {
                Assert(op->Symbol()->Type() == Lexer::FSymbol::RShift);
                result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Rsh>(), result.get(), rhs.get(), op->Site());
            }
        }

        return result;
    }))

,   _compare(
    _lshRsh.Ref().And(Parser::ExpectMask(
        Lexer::FSymbol::Less
    |   Lexer::FSymbol::LessOrEqual
    |   Lexer::FSymbol::Greater
    |   Lexer::FSymbol::GreaterOrEqual).And(_lshRsh.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            switch (op->Symbol()->Type())
            {
            case Lexer::FSymbol::Less:
                result = Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_Less>(), result.get(), rhs.get(), op->Site());
                break;

            case Lexer::FSymbol::LessOrEqual:
                result = Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_LessOrEqual>(), result.get(), rhs.get(), op->Site());
                break;

            case Lexer::FSymbol::Greater:
                result = Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_Greater>(), result.get(), rhs.get(), op->Site());
                break;

            default:
                Assert(op->Symbol()->Type() == Lexer::FSymbol::GreaterOrEqual);
                result = Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_GreaterOrEqual>(), result.get(), rhs.get(), op->Site());
                break;
            }
        }

        return result;
    }))

,   _equalsNotEquals(
    _compare.Ref().And(Parser::ExpectMask(
        Lexer::FSymbol::Equals
    |   Lexer::FSymbol::NotEquals).And(_compare.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            if (op->Symbol()->Type() == Lexer::FSymbol::Equals) {
                result = Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_Equals>(), result.get(), rhs.get(), op->Site());
            }
            else {
                Assert(op->Symbol()->Type() == Lexer::FSymbol::NotEquals);
                result = Parser::MakeBinaryFunction(TBinaryOp<FCmpOp_NotEquals>(), result.get(), rhs.get(), op->Site());
            }
        }

        return result;
    }))

,   _and(
    _equalsNotEquals.Ref().And(Parser::Expect(Lexer::FSymbol::And).And(_equalsNotEquals.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::FSymbol::And);
            result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_And>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _xor(
    _and.Ref().And(Parser::Expect(Lexer::FSymbol::Xor).And(_and.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::FSymbol::Xor);
            result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Xor>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _or(
    _xor.Ref().And(Parser::Expect(Lexer::FSymbol::Or).And(_xor.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::FSymbol::Or);
            result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Or>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _ternary(
    _or.And(
                Parser::Expect(Lexer::FSymbol::Question)
        .And(   _or.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::Colon))
        .And(   _or.Ref())
        .Many() )
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression, match_p, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression, match_p, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            match_p question = std::get<0>(it);
            match_p colon = std::get<2>(it);

            Assert(question && question->Symbol()->Type() == Lexer::FSymbol::Question);
            Assert(colon && colon->Symbol()->Type() == Lexer::FSymbol::Colon);
            UNUSED(colon);

            const Parser::PCParseExpression& ptrue = std::get<1>(it);
            const Parser::PCParseExpression& pfalse = std::get<3>(it);

            result = Parser::MakeTernary(FTernaryOp(), result.get(), ptrue.get(), pfalse.get(), question->Site());
        }

        return result;
    }))

,   _tuple(
                Parser::Expect(Lexer::FSymbol::LParenthese)
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::Comma).And(_expr.Ref()).Many() )
        .And(   Parser::Expect(Lexer::FSymbol::RParenthese))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, Parser::PCParseExpression, match_p, Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >, match_p> & args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& arg0 = std::get<1>(args);
        const Parser::PCParseExpression& arg1 = std::get<3>(args);
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& args_1_N = std::get<4>(args);

        Parser::FTupleExpr::elements_type elts;
        elts.reserve(2 + args_1_N.size());
        elts.push_back(arg0);
        elts.push_back(arg1);

        for (const auto& it : args_1_N)
            elts.push_back(std::get<1>(it));

        const Lexer::FSpan site = Lexer::FSpan::FromSpan(
            std::get<0>(args)->Site(),
            std::get<5>(args)->Site()
        );

        return Parser::MakeTupleExpr(std::move(elts), site);
    }))

,   _array(
            Parser::Expect(Lexer::FSymbol::LBracket)
    .And(   Parser::Expect(Lexer::FSymbol::RBracket))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, match_p>& args) -> Parser::PCParseExpression {
        const Lexer::FSpan site = Lexer::FSpan::FromSpan(
            std::get<0>(args)->Site(),
            std::get<1>(args)->Site()
        );

        return Parser::MakeArrayExpr(site);
    })
    .Or(    Parser::Expect(Lexer::FSymbol::LBracket)
    .And(   _expr.Ref())
    .And(   Parser::Expect(Lexer::FSymbol::Comma).And(_expr.Ref()).Many() )
    .And(   Parser::Expect(Lexer::FSymbol::RBracket))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, Parser::PCParseExpression, Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >, match_p>& args) -> Parser::PCParseExpression {
        const Parser::TEnumerable<TTuple<match_p, Parser::PCParseExpression> >& items_1_N = std::get<2>(args);

        Parser::FArrayExpr::items_type arr;
        arr.reserve(1 + items_1_N.size());
        arr.push_back(std::get<1>(args));

        for (const auto& it : items_1_N)
            arr.push_back(std::get<1>(it));

        const Lexer::FSpan site = Lexer::FSpan::FromSpan(
            std::get<0>(args)->Site(),
            std::get<3>(args)->Site()
        );

        return Parser::MakeArrayExpr(std::move(arr), site);
    })
    ))

,   _dictionary(
            Parser::Expect(Lexer::FSymbol::LBrace)
    .And(   Parser::Expect(Lexer::FSymbol::RBrace))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, match_p>& args) -> Parser::PCParseExpression {
        const Lexer::FSpan site = Lexer::FSpan::FromSpan(
            std::get<0>(args)->Site(),
            std::get<1>(args)->Site()
        );

        return Parser::MakeDictionaryExpr(site);
    })
        .Or(Parser::Expect(Lexer::FSymbol::LBrace)
    .And(
                Parser::Expect(Lexer::FSymbol::LParenthese)
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::RParenthese))
        .Select<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >([](const TTuple<match_p, Parser::PCParseExpression, match_p, Parser::PCParseExpression, match_p>& args) -> TTuple<Parser::PCParseExpression, Parser::PCParseExpression> {
            const Parser::PCParseExpression& lhs = std::get<1>(args);
            const Parser::PCParseExpression& rhs = std::get<3>(args);

            return MakeTuple(lhs, rhs);
        })
        )
    .And(
                Parser::Expect(Lexer::FSymbol::Comma)
        .And(   Parser::Expect(Lexer::FSymbol::LParenthese))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::RParenthese))
        .Select<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >([](const TTuple<match_p, match_p, Parser::PCParseExpression, match_p, Parser::PCParseExpression, match_p>& args) -> TTuple<Parser::PCParseExpression, Parser::PCParseExpression> {
            const Parser::PCParseExpression& lhs = std::get<2>(args);
            const Parser::PCParseExpression& rhs = std::get<4>(args);

            return MakeTuple(lhs, rhs);
        })
        .Many()
        )
    .And(   Parser::Expect(Lexer::FSymbol::RBrace))
    .Select<Parser::PCParseExpression>([](const TTuple<match_p, Parser::PCParseExpression, Parser::PCParseExpression, Parser::TEnumerable<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >, match_p>& args) -> Parser::PCParseExpression {
        const Parser::TEnumerable<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >& items_1_N = std::get<3>(args);

        Parser::FDictionaryExpr::dico_type dico;
        dico.reserve(1 + items_1_N.size());
        dico.Add(std::get<1>(args)) = std::get<2>(args);

        for (const auto& it : items_1_N)
            dico.Add(std::get<0>(it)) = std::get<1>(it);

        const Lexer::FSpan site = Lexer::FSpan::FromSpan(
            std::get<0>(args)->Site(),
            std::get<4>(args)->Site()
        );

        return Parser::MakeDictionaryExpr(std::move(dico), site);
    })
    ))


,   _lvalue(    _ternary.Ref()
            .Or(_rvalue.Ref())
    )

,   _export(
            Parser::Optional(Lexer::FSymbol::Export)
    .And(   Parser::Expect(Lexer::FSymbol::Identifier))
    .And(   Parser::Expect(Lexer::FSymbol::Is))
    .And(   _lvalue.Ref() )
        .Select<Parser::PCParseExpression>([](const TTuple<match_p, match_p, match_p, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            match_p const exportIFP = std::get<0>(args);
            match_p const name = std::get<1>(args);
            const Parser::PCParseExpression& value = std::get<3>(args);

            Assert(name);
            Assert(value);

            const Parser::FVariableExport::EFlags scope = (exportIFP)
                ? Parser::FVariableExport::Global
                : Parser::FVariableExport::Public;

            const Lexer::FSpan site = Lexer::FSpan::FromSpan(
                name->Site(),
                std::get<2>(args)->Site()
            );

            return Parser::MakeVariableExport(RTTI::FName(name->Value()), value, scope, site);
        })
    )

,   _expr(  _export.Ref()
        .Or(_lvalue.Ref() )
    )

,   _property(
            Parser::Expect(Lexer::FSymbol::Identifier)
    .And(   Parser::Expect(Lexer::FSymbol::Assignment))
    .And(   _expr.Ref())
        .Select<Parser::PCParseStatement>([](TTuple<match_p, match_p, Parser::PCParseExpression>&& args) -> Parser::PCParseStatement {
            match_p name = std::get<0>(args);
            const Parser::PCParseExpression& value = std::get<2>(args);

            Assert(name);
            Assert(value);

            return Parser::MakePropertyAssignment(RTTI::FName(name->Value()), value, name->Site());
        })
    )

,   _statement(
        _expr.Ref().Select<Parser::PCParseStatement>([](Parser::PCParseExpression&& expr) -> Parser::PCParseStatement {
            return Parser::MakeEvalExpr(expr);
        })
    )

{}
//----------------------------------------------------------------------------
FGrammarImpl::~FGrammarImpl() {}
//----------------------------------------------------------------------------
Parser::PCParseExpression FGrammarImpl::ParseExpression(Parser::FParseList& input) const {
    return ((input.Peek() == nullptr)
        ? Parser::PCParseExpression{}
        : _expr.Parse(input) );
}
//----------------------------------------------------------------------------
Parser::PCParseStatement FGrammarImpl::ParseStatement(Parser::FParseList& input) const {
    return ((input.Peek() == nullptr)
        ? Parser::PCParseStatement{}
        : _statement.Parse(input) );
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
