#pragma once

#include "Parser/ParseExpression.h"

#include "Parser/ParseContext.h"

#include "Parser.h"

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TLiteral<T>::TLiteral(T&& rvalue, const Lexer::FSpan& site)
:   FParseExpression(site)
,   _literal(std::move(rvalue)) {
}
//----------------------------------------------------------------------------
template <typename T>
TLiteral<T>::~TLiteral() = default;
//----------------------------------------------------------------------------
template <typename T>
RTTI::FAtom TLiteral<T>::Eval(FParseContext* context) const {
    return context->CreateAtomFrom(T(_literal));
}
//----------------------------------------------------------------------------
template <typename T>
FString TLiteral<T>::ToString() const {
    return RTTI::MakeAtom(&_literal).ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
TUnaryFunction<_Functor>::TUnaryFunction(FString&& symbol, _Functor&& functor, const FParseExpression *expr, const Lexer::FSpan& site)
:   FParseExpression(site)
,   _symbol(std::move(symbol))
,   _functor(std::move(functor))
,   _expr(expr) {
    Assert(_expr);
    Assert(not _symbol.empty());
}
//----------------------------------------------------------------------------
template <typename _Functor>
TUnaryFunction<_Functor>::~TUnaryFunction() = default;
//----------------------------------------------------------------------------
template <typename _Functor>
RTTI::FAtom TUnaryFunction<_Functor>::Eval(FParseContext *context) const {
    Assert(_expr);

    return _functor(context, _expr.get());
}
//----------------------------------------------------------------------------
template <typename _Functor>
FString TUnaryFunction<_Functor>::ToString() const {
    FStringBuilder oss;
    oss << _symbol << _expr->ToString();
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
TBinaryFunction<_Functor>::TBinaryFunction(FString&& symbol, _Functor&& functor, const FParseExpression* lhs, const FParseExpression* rhs, const Lexer::FSpan& site)
:   FParseExpression(site)
,   _symbol(std::move(symbol))
,   _functor(std::move(functor))
,   _lhs(lhs)
,   _rhs(rhs) {
    Assert(not _symbol.empty());
    Assert(_lhs);
    Assert(_rhs);
}
//----------------------------------------------------------------------------
template <typename _Functor>
TBinaryFunction<_Functor>::~TBinaryFunction() = default;
//----------------------------------------------------------------------------
template <typename _Functor>
RTTI::FAtom TBinaryFunction<_Functor>::Eval(FParseContext *context) const {
    Assert(_lhs);
    Assert(_rhs);

    return _functor(context, _lhs.get(), _rhs.get());
}
//----------------------------------------------------------------------------
template <typename _Functor>
FString TBinaryFunction<_Functor>::ToString() const {
    FStringBuilder oss;
    oss << _lhs->ToString() << ' ' << _symbol << ' ' << _rhs->ToString();
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Test>
TTernary<_Test>::TTernary(_Test&& test, const FParseExpression *pif, const FParseExpression *ptrue, const FParseExpression *pfalse, const Lexer::FSpan& site)
:   FParseExpression(site)
,   _test(std::move(test))
,   _if(pif), _true(ptrue), _false(pfalse) {
    Assert(pif);
    Assert(ptrue);
    Assert(pfalse);
}
//----------------------------------------------------------------------------
template <typename _Test>
TTernary<_Test>::~TTernary() = default;
//----------------------------------------------------------------------------*
template <typename _Test>
RTTI::FAtom TTernary<_Test>::Eval(FParseContext *context) const {
    Assert(_if);
    Assert(_true);
    Assert(_false);

    return _test(context, _if.get())
        ? _true->Eval(context)
        : _false->Eval(context);
}
//----------------------------------------------------------------------------
template <typename _Test>
FString TTernary<_Test>::ToString() const {
    FStringBuilder oss;
    oss << _if->ToString() << '?' << _true->ToString() << ':' << _false->ToString();
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
