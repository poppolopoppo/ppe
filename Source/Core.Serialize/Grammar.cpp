#include "stdafx.h"

#include "Grammar.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

#include "IO/Format.h"
#include "IO/Stream.h"

#include "RTTI/Object/MetaObject.h"
#include "RTTI/Type/MetaTypePromote.h"

//#pragma warning( disable : 4702 ) // warning C4702: impossible d'atteindre le code
// due to compiler optimizations in release mode

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
typedef bool                ParseBool;
typedef int64_t             ParseInt;
typedef double              ParseFloat;
typedef Core::String        ParseString; // TODO - WString, change Lexer to wchar_t
typedef RTTI::PMetaObject   ParseObject;
//----------------------------------------------------------------------------
STATIC_ASSERT(RTTI::MetaType<ParseBool    >::TypeId == 1 );
STATIC_ASSERT(RTTI::MetaType<ParseInt     >::TypeId == 5 );
STATIC_ASSERT(RTTI::MetaType<ParseFloat   >::TypeId == 11);
STATIC_ASSERT(RTTI::MetaType<ParseString  >::TypeId == 12);
STATIC_ASSERT(RTTI::MetaType<ParseObject  >::TypeId == 15);
//----------------------------------------------------------------------------
enum ParseTypeId : RTTI::MetaTypeId {
    PARSEID_BOOL    = RTTI::MetaType<ParseBool  >::TypeId,
    PARSEID_INT     = RTTI::MetaType<ParseInt   >::TypeId,
    PARSEID_FLOAT   = RTTI::MetaType<ParseFloat >::TypeId,
    PARSEID_STRING  = RTTI::MetaType<ParseString>::TypeId,
    PARSEID_OBJECT  = RTTI::MetaType<ParseObject>::TypeId
};
//----------------------------------------------------------------------------
#define DEF_UNARYOPERATOR_FUNCTOR(_Name, _Op) template <typename T> \
    struct CONCAT(UnOp_, _Name) { \
        T operator ()(const Parser::ParseExpression *, const T& value) const { \
            return (_Op value); \
        } \
    }

DEF_UNARYOPERATOR_FUNCTOR(Add,  +);
DEF_UNARYOPERATOR_FUNCTOR(Sub,  -);
DEF_UNARYOPERATOR_FUNCTOR(Not,  !);
DEF_UNARYOPERATOR_FUNCTOR(Cpl,  ~);
//----------------------------------------------------------------------------
#define FORBID_UNARYOPERATOR_FUNCTOR(_Name, _Op, _Type) template <> \
    struct CONCAT(UnOp_, _Name)< _Type > { \
        _Type operator ()(const Parser::ParseExpression *expr, const _Type& ) const { \
            throw Parser::ParserException("unary operator " STRINGIZE(_Op) " is not available for <" STRINGIZE(_Type) ">", expr->Site(), expr); \
        } \
    }

FORBID_UNARYOPERATOR_FUNCTOR(Add, +, ParseBool);
FORBID_UNARYOPERATOR_FUNCTOR(Sub, -, ParseBool);
FORBID_UNARYOPERATOR_FUNCTOR(Cpl, ~, ParseBool);

FORBID_UNARYOPERATOR_FUNCTOR(Not, !, ParseFloat);
FORBID_UNARYOPERATOR_FUNCTOR(Cpl, ~, ParseFloat);

