#include "stdafx.h"

#pragma warning( push )
#pragma warning( disable : 4503) // C4503 'XXX'�: longueur du nom d�cor� d�pass�e, le nom a �t� tronqu�

#include "Grammar.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaTypePromote.h"

#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/Thread/ThreadContext.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
typedef bool                FParseBool;
typedef int64_t             FParseInt;
typedef double              FParseFloat;
typedef Core::FString       FParseString; // TODO - FWString, change FLexer to wchar_t
typedef RTTI::PMetaAtom     FParseAtom;
typedef RTTI::PMetaObject   FParseObject;
//----------------------------------------------------------------------------
STATIC_ASSERT(RTTI::TMetaType<FParseBool    >::TypeId == 1 );
STATIC_ASSERT(RTTI::TMetaType<FParseInt     >::TypeId == 5 );
STATIC_ASSERT(RTTI::TMetaType<FParseFloat   >::TypeId == 11);
STATIC_ASSERT(RTTI::TMetaType<FParseString  >::TypeId == 33);
STATIC_ASSERT(RTTI::TMetaType<FParseAtom    >::TypeId == 35);
STATIC_ASSERT(RTTI::TMetaType<FParseObject  >::TypeId == 36);
//----------------------------------------------------------------------------
enum EParseTypeId : RTTI::FMetaTypeId {
    PARSEID_BOOL    = RTTI::TMetaType<FParseBool  >::TypeId,
    PARSEID_INT     = RTTI::TMetaType<FParseInt   >::TypeId,
    PARSEID_FLOAT   = RTTI::TMetaType<FParseFloat >::TypeId,
    PARSEID_STRING  = RTTI::TMetaType<FParseString>::TypeId,
    PARSEID_OBJECT  = RTTI::TMetaType<FParseObject>::TypeId
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
            CORE_THROW_IT(Parser::FParserException("unary operator " STRINGIZE(_Op) " is not available for <" STRINGIZE(_Type) ">", expr->Site(), expr)); \
        } \
    }

FORBID_UNARYOPERATOR_FUNCTOR(Add, +, FParseBool);
FORBID_UNARYOPERATOR_FUNCTOR(Sub, -, FParseBool);
FORBID_UNARYOPERATOR_FUNCTOR(Cpl, ~, FParseBool);

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
            CORE_THROW_IT(Parser::FParserException("binary operator " STRINGIZE(_Op) " is not available for <" STRINGIZE(_Type) ">", expr->Site(), expr)); \
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
#pragma warning(push)
#pragma warning(disable: 4702) // warning C4702: impossible d'atteindre le code
template <typename T, template <typename > class _Op>
static bool TryAssignCopy_(const RTTI::FMetaAtom* value, const Parser::FParseExpression *expr, RTTI::PMetaAtom& result) {
    Assert(!result);
    T tmp;
    if (RTTI::AssignCopy(&tmp, value)) {
        result = RTTI::MakeAtom(_Op<T>()(expr, tmp));
        return true;
    }
    else {
        return false;
    }
}
#pragma warning(pop)
//----------------------------------------------------------------------------
template <template <typename > class _Op>
struct TUnaryOp {
    RTTI::FMetaAtom *operator ()(Parser::FParseContext *context, const Parser::FParseExpression *expr) const {
        const RTTI::PCMetaAtom value = expr->Eval(context);

        Assert(value);

        switch (value->TypeInfo().Id) {
        case PARSEID_BOOL:
            return RTTI::MakeAtom( _Op< FParseBool >()(expr, value->Cast<FParseBool>()->Wrapper()) );

        case PARSEID_INT:
            return RTTI::MakeAtom( _Op< FParseInt >()(expr, value->Cast<FParseInt>()->Wrapper()) );

        case PARSEID_FLOAT:
            return RTTI::MakeAtom( _Op< FParseFloat >()(expr, value->Cast<FParseFloat>()->Wrapper()) );

        case PARSEID_STRING:
            return RTTI::MakeAtom( _Op< FParseString >()(expr, value->Cast<FParseString>()->Wrapper()) );

        default:
            // now try to cast, ie not the current type but accessible through meta cast
            {
                RTTI::PMetaAtom result;
                if (TryAssignCopy_<FParseBool, _Op>(value.get(), expr, result) ||
                    TryAssignCopy_<FParseInt, _Op>(value.get(), expr, result) ||
                    TryAssignCopy_<FParseFloat, _Op>(value.get(), expr, result) ||
                    TryAssignCopy_<FParseString, _Op>(value.get(), expr, result) ) {
                    Assert(result);
                    return RemoveRef_AssertReachZero_KeepAlive(result);
                }
            }
        }

        CORE_THROW_IT(Parser::FParserException("invalid atom type", expr));
    }
};
//----------------------------------------------------------------------------
template <template <typename > class _Op>
struct TBinaryOp {
    static RTTI::FMetaAtom *BooleanOp_(
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FMetaAtom *lhs_value,
        const RTTI::FMetaAtom *rhs_value) {
        const RTTI::FMetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        if (rhs_type_id == PARSEID_BOOL) {
            return RTTI::MakeAtom(
                _Op< FParseBool >()(lhs, lhs_value->Cast<FParseBool>()->Wrapper(),
                                        rhs_value->Cast<FParseBool>()->Wrapper() )
                );
        }
        else {
            FParseBool b;
            if (rhs_type_id == PARSEID_INT)
                b = (FParseInt(0) != rhs_value->Cast<FParseInt>()->Wrapper());
            else if (rhs_type_id == PARSEID_FLOAT)
                b = (FParseFloat(0) != rhs_value->Cast<FParseFloat>()->Wrapper());
            else {
                FParseInt integer;
                FParseFloat fp;

                if (RTTI::AssignCopy(&integer, rhs_value))
                    b = (FParseInt(0) != integer);
                else if (RTTI::AssignCopy(&fp, rhs_value))
                    b = (FParseFloat(0) != fp);
                else
                    CORE_THROW_IT(Parser::FParserException("could not convert to boolean", rhs));
            }

            return RTTI::MakeAtom(
                _Op< FParseBool >()(lhs, lhs_value->Cast<FParseBool>()->Wrapper(), b)
                );
        }
    }