FORBID_UNARYOPERATOR_FUNCTOR(Add, +, ParseString);
FORBID_UNARYOPERATOR_FUNCTOR(Sub, -, ParseString);
FORBID_UNARYOPERATOR_FUNCTOR(Not, !, ParseString);
FORBID_UNARYOPERATOR_FUNCTOR(Cpl, ~, ParseString);
//----------------------------------------------------------------------------
#define DEF_BINARYOPERATOR_FUNCTOR(_Name, _Op) template <typename T> \
    struct CONCAT(BinOp_, _Name) { \
        T operator ()(const Parser::ParseExpression *, const T& lhs, const T& rhs) const { \
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
struct BinOp_Pow {
    T operator ()(const Parser::ParseExpression *, const T& lhs, const T& rhs) const {
        return T(std::pow(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
#define FORBID_BINARYOPERATOR_FUNCTOR(_Name, _Op, _Type) template <> \
    struct CONCAT(BinOp_, _Name)< _Type > { \
        _Type operator ()(const Parser::ParseExpression *expr, const _Type& , const _Type& ) const { \
            throw Parser::ParserException("binary operator " STRINGIZE(_Op) " is not available for <" STRINGIZE(_Type) ">", expr->Site(), expr); \
        } \
    }

FORBID_BINARYOPERATOR_FUNCTOR(Sub,  -, ParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Div,  /, ParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Mod,  %, ParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Lsh, <<, ParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Rsh, >>, ParseBool);
FORBID_BINARYOPERATOR_FUNCTOR(Pow, **, ParseBool);

FORBID_BINARYOPERATOR_FUNCTOR(And,  &, ParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Or,   |, ParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Xor,  ^, ParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Lsh, <<, ParseFloat);
FORBID_BINARYOPERATOR_FUNCTOR(Rsh, >>, ParseFloat);

FORBID_BINARYOPERATOR_FUNCTOR(Sub,  -, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Mul,  *, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Div,  /, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Mod,  %, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Pow, **, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(And,  &, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Or,   |, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Xor,  ^, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Lsh, <<, ParseString);
FORBID_BINARYOPERATOR_FUNCTOR(Rsh, >>, ParseString);
//----------------------------------------------------------------------------
template <>
struct BinOp_Add<ParseBool> {
    ParseBool operator ()(const Parser::ParseExpression *, ParseBool lhs, ParseBool rhs) const {
        return lhs || rhs;
    }
};
//----------------------------------------------------------------------------
template <>
struct BinOp_Mul<ParseBool> {
    ParseBool operator ()(const Parser::ParseExpression *, ParseBool lhs, ParseBool rhs) const {
        return lhs && rhs;
    }
};
//----------------------------------------------------------------------------
template <>
struct BinOp_Mod<ParseFloat> {
    ParseFloat operator ()(const Parser::ParseExpression *, ParseFloat lhs, ParseFloat rhs) const {
        return std::fmod(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
#define DEF_BINARYOPERATOR_COMPARATOR(_Name, _Op) template <typename T> \
    struct CONCAT(CmpOp_, _Name) { \
        bool operator ()(const Parser::ParseExpression *, const T& lhs, const T& rhs) const { \
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
template <template <typename > class _Op>
struct UnaryOp {
    RTTI::MetaAtom *operator ()(Parser::ParseContext *context, const Parser::ParseExpression *expr) const {
        const RTTI::PCMetaAtom value = expr->Eval(context);

        Assert(value);

        switch (value->TypeInfo().Id) {
        case PARSEID_BOOL:
            return RTTI::MakeAtom( _Op< ParseBool >()(expr, value->Cast<ParseBool>()->Wrapper()) );

        case PARSEID_INT:
            return RTTI::MakeAtom( _Op< ParseInt >()(expr, value->Cast<ParseInt>()->Wrapper()) );

        case PARSEID_FLOAT:
            return RTTI::MakeAtom( _Op< ParseFloat >()(expr, value->Cast<ParseFloat>()->Wrapper()) );

        case PARSEID_STRING:
            return RTTI::MakeAtom( _Op< ParseString >()(expr, value->Cast<ParseString>()->Wrapper()) );

        default:
            // now try to cast, ie not the current type but accessible through meta cast
            {
                ParseBool tmp;
                if (RTTI::AssignCopy(&tmp, value.get()))
                    return RTTI::MakeAtom(_Op< ParseBool >()(expr, tmp));
            }{
                ParseInt tmp;
                if (RTTI::AssignCopy(&tmp, value.get()))
                    return RTTI::MakeAtom(_Op< ParseInt >()(expr, tmp));
            }{
                ParseFloat tmp;
                if (RTTI::AssignCopy(&tmp, value.get()))
                    return RTTI::MakeAtom(_Op< ParseFloat >()(expr, tmp));
            }{
                ParseString tmp;
                if (RTTI::AssignCopy(&tmp, value.get()))
                    return RTTI::MakeAtom(_Op< ParseString >()(expr, tmp));
            }
        }

        throw Parser::ParserException("invalid atom type", expr);
    }
};
//----------------------------------------------------------------------------
template <template <typename > class _Op>
struct BinaryOp {
    static RTTI::MetaAtom *BooleanOp_(
        const Parser::ParseExpression *lhs,
        const Parser::ParseExpression *rhs,
        const RTTI::MetaAtom *lhs_value,
        const RTTI::MetaAtom *rhs_value) {
        const RTTI::MetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        if (rhs_type_id == PARSEID_BOOL)
            return RTTI::MakeAtom(
                _Op< ParseBool >()(lhs, lhs_value->Cast<ParseBool>()->Wrapper(), rhs_value->Cast<ParseBool>()->Wrapper())
                );

        ParseBool b;
        if (rhs_type_id == PARSEID_INT)
            b = (ParseInt(0) != rhs_value->Cast<ParseInt>()->Wrapper());
        else if (rhs_type_id == PARSEID_FLOAT)
            b = (ParseFloat(0) != rhs_value->Cast<ParseFloat>()->Wrapper());
        else {
            ParseInt integer;
            ParseFloat fp;

            if (RTTI::AssignCopy(&integer, rhs_value))
                b = (ParseInt(0) != integer);
            else if (RTTI::AssignCopy(&fp, rhs_value))
                b = (ParseFloat(0) != fp);
            else
                throw Parser::ParserException("could not convert to boolean", rhs);
        }

        return RTTI::MakeAtom(
            _Op< ParseBool >()(lhs, lhs_value->Cast<ParseBool>()->Wrapper(), b)
            );
    }

    static RTTI::MetaAtom *IntegerOp_(
        const Parser::ParseExpression *lhs,
        const Parser::ParseExpression *rhs,
        const RTTI::MetaAtom *lhs_value,
        const RTTI::MetaAtom *rhs_value) {
        const RTTI::MetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        if (rhs_type_id == PARSEID_BOOL)
            return RTTI::MakeAtom(
                _Op< ParseInt >()(lhs, lhs_value->Cast<ParseInt>()->Wrapper(), rhs_value->Cast<ParseBool>()->Wrapper() ? ParseInt(1) : ParseInt(0))
                );
        else if (rhs_type_id == PARSEID_INT)
            return RTTI::MakeAtom(
                _Op< ParseInt >()(lhs, lhs_value->Cast<ParseInt>()->Wrapper(), rhs_value->Cast<ParseInt>()->Wrapper())
                );
        else if (rhs_type_id == PARSEID_FLOAT)
            return RTTI::MakeAtom(
                _Op< ParseFloat >()(lhs, static_cast<ParseFloat>(lhs_value->Cast<ParseInt>()->Wrapper()), rhs_value->Cast<ParseFloat>()->Wrapper())
                );
        else {
            ParseInt integer;
            ParseFloat fp;

            if (RTTI::AssignCopy(&integer, rhs_value))
                return RTTI::MakeAtom(
                    _Op< ParseInt >()(lhs, lhs_value->Cast<ParseInt>()->Wrapper(), integer)
                    );
            else if (RTTI::AssignCopy(&fp, rhs_value))
                return RTTI::MakeAtom(
                    _Op< ParseFloat >()(lhs, static_cast<ParseFloat>(lhs_value->Cast<ParseInt>()->Wrapper()), fp)
                    );
        }

        throw Parser::ParserException("could not convert to integer", rhs);
    }

    static RTTI::MetaAtom *FloatOp_(
        const Parser::ParseExpression *lhs,
        const Parser::ParseExpression *rhs,
        const RTTI::MetaAtom *lhs_value,
        const RTTI::MetaAtom *rhs_value) {
        const RTTI::MetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        ParseFloat f;
        if (rhs_type_id == PARSEID_FLOAT)
            f = rhs_value->Cast<ParseFloat>()->Wrapper();
        else if (rhs_type_id == PARSEID_INT)
            f = static_cast<ParseFloat>(rhs_value->Cast<ParseInt>()->Wrapper());
        else if (rhs_type_id == PARSEID_BOOL)
            f = rhs_value->Cast<ParseBool>()->Wrapper() ? ParseFloat(1) : ParseFloat(0);
        else {
            ParseInt integer;
            ParseFloat fp;

            if (RTTI::AssignCopy(&integer, rhs_value))
                f = static_cast<ParseFloat>(integer);
            else if (RTTI::AssignCopy(&fp, rhs_value))
                f = fp;
            else
                throw Parser::ParserException("could not convert to float", rhs);
        }

        return RTTI::MakeAtom(
            _Op< ParseFloat >()(lhs, lhs_value->Cast<ParseFloat>()->Wrapper(), f)
            );
    }

    static RTTI::MetaAtom *StringOp_(
        const Parser::ParseExpression *lhs,
        const Parser::ParseExpression *rhs,
        const RTTI::MetaAtom *lhs_value,
        const RTTI::MetaAtom *rhs_value) {
        const RTTI::MetaTypeId rhs_type_id = rhs_value->TypeInfo().Id;

        if (rhs_type_id == PARSEID_STRING)
            return RTTI::MakeAtom(
                _Op< ParseString >()(lhs, lhs_value->Cast<ParseString>()->Wrapper(), rhs_value->Cast<ParseString>()->Wrapper())
                );

        char cstr[256];
        {
            OCStrStream oss(cstr);
            if (rhs_type_id == PARSEID_BOOL)
                oss << rhs_value->Cast<ParseBool>()->Wrapper();
            else if (rhs_type_id == PARSEID_INT)
                oss << rhs_value->Cast<ParseInt>()->Wrapper();
            else if (rhs_type_id == PARSEID_FLOAT)
                oss << rhs_value->Cast<ParseFloat>()->Wrapper();
            else {
                ParseInt integer;
                ParseFloat fp;
                ParseBool boolean;

                if (RTTI::AssignCopy(&boolean, rhs_value))
                    oss << boolean;
                else if (RTTI::AssignCopy(&integer, rhs_value))
                    oss << integer;
                else if (RTTI::AssignCopy(&fp, rhs_value))
                    oss << fp;
                else
                    throw Parser::ParserException("could not convert to string", rhs);
            }
        }

        return RTTI::MakeAtom(
            _Op< ParseString >()(lhs, lhs_value->Cast<ParseString>()->Wrapper(), cstr)
            );
    }

    RTTI::MetaAtom *operator ()(Parser::ParseContext *context, const Parser::ParseExpression *lhs, const Parser::ParseExpression *rhs) const {
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
                RTTI::MetaWrappedAtom< ParseBool > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return BooleanOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }{
                RTTI::MetaWrappedAtom< ParseInt > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return IntegerOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }{
                RTTI::MetaWrappedAtom< ParseFloat > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return FloatOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }{
                RTTI::MetaWrappedAtom< ParseString > metaCasted;
                if (RTTI::AssignCopy(&metaCasted.Wrapper(), lhs_value.get()))
                    return StringOp_(lhs, rhs, &metaCasted, rhs_value.get());
            }
        }

        throw Parser::ParserException("invalid atom type", lhs);
    }
};
//----------------------------------------------------------------------------
struct TernaryOp {
    bool operator ()(Parser::ParseContext *context, const Parser::ParseExpression *expr) const {
        const RTTI::PCMetaAtom value = expr->Eval(context);

        Assert(value);

        switch (value->TypeInfo().Id) {
        case PARSEID_BOOL:
            return value->Cast<ParseBool>()->Wrapper();

        case PARSEID_INT:
            return value->Cast<ParseInt>()->Wrapper() != ParseInt(0);

        case PARSEID_FLOAT:
            return value->Cast<ParseFloat>()->Wrapper() != ParseFloat(0);

        case PARSEID_STRING:
            return value->Cast<ParseString>()->Wrapper().size() != 0;
        }

        throw Parser::ParserException("invalid atom type", expr);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GrammarImpl {
public:
    GrammarImpl();
    ~GrammarImpl();

    Parser::PCParseItem Parse(Parser::ParseList& input) const;

private:
    Parser::Production< Parser::PCParseExpression > _literal;
    Parser::Production< Parser::PCParseExpression > _variable;
    Parser::Production< Parser::PCParseExpression > _object;
    Parser::Production< Parser::PCParseExpression > _rvalue;
    Parser::Production< Parser::PCParseExpression > _member;

    Parser::Production< Parser::PCParseExpression > _pow;
    Parser::Production< Parser::PCParseExpression > _unary;
    Parser::Production< Parser::PCParseExpression > _mulDivMod;
    Parser::Production< Parser::PCParseExpression > _addSub;
    Parser::Production< Parser::PCParseExpression > _lshRsh;
    Parser::Production< Parser::PCParseExpression > _compare;
    Parser::Production< Parser::PCParseExpression > _equalsNotEquals;
    Parser::Production< Parser::PCParseExpression > _and;
    Parser::Production< Parser::PCParseExpression > _xor;
    Parser::Production< Parser::PCParseExpression > _or;
    Parser::Production< Parser::PCParseExpression > _ternary;

    Parser::Production< Parser::PCParseExpression > _pair;
    Parser::Production< Parser::PCParseExpression > _array;
    Parser::Production< Parser::PCParseExpression > _dictionary;

    Parser::Production< Parser::PCParseExpression > _export;
    Parser::Production< Parser::PCParseExpression > _expr;

    Parser::Production< Parser::PCParseStatement >  _property;
    Parser::Production< Parser::PCParseStatement >  _statement;
};
//----------------------------------------------------------------------------
GrammarImpl::GrammarImpl()
:   _literal(
        Parser::Expect(Lexer::Symbol::Nil).Select<Parser::PCParseExpression>([](const Lexer::Match *&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(ParseObject(), m->Site());
    })
    .Or(Parser::Expect(Lexer::Symbol::True).Select<Parser::PCParseExpression>([](const Lexer::Match *&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(true, m->Site());
    }))
    .Or(Parser::Expect(Lexer::Symbol::False).Select<Parser::PCParseExpression>([](const Lexer::Match *&& m) -> Parser::PCParseExpression {
        Assert(m);
        return Parser::MakeLiteral(false, m->Site());
    }))
    .Or(Parser::Expect(Lexer::Symbol::Int).Select<Parser::PCParseExpression>([](const Lexer::Match *&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        const int64_t value = atoll(m->Value().c_str());
        return Parser::MakeLiteral(value, m->Site());
    }))
    .Or(Parser::Expect(Lexer::Symbol::Float).Select<Parser::PCParseExpression>([](const Lexer::Match *&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        const double value = atof(m->Value().c_str());
        return Parser::MakeLiteral(value, m->Site());
    }))
    .Or(Parser::Expect(Lexer::Symbol::String).Select<Parser::PCParseExpression>([](const Lexer::Match *&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        return Parser::MakeLiteral(m->Value(), m->Site());
    }))
    )

,   _variable(
        Parser::Expect(Lexer::Symbol::Identifier).Select<Parser::PCParseExpression>([](const Lexer::Match *&& m) -> Parser::PCParseExpression {
        Assert(m && m->Value().size());
        return Parser::MakeVariableReference(m->Value(), m->Site());
    })
    .Or(        Parser::Expect(Lexer::Symbol::Dollar)
            .Or(Parser::Expect(Lexer::Symbol::Complement))
        .And(  (    Parser::Expect(Lexer::Symbol::Div)
            .And(   Parser::Expect(Lexer::Symbol::Identifier))
               ).AtLeastOnce())
        .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, Parser::Enumerable<Tuple<const Lexer::Match *, const Lexer::Match *> > >& args) -> Parser::PCParseExpression {
            const Lexer::Match *root = std::get<0>(args);
            const Parser::Enumerable<Tuple<const Lexer::Match *, const Lexer::Match *> >& path = std::get<1>(args);

            char name[512]; {
                OCStrStream oss(name);

                if (Lexer::Symbol::Dollar == root->Symbol()->Type())
                    oss << '$';
                else if (Lexer::Symbol::Complement == root->Symbol()->Type())
                    oss << '~'; // TODO : namespace name
                else
                    Assert(false);

                for (const auto& it : path) {
                    Assert(Lexer::Symbol::Div == std::get<0>(it)->Symbol()->Type());
                    oss << '/' << std::get<1>(it)->Value();
                }
            }

            return Parser::MakeVariableReference(name, root->Site());
        })
    ))

,   _object(
            Parser::Expect(Lexer::Symbol::Identifier)
    .And(   Parser::Expect(Lexer::Symbol::LParenthese))
    .And(       _property.Ref()
            .Or(_statement.Ref())
            .Many() )
    .And(   Parser::Expect(Lexer::Symbol::RParenthese))
    .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, const Lexer::Match *, const Parser::Enumerable<Parser::PCParseStatement>, const Lexer::Match *>& args) -> Parser::PCParseExpression {
        const Lexer::Match *metaclassName = std::get<0>(args);
        const Parser::Enumerable<Parser::PCParseStatement>& statements = std::get<2>(args);

        Assert(metaclassName);

        return Parser::MakeObjectDefinition(metaclassName->Value(), metaclassName->Site(), statements.begin(), statements.end());
    }))

,   _rvalue(
    _literal.Ref()
    .Or(_object.Ref())
    .Or(_variable.Ref())
    .Or(Parser::Expect(Lexer::Symbol::LParenthese).And(_expr.Ref()).And(Parser::Expect(Lexer::Symbol::RParenthese))
        .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *>& args) -> Parser::PCParseExpression {
            return std::get<1>(args);
        })
    ))

,   _member(
            _rvalue.Ref()
    .And(       Parser::Expect(Lexer::Symbol::Dot)
        .And(   Parser::Expect(Lexer::Symbol::Identifier))
        .Many())
        .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, const Lexer::Match *> > >& args) -> Parser::PCParseExpression {
            const Parser::PCParseExpression& rvalue = std::get<0>(args);
            const Parser::Enumerable<Tuple<const Lexer::Match *, const Lexer::Match *> >& members = std::get<1>(args);

            Assert(rvalue);
            if (members.empty())
                return rvalue;

            Parser::PCParseExpression result = rvalue;
            for (const auto& it : members) {
                const Lexer::Match *dot = std::get<0>(it);
                const Lexer::Match *member = std::get<1>(it);

                Assert(dot);
                Assert(member);

                result = Parser::MakePropertyReference(result, member->Value(), dot->Site());
            }

            return result;
        })
    )

,   _pow(
    _member.Ref().And(Parser::Expect(Lexer::Symbol::Pow).And(_member.Ref()).Many())
        .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
            const Parser::PCParseExpression& lhs = std::get<0>(args);
            const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

            Parser::PCParseExpression result = lhs;

            for (const auto& it : ops) {
                const Lexer::Match *op = std::get<0>(it);
                const Parser::PCParseExpression& rhs = std::get<1>(it);

                Assert(Lexer::Symbol::Pow == op->Symbol()->Type());
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Pow>(), result.get(), rhs.get(), op->Site());
            }

            return result;
        })
    )

,   _unary(
    Parser::ExpectMask(
        Lexer::Symbol::Add
    |   Lexer::Symbol::Sub
    |   Lexer::Symbol::Not
    |   Lexer::Symbol::Complement)
    .Many()
    .And(_pow.Ref())
        .Select<Parser::PCParseExpression>([](const Tuple<Parser::Enumerable<const Lexer::Match *>, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            const Parser::Enumerable<const Lexer::Match *>& ops = std::get<0>(args);
            const Parser::PCParseExpression& expr = std::get<1>(args);

            Parser::PCParseExpression result = expr;

            for (const Lexer::Match *op : ops) {
                switch (op->Symbol()->Type())
                {
                case Lexer::Symbol::Add:
                    return expr;

                case Lexer::Symbol::Sub:
                    result = Parser::MakeUnaryFunction(UnaryOp<UnOp_Sub>(), result.get(), op->Site());
                    break;

                case Lexer::Symbol::Not:
                    result = Parser::MakeUnaryFunction(UnaryOp<UnOp_Not>(), result.get(), op->Site());
                    break;

                default:
                    Assert(Lexer::Symbol::Complement == op->Symbol()->Type());
                    result = Parser::MakeUnaryFunction(UnaryOp<UnOp_Cpl>(), result.get(), op->Site());
                    break;
                }
            }

            return result;
        })
    )

,   _mulDivMod(
    _unary.Ref().And(Parser::ExpectMask(
        Lexer::Symbol::Mul
    |   Lexer::Symbol::Div
    |   Lexer::Symbol::Mod).And(_unary.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            switch (op->Symbol()->Type())
            {
            case Lexer::Symbol::Mul:
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Mul>(), result.get(), rhs.get(), op->Site());
                break;

            case Lexer::Symbol::Div:
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Div>(), result.get(), rhs.get(), op->Site());
                break;

            default:
                Assert(Lexer::Symbol::Mod == op->Symbol()->Type());
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Mod>(), result.get(), rhs.get(), op->Site());
                break;
            }
        }

        return result;
    }))