    static RTTI::FMetaAtom *IntegerOp_(
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FMetaAtom *lhs_value,
        const RTTI::FMetaAtom *rhs_value) {
        const RTTI::FMetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        if (rhs_type_id == PARSEID_BOOL)
            return RTTI::MakeAtom(
                _Op< FParseInt >()(lhs, lhs_value->Cast<FParseInt>()->Wrapper(), rhs_value->Cast<FParseBool>()->Wrapper() ? FParseInt(1) : FParseInt(0))
                );
        else if (rhs_type_id == PARSEID_INT)
            return RTTI::MakeAtom(
                _Op< FParseInt >()(lhs, lhs_value->Cast<FParseInt>()->Wrapper(), rhs_value->Cast<FParseInt>()->Wrapper())
                );
        else if (rhs_type_id == PARSEID_FLOAT)
            return RTTI::MakeAtom(
                _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value->Cast<FParseInt>()->Wrapper()), rhs_value->Cast<FParseFloat>()->Wrapper())
                );
        else {
            FParseInt integer;
            FParseFloat fp;

            if (RTTI::AssignCopy(&integer, rhs_value))
                return RTTI::MakeAtom(
                    _Op< FParseInt >()(lhs, lhs_value->Cast<FParseInt>()->Wrapper(), integer)
                    );
            else if (RTTI::AssignCopy(&fp, rhs_value))
                return RTTI::MakeAtom(
                    _Op< FParseFloat >()(lhs, static_cast<FParseFloat>(lhs_value->Cast<FParseInt>()->Wrapper()), fp)
                    );
        }

        CORE_THROW_IT(Parser::FParserException("could not convert to integer", rhs));
    }

    static RTTI::FMetaAtom *FloatOp_(
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FMetaAtom *lhs_value,
        const RTTI::FMetaAtom *rhs_value) {
        const RTTI::FMetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        FParseFloat f;
        if (rhs_type_id == PARSEID_FLOAT)
            f = rhs_value->Cast<FParseFloat>()->Wrapper();
        else if (rhs_type_id == PARSEID_INT)
            f = static_cast<FParseFloat>(rhs_value->Cast<FParseInt>()->Wrapper());
        else if (rhs_type_id == PARSEID_BOOL)
            f = rhs_value->Cast<FParseBool>()->Wrapper() ? FParseFloat(1) : FParseFloat(0);
        else {
            FParseInt integer;
            FParseFloat fp;

            if (RTTI::AssignCopy(&integer, rhs_value))
                f = static_cast<FParseFloat>(integer);
            else if (RTTI::AssignCopy(&fp, rhs_value))
                f = fp;
            else
                CORE_THROW_IT(Parser::FParserException("could not convert to float", rhs));
        }

        return RTTI::MakeAtom(
            _Op< FParseFloat >()(lhs, lhs_value->Cast<FParseFloat>()->Wrapper(), f)
            );
    }

    static RTTI::FMetaAtom *StringOp_(
        const Parser::FParseExpression *lhs,
        const Parser::FParseExpression *rhs,
        const RTTI::FMetaAtom *lhs_value,
        const RTTI::FMetaAtom *rhs_value) {
        const RTTI::FMetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        if (rhs_type_id == PARSEID_STRING)
            return RTTI::MakeAtom(
                _Op< FParseString >()(lhs, lhs_value->Cast<FParseString>()->Wrapper(), rhs_value->Cast<FParseString>()->Wrapper())
                );

        STACKLOCAL_OCSTRSTREAM(oss, 256);

        if (rhs_type_id == PARSEID_BOOL)
            oss << rhs_value->Cast<FParseBool>()->Wrapper();
        else if (rhs_type_id == PARSEID_INT)
            oss << rhs_value->Cast<FParseInt>()->Wrapper();
        else if (rhs_type_id == PARSEID_FLOAT)
            oss << rhs_value->Cast<FParseFloat>()->Wrapper();
        else {
            FParseInt integer;
            FParseFloat fp;
            FParseBool boolean;

            if (RTTI::AssignCopy(&boolean, rhs_value))
                oss << boolean;
            else if (RTTI::AssignCopy(&integer, rhs_value))
                oss << integer;
            else if (RTTI::AssignCopy(&fp, rhs_value))
                oss << fp;
            else
                CORE_THROW_IT(Parser::FParserException("could not convert to string", rhs));
        }

        return RTTI::MakeAtom(
            _Op< FParseString >()(lhs, lhs_value->Cast<FParseString>()->Wrapper(), oss.NullTerminatedStr())
            );
    }

    RTTI::FMetaAtom *operator ()(Parser::FParseContext *context, const Parser::FParseExpression *lhs, const Parser::FParseExpression *rhs) const {
        const RTTI::PCMetaAtom lhs_value = lhs->Eval(context);
        const RTTI::PCMetaAtom rhs_value = rhs->Eval(context);

        Assert(lhs_value);
        Assert(rhs_value);

        switch (lhs_value->TypeInfo().Id)
        {
        case PARSEID_BOOL:
            return BooleanOp_(lhs, rhs, lhs_value.get(), rhs_value.get());

        case PARSEID_INT:
            return IntegerOp_(lhs, rhs, lhs_value.get(), rhs_value.get());

        case PARSEID_FLOAT:
            return FloatOp_(lhs, rhs, lhs_value.get(), rhs_value.get());

        case PARSEID_STRING:
            return StringOp_(lhs, rhs, lhs_value.get(), rhs_value.get());

        default:
            // now try to cast, ie not the current type but accessible through meta cast
            {
                RTTI::TMetaTypedAtom< FParseBool > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return BooleanOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }{
                RTTI::TMetaTypedAtom< FParseInt > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return IntegerOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }{
                RTTI::TMetaTypedAtom< FParseFloat > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return FloatOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }{
                RTTI::TMetaTypedAtom< FParseString > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return StringOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }
        }

        CORE_THROW_IT(Parser::FParserException("invalid atom type", lhs));
    }
};
//----------------------------------------------------------------------------
struct FTernaryOp {
    bool operator ()(Parser::FParseContext *context, const Parser::FParseExpression *expr) const {
        const RTTI::PCMetaAtom value = expr->Eval(context);

        Assert(value);

        switch (value->TypeInfo().Id) {
        case PARSEID_BOOL:
            return value->Cast<FParseBool>()->Wrapper();

        case PARSEID_INT:
            return value->Cast<FParseInt>()->Wrapper() != FParseInt(0);

        case PARSEID_FLOAT:
            return value->Cast<FParseFloat>()->Wrapper() != FParseFloat(0);

        case PARSEID_STRING:
            return value->Cast<FParseString>()->Wrapper().size() != 0;
        }

        CORE_THROW_IT(Parser::FParserException("invalid atom type", expr));
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGrammarImpl {
public:
    FGrammarImpl();
    ~FGrammarImpl();

    Parser::PCParseItem Parse(Parser::FParseList& input) const;

private:
    Parser::TProduction< Parser::PCParseExpression > _literal;
    Parser::TProduction< Parser::PCParseExpression > _variable;
    Parser::TProduction< Parser::PCParseExpression > _object;
    Parser::TProduction< Parser::PCParseExpression > _rvalue;
    Parser::TProduction< Parser::PCParseExpression > _member;

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

    Parser::TProduction< Parser::PCParseExpression > _pair;
    Parser::TProduction< Parser::PCParseExpression > _array;
    Parser::TProduction< Parser::PCParseExpression > _dictionary;
    Parser::TProduction< Parser::PCParseExpression > _cast;

    Parser::TProduction< Parser::PCParseExpression > _export;
    Parser::TProduction< Parser::PCParseExpression > _expr;

    Parser::TProduction< Parser::PCParseStatement >  _property;
    Parser::TProduction< Parser::PCParseStatement >  _statement;
};
//----------------------------------------------------------------------------
FGrammarImpl::FGrammarImpl()
:   _literal(
        Parser::Expect(Lexer::FSymbol::Nil).Select<Parser::PCParseExpression>([](const Lexer::FMatch *&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(FParseObject(), m->Site());
    })
    .Or(Parser::Expect(Lexer::FSymbol::True).Select<Parser::PCParseExpression>([](const Lexer::FMatch *&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(true, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::False).Select<Parser::PCParseExpression>([](const Lexer::FMatch *&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(false, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::Int).Select<Parser::PCParseExpression>([](const Lexer::FMatch *&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        const int64_t value = atoll(m->Value().c_str());
        return Parser::MakeLiteral(value, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::Float).Select<Parser::PCParseExpression>([](const Lexer::FMatch *&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        const double value = atof(m->Value().c_str());
        return Parser::MakeLiteral(value, m->Site());
    }))
    .Or(Parser::Expect(Lexer::FSymbol::String).Select<Parser::PCParseExpression>([](const Lexer::FMatch *&& m) -> Parser::PCParseExpression {
        Assert(m/* && m->Value().size()*//* strings can be empty */);
        return Parser::MakeLiteral(m->Value(), m->Site());
    }))
    )

,   _variable(
        Parser::Expect(Lexer::FSymbol::Identifier).Select<Parser::PCParseExpression>([](const Lexer::FMatch *&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        return Parser::MakeVariableReference(RTTI::FName(m->Value()), m->Site());
    })
    .Or(        Parser::Expect(Lexer::FSymbol::Dollar)
            .Or(Parser::Expect(Lexer::FSymbol::Complement))
        .And(  (    Parser::Expect(Lexer::FSymbol::Div)
            .And(   Parser::Expect(Lexer::FSymbol::Identifier))
               ).AtLeastOnce())
        .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, Parser::TEnumerable<TTuple<const Lexer::FMatch *, const Lexer::FMatch *> > >& args) -> Parser::PCParseExpression {
            const Lexer::FMatch *root = std::get<0>(args);
            const Parser::TEnumerable<TTuple<const Lexer::FMatch *, const Lexer::FMatch *> >& path = std::get<1>(args);

            STACKLOCAL_OCSTRSTREAM(oss, 256);

            if (Lexer::FSymbol::Dollar == root->Symbol()->Type())
                oss << '$';
            else if (Lexer::FSymbol::Complement == root->Symbol()->Type())
                oss << '~'; // TODO : namespace name
            else
                Assert(false);

            for (const auto& it : path) {
                Assert(Lexer::FSymbol::Div == std::get<0>(it)->Symbol()->Type());
                oss << '/' << std::get<1>(it)->Value();
            }

            return Parser::MakeVariableReference(RTTI::FName(oss.MakeView_NullTerminated()), root->Site());
        })
    ))

,   _object(
            Parser::Expect(Lexer::FSymbol::Identifier)
    .And(   Parser::Expect(Lexer::FSymbol::LParenthese))
    .And(       _property.Ref()
            .Or(_statement.Ref())
            .Many() )
    .And(   Parser::Expect(Lexer::FSymbol::RParenthese))
    .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, const Lexer::FMatch *, const Parser::TEnumerable<Parser::PCParseStatement>, const Lexer::FMatch *>& args) -> Parser::PCParseExpression {
        const Lexer::FMatch *name = std::get<0>(args);
        const Parser::TEnumerable<Parser::PCParseStatement>& statements = std::get<2>(args);

        Assert(name);

        return Parser::MakeObjectDefinition(RTTI::FName(name->Value()), name->Site(), statements.begin(), statements.end());
    }))

,   _rvalue(
    _literal.Ref()
    .Or(_object.Ref())
    .Or(_variable.Ref())
    .Or(Parser::Expect(Lexer::FSymbol::LParenthese).And(_expr.Ref()).And(Parser::Expect(Lexer::FSymbol::RParenthese))
        .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *>& args) -> Parser::PCParseExpression {
            return std::get<1>(args);
        })
    ))

,   _member(
            _rvalue.Ref()
    .And(       Parser::Expect(Lexer::FSymbol::Dot)
        .And(   Parser::Expect(Lexer::FSymbol::Identifier))
        .Many())
        .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, const Lexer::FMatch *> > >& args) -> Parser::PCParseExpression {
            const Parser::PCParseExpression& rvalue = std::get<0>(args);
            const Parser::TEnumerable<TTuple<const Lexer::FMatch *, const Lexer::FMatch *> >& members = std::get<1>(args);

            Assert(rvalue);
            if (members.empty())
                return rvalue;

            Parser::PCParseExpression result = rvalue;
            for (const auto& it : members) {
                const Lexer::FMatch *dot = std::get<0>(it);
                const Lexer::FMatch *member = std::get<1>(it);

                Assert(dot);
                Assert(member);

                result = Parser::MakePropertyReference(result, RTTI::FName(member->Value()), dot->Site());
            }

            return result;
        })
    )

,   _pow(
    _member.Ref().And(Parser::Expect(Lexer::FSymbol::Pow).And(_member.Ref()).Many())
        .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
            const Parser::PCParseExpression& lhs = std::get<0>(args);
            const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

            Parser::PCParseExpression result = lhs;

            for (const auto& it : ops) {
                const Lexer::FMatch *op = std::get<0>(it);
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
    |   Lexer::FSymbol::TNot
    |   Lexer::FSymbol::Complement)
    .Many()
    .And(_pow.Ref())
        .Select<Parser::PCParseExpression>([](const TTuple<Parser::TEnumerable<const Lexer::FMatch *>, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            const Parser::TEnumerable<const Lexer::FMatch *>& ops = std::get<0>(args);
            const Parser::PCParseExpression& expr = std::get<1>(args);

            Parser::PCParseExpression result = expr;

            for (const Lexer::FMatch *op : ops) {
                switch (op->Symbol()->Type())
                {
                case Lexer::FSymbol::Add:
                    return expr;

                case Lexer::FSymbol::Sub:
                    result = Parser::MakeUnaryFunction(TUnaryOp<TUnOp_Sub>(), result.get(), op->Site());
                    break;

                case Lexer::FSymbol::TNot:
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
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            switch (op->Symbol()->Type())
            {
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
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
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
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
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
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
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
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
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
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::FSymbol::And);
            result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_And>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _xor(
    _and.Ref().And(Parser::Expect(Lexer::FSymbol::Xor).And(_and.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::FSymbol::Xor);
            result = Parser::MakeBinaryFunction(TBinaryOp<TBinOp_Xor>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _or(
    _xor.Ref().And(Parser::Expect(Lexer::FSymbol::Or).And(_xor.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *op = std::get<0>(it);
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
    .Select<Parser::PCParseExpression>([](const TTuple<Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::FMatch *question = std::get<0>(it);
            const Lexer::FMatch *colon = std::get<2>(it);

            Assert(question && question->Symbol()->Type() == Lexer::FSymbol::Question);
            Assert(colon && colon->Symbol()->Type() == Lexer::FSymbol::Colon);
            UNUSED(colon);

            const Parser::PCParseExpression& ptrue = std::get<1>(it);
            const Parser::PCParseExpression& pfalse = std::get<3>(it);

            result = Parser::MakeTernary(FTernaryOp(), result.get(), ptrue.get(), pfalse.get(), question->Site());
        }

        return result;
    }))

    , _pair(
                Parser::Expect(Lexer::FSymbol::LParenthese)
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::RParenthese))
    .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *> & args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<1>(args);
        const Parser::PCParseExpression& rhs = std::get<3>(args);

        return Parser::MakePair(lhs, rhs, std::get<0>(args)->Site());
    }))

,   _array(
            Parser::Expect(Lexer::FSymbol::LBracket)
    .And(   Parser::Expect(Lexer::FSymbol::RBracket))
    .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, const Lexer::FMatch *>& args) -> Parser::PCParseExpression {
        const Lexer::FMatch *lbracket = std::get<0>(args);

        return Parser::MakeArray(TMemoryView<const Parser::PCParseExpression>(), lbracket->Site());
    })
    .Or(    Parser::Expect(Lexer::FSymbol::LBracket)
    .And(   _expr.Ref())
    .And(   Parser::Expect(Lexer::FSymbol::Comma).And(_expr.Ref()).Many() )
    .And(   Parser::Expect(Lexer::FSymbol::RBracket))
    .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, Parser::PCParseExpression, Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >, const Lexer::FMatch *>& args) -> Parser::PCParseExpression {
        const Lexer::FMatch *lbracket = std::get<0>(args);
        const Parser::PCParseExpression& item0 = std::get<1>(args);
        const Parser::TEnumerable<TTuple<const Lexer::FMatch *, Parser::PCParseExpression> >& item1N = std::get<2>(args);

        Parser::TArray *array = new Parser::TArray(lbracket->Site());
        array->reserve(1 + item1N.size());

        array->push_back(item0);

        for (const auto& it : item1N) {
            const Parser::PCParseExpression& expr = std::get<1>(it);
            array->push_back(expr);
        }

        return array;
    })
    ))

,   _dictionary(
            Parser::Expect(Lexer::FSymbol::LBrace)
    .And(   Parser::Expect(Lexer::FSymbol::RBrace))
    .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, const Lexer::FMatch *>& args) -> Parser::PCParseExpression {
        const Lexer::FMatch *lbracket = std::get<0>(args);

        return Parser::MakeDictionary(TMemoryView<const TPair<Parser::PCParseExpression, Parser::PCParseExpression>>(), lbracket->Site());
    })
        .Or(Parser::Expect(Lexer::FSymbol::LBrace)
    .And(
                Parser::Expect(Lexer::FSymbol::LParenthese)
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::FSymbol::RParenthese))
        .Select<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >([](const TTuple<const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *>& args) -> TTuple<Parser::PCParseExpression, Parser::PCParseExpression> {
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
        .Select<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >([](const TTuple<const Lexer::FMatch *, const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *, Parser::PCParseExpression, const Lexer::FMatch *>& args) -> TTuple<Parser::PCParseExpression, Parser::PCParseExpression> {
            const Parser::PCParseExpression& lhs = std::get<2>(args);
            const Parser::PCParseExpression& rhs = std::get<4>(args);

            return MakeTuple(lhs, rhs);
        })
        .Many()
        )
    .And(   Parser::Expect(Lexer::FSymbol::RBrace))
    .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, Parser::PCParseExpression, Parser::PCParseExpression, Parser::TEnumerable<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >, const Lexer::FMatch *>& args) -> Parser::PCParseExpression {
        const Lexer::FMatch *lbracket = std::get<0>(args);
        const Parser::PCParseExpression& key0 = std::get<1>(args);
        const Parser::PCParseExpression& value0 = std::get<2>(args);
        const Parser::TEnumerable<TTuple<Parser::PCParseExpression, Parser::PCParseExpression> >& item1N = std::get<3>(args);

        Parser::TDictionary *dict = new Parser::TDictionary(lbracket->Site());
        dict->reserve(1 + item1N.size());

        dict->insert(key0, value0);

        for (const auto& it : item1N)
            dict->insert(std::get<0>(it), std::get<1>(it));

        return dict;
    })
    ))