,   _addSub(
    _mulDivMod.Ref().And(Parser::ExpectMask(
        Lexer::Symbol::Add
    |   Lexer::Symbol::Sub).And(_mulDivMod.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            if (op->Symbol()->Type() == Lexer::Symbol::Add) {
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Add>(), result.get(), rhs.get(), op->Site());
            }
            else {
                Assert(op->Symbol()->Type() == Lexer::Symbol::Sub);
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Sub>(), result.get(), rhs.get(), op->Site());
            }
        }

        return result;
    }))

,   _lshRsh(
    _addSub.Ref().And(Parser::ExpectMask(
        Lexer::Symbol::LShift
    |   Lexer::Symbol::RShift).And(_addSub.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            if (op->Symbol()->Type() == Lexer::Symbol::LShift) {
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Lsh>(), result.get(), rhs.get(), op->Site());
            }
            else {
                Assert(op->Symbol()->Type() == Lexer::Symbol::RShift);
                result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Rsh>(), result.get(), rhs.get(), op->Site());
            }
        }

        return result;
    }))

,   _compare(
    _lshRsh.Ref().And(Parser::ExpectMask(
        Lexer::Symbol::Less
    |   Lexer::Symbol::LessOrEqual
    |   Lexer::Symbol::Greater
    |   Lexer::Symbol::GreaterOrEqual).And(_lshRsh.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            switch (op->Symbol()->Type())
            {
            case Lexer::Symbol::Less:
                result = Parser::MakeBinaryFunction(BinaryOp<CmpOp_Less>(), result.get(), rhs.get(), op->Site());
                break;

            case Lexer::Symbol::LessOrEqual:
                result = Parser::MakeBinaryFunction(BinaryOp<CmpOp_LessOrEqual>(), result.get(), rhs.get(), op->Site());
                break;

            case Lexer::Symbol::Greater:
                result = Parser::MakeBinaryFunction(BinaryOp<CmpOp_Greater>(), result.get(), rhs.get(), op->Site());
                break;

            default:
                Assert(op->Symbol()->Type() == Lexer::Symbol::GreaterOrEqual);
                result = Parser::MakeBinaryFunction(BinaryOp<CmpOp_GreaterOrEqual>(), result.get(), rhs.get(), op->Site());
                break;
            }
        }

        return result;
    }))

,   _equalsNotEquals(
    _compare.Ref().And(Parser::ExpectMask(
        Lexer::Symbol::Equals
    |   Lexer::Symbol::NotEquals).And(_compare.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            if (op->Symbol()->Type() == Lexer::Symbol::Equals) {
                result = Parser::MakeBinaryFunction(BinaryOp<CmpOp_Equals>(), result.get(), rhs.get(), op->Site());
            }
            else {
                Assert(op->Symbol()->Type() == Lexer::Symbol::NotEquals);
                result = Parser::MakeBinaryFunction(BinaryOp<CmpOp_NotEquals>(), result.get(), rhs.get(), op->Site());
            }
        }

        return result;
    }))

,   _and(
    _equalsNotEquals.Ref().And(Parser::Expect(Lexer::Symbol::And).And(_equalsNotEquals.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::Symbol::And);
            result = Parser::MakeBinaryFunction(BinaryOp<BinOp_And>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _xor(
    _and.Ref().And(Parser::Expect(Lexer::Symbol::Xor).And(_and.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::Symbol::Xor);
            result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Xor>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _or(
    _xor.Ref().And(Parser::Expect(Lexer::Symbol::Or).And(_xor.Ref()).Many())
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *op = std::get<0>(it);
            const Parser::PCParseExpression& rhs = std::get<1>(it);

            Assert(op->Symbol()->Type() == Lexer::Symbol::Or);
            result = Parser::MakeBinaryFunction(BinaryOp<BinOp_Or>(), result.get(), rhs.get(), op->Site());
        }

        return result;
    }))

,   _ternary(
    _or.And(
                Parser::Expect(Lexer::Symbol::Question)
        .And(   _or.Ref())
        .And(   Parser::Expect(Lexer::Symbol::Colon))
        .And(   _or.Ref())
        .Many() )
    .Select<Parser::PCParseExpression>([](const Tuple<Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *, Parser::PCParseExpression> > >& args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<0>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *, Parser::PCParseExpression> >& ops = std::get<1>(args);

        Parser::PCParseExpression result = lhs;

        for (const auto& it : ops) {
            const Lexer::Match *question = std::get<0>(it);
            const Lexer::Match *colon = std::get<2>(it);

            Assert(question && question->Symbol()->Type() == Lexer::Symbol::Question);
            Assert(colon && colon->Symbol()->Type() == Lexer::Symbol::Colon);

            const Parser::PCParseExpression& ptrue = std::get<1>(it);
            const Parser::PCParseExpression& pfalse = std::get<3>(it);

            result = Parser::MakeTernary(TernaryOp(), result.get(), ptrue.get(), pfalse.get(), question->Site());
        }

        return result;
    }))

    , _pair(
                Parser::Expect(Lexer::Symbol::LParenthese)
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::Symbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::Symbol::RParenthese))
    .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *> & args) -> Parser::PCParseExpression {
        const Parser::PCParseExpression& lhs = std::get<1>(args);
        const Parser::PCParseExpression& rhs = std::get<3>(args);

        return Parser::MakePair(lhs, rhs, std::get<0>(args)->Site());
    }))