,   _cast(
            Parser::Expect(Lexer::FSymbol::Typename)
    .And(   Parser::Expect(Lexer::FSymbol::Colon))
    .And(   _pair.Ref().Or(_array.Ref()).Or(_dictionary.Ref()).Or(_ternary.Ref()).Or(_cast.Ref()) )
        .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, const Lexer::FMatch *, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            const Lexer::FMatch *typename_ = std::get<0>(args);
            const Parser::PCParseExpression& value = std::get<2>(args);

            Assert(typename_);
            Assert(value);

            return Parser::MakeCastExpr(RTTI::FMetaTypeId(typename_->Symbol()->Ord()), value.get(), typename_->Site());
        })
    )

,   _export(
            Parser::Optional(Lexer::FSymbol::Export)
    .And(   Parser::Expect(Lexer::FSymbol::Identifier))
    .And(   Parser::Expect(Lexer::FSymbol::Is))
    .And(   _cast.Ref().Or(_pair.Ref()).Or(_array.Ref()).Or(_dictionary.Ref()).Or(_ternary.Ref()))
        .Select<Parser::PCParseExpression>([](const TTuple<const Lexer::FMatch *, const Lexer::FMatch *, const Lexer::FMatch *, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            const Lexer::FMatch *exportIFP = std::get<0>(args);
            const Lexer::FMatch *name = std::get<1>(args);
            const Parser::PCParseExpression& value = std::get<3>(args);

            Assert(name);
            Assert(value);

            const Parser::FVariableExport::EFlags scope = (exportIFP)
                ? Parser::FVariableExport::Global
                : Parser::FVariableExport::Public;

            return Parser::MakeVariableExport(RTTI::FName(name->Value()), value, scope, name->Site());
        })
    )