,   _array(
            Parser::Expect(Lexer::Symbol::LBracket)
    .And(   Parser::Expect(Lexer::Symbol::RBracket))
    .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, const Lexer::Match *>& args) -> Parser::PCParseExpression {
        const Lexer::Match *lbracket = std::get<0>(args);

        return Parser::MakeArray(MemoryView<const Parser::PCParseExpression>(), lbracket->Site());
    })
    .Or(    Parser::Expect(Lexer::Symbol::LBracket)
    .And(   _expr.Ref())
    .And(   Parser::Expect(Lexer::Symbol::Comma).And(_expr.Ref()).Many() )
    .And(   Parser::Expect(Lexer::Symbol::RBracket))
    .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, Parser::PCParseExpression, Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >, const Lexer::Match *>& args) -> Parser::PCParseExpression {
        const Lexer::Match *lbracket = std::get<0>(args);
        const Parser::PCParseExpression& item0 = std::get<1>(args);
        const Parser::Enumerable<Tuple<const Lexer::Match *, Parser::PCParseExpression> >& item1N = std::get<2>(args);

        Parser::Array *array = new Parser::Array(lbracket->Site());
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
            Parser::Expect(Lexer::Symbol::LBrace)
    .And(   Parser::Expect(Lexer::Symbol::RBrace))
    .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, const Lexer::Match *>& args) -> Parser::PCParseExpression {
        const Lexer::Match *lbracket = std::get<0>(args);

        return Parser::MakeDictionary(MemoryView<const Pair<Parser::PCParseExpression, Parser::PCParseExpression>>(), lbracket->Site());
    })
        .Or(Parser::Expect(Lexer::Symbol::LBrace)
    .And(
                Parser::Expect(Lexer::Symbol::LParenthese)
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::Symbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::Symbol::RParenthese))
        .Select<Tuple<Parser::PCParseExpression, Parser::PCParseExpression> >([](const Tuple<const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *>& args) -> Tuple<Parser::PCParseExpression, Parser::PCParseExpression> {
            const Parser::PCParseExpression& lhs = std::get<1>(args);
            const Parser::PCParseExpression& rhs = std::get<3>(args);

            return MakeTuple(lhs, rhs);
        })
        )
    .And(
                Parser::Expect(Lexer::Symbol::Comma)
        .And(   Parser::Expect(Lexer::Symbol::LParenthese))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::Symbol::Comma))
        .And(   _expr.Ref())
        .And(   Parser::Expect(Lexer::Symbol::RParenthese))
        .Select<Tuple<Parser::PCParseExpression, Parser::PCParseExpression> >([](const Tuple<const Lexer::Match *, const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *, Parser::PCParseExpression, const Lexer::Match *>& args) -> Tuple<Parser::PCParseExpression, Parser::PCParseExpression> {
            const Parser::PCParseExpression& lhs = std::get<2>(args);
            const Parser::PCParseExpression& rhs = std::get<4>(args);

            return MakeTuple(lhs, rhs);
        })
        .Many()
        )
    .And(   Parser::Expect(Lexer::Symbol::RBrace))
    .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, Parser::PCParseExpression, Parser::PCParseExpression, Parser::Enumerable<Tuple<Parser::PCParseExpression, Parser::PCParseExpression> >, const Lexer::Match *>& args) -> Parser::PCParseExpression {
        const Lexer::Match *lbracket = std::get<0>(args);
        const Parser::PCParseExpression& key0 = std::get<1>(args);
        const Parser::PCParseExpression& value0 = std::get<2>(args);
        const Parser::Enumerable<Tuple<Parser::PCParseExpression, Parser::PCParseExpression> >& item1N = std::get<3>(args);

        Parser::Dictionary *dict = new Parser::Dictionary(lbracket->Site());
        dict->reserve(1 + item1N.size());

        dict->insert(key0, value0);

        for (const auto& it : item1N)
            dict->insert(std::get<0>(it), std::get<1>(it));

        return dict;
    })
    ))