,   _expr(
        _export.Ref().Or(_cast.Ref()).Or(_pair.Ref()).Or(_array.Ref()).Or(_dictionary.Ref()).Or(_ternary.Ref())
        )

,   _property(
            Parser::Expect(Lexer::FSymbol::Identifier)
    .And(   Parser::Expect(Lexer::FSymbol::Assignment))
    .And(   _expr.Ref())
        .Select<Parser::PCParseStatement>([](const TTuple<const Lexer::FMatch *, const Lexer::FMatch *, Parser::PCParseExpression>& args) {
            const Lexer::FMatch *name = std::get<0>(args);
            const Parser::PCParseExpression& value = std::get<2>(args);

            Assert(name);
            Assert(value);

            return Parser::MakePropertyAssignment(RTTI::FName(name->Value()), value);
        })
    )

,   _statement(
        _expr.Ref().Select<Parser::PCParseStatement>([](const Parser::PCParseExpression& expr) {
            return Parser::MakeEvalExpr(expr);
        })
    )

    {}
//----------------------------------------------------------------------------
FGrammarImpl::~FGrammarImpl() {}
//----------------------------------------------------------------------------
Parser::PCParseItem FGrammarImpl::Parse(Parser::FParseList& input) const {
    return _statement.Parse(input);
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
void FGrammarStartup::ClearAll_UnusedMemory() {
    Lexer::FLexerStartup::ClearAll_UnusedMemory();
    Parser::FParserStartup::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
Parser::PCParseItem FGrammarStartup::Parse(Parser::FParseList& input) {
    Assert(sGrammarImpl);
    return sGrammarImpl->Parse(input);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core

#pragma warning( pop )