,   _export(
            Parser::Optional(Lexer::Symbol::Export)
    .And(   Parser::Expect(Lexer::Symbol::Identifier))
    .And(   Parser::Expect(Lexer::Symbol::Is))
    .And(   _pair.Ref().Or(_array.Ref()).Or(_dictionary.Ref()).Or(_ternary.Ref()))
        .Select<Parser::PCParseExpression>([](const Tuple<const Lexer::Match *, const Lexer::Match *, const Lexer::Match *, Parser::PCParseExpression>& args) -> Parser::PCParseExpression {
            const Lexer::Match *exportIFP = std::get<0>(args);
            const Lexer::Match *name = std::get<1>(args);
            const Parser::PCParseExpression& value = std::get<3>(args);

            Assert(name);
            Assert(value);

            const Parser::VariableExport::Flags scope = (exportIFP)
                ? Parser::VariableExport::Global
                : Parser::VariableExport::Public;

            return Parser::MakeVariableExport(name->Value(), value, scope, name->Site());
        })
    )

,   _expr(
        _export.Ref().Or(_pair.Ref()).Or(_array.Ref()).Or(_dictionary.Ref()).Or(_ternary.Ref())
        )

,   _property(
            Parser::Expect(Lexer::Symbol::Identifier)
    .And(   Parser::Expect(Lexer::Symbol::Assignment))
    .And(   _expr.Ref())
        .Select<Parser::PCParseStatement>([](const Tuple<const Lexer::Match *, const Lexer::Match *, Parser::PCParseExpression>& args) {
            const Lexer::Match *name = std::get<0>(args);
            const Parser::PCParseExpression& value = std::get<2>(args);

            Assert(name);
            Assert(value);

            return Parser::MakePropertyAssignment(name->Value(), value);
        })
    )

,   _statement(
        _expr.Ref().Select<Parser::PCParseStatement>([](const Parser::PCParseExpression& expr) {
            return Parser::MakeEvalExpr(expr);
        })
    )

    {}
//----------------------------------------------------------------------------
GrammarImpl::~GrammarImpl() {}
//----------------------------------------------------------------------------
Parser::PCParseItem GrammarImpl::Parse(Parser::ParseList& input) const {
    return _statement.Parse(input);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Grammar::Grammar()
:   _impl(new GrammarImpl()) {}
//----------------------------------------------------------------------------
Grammar::~Grammar() {}
//----------------------------------------------------------------------------
Parser::PCParseItem Grammar::Parse(Parser::ParseList& input) const {
    Assert(_impl);
    return _impl->Parse(input);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
