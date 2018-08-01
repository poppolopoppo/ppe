#pragma once

#include "Core.Serialize/Parser/ParseExpression.h"

#include "Core.Serialize/Parser/ParseContext.h"

#include "Core/Allocator/PoolAllocator-impl.h"

#include "Parser.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, TLiteral<T>, template <typename T>)
//----------------------------------------------------------------------------
template <typename T>
TLiteral<T>::TLiteral(T&& rvalue, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _literal(std::move(rvalue)) {
}
//----------------------------------------------------------------------------
template <typename T>
TLiteral<T>::TLiteral(const T& value, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _literal(value) {
}
//----------------------------------------------------------------------------
template <typename T>
TLiteral<T>::~TLiteral() {}
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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, TUnaryFunction<_Functor>, template <typename _Functor>)
//----------------------------------------------------------------------------
template <typename _Functor>
TUnaryFunction<_Functor>::TUnaryFunction(_Functor&& functor, const FParseExpression *expr, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _functor(std::move(functor)), _expr(expr) {
    Assert(expr);
}
//----------------------------------------------------------------------------
template <typename _Functor>
TUnaryFunction<_Functor>::~TUnaryFunction() {}
//----------------------------------------------------------------------------
template <typename _Functor>
RTTI::FAtom TUnaryFunction<_Functor>::Eval(FParseContext *context) const {
    Assert(_expr);

    return _functor(context, _expr.get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, TBinaryFunction<_Functor>, template <typename _Functor>)
//----------------------------------------------------------------------------
template <typename _Functor>
TBinaryFunction<_Functor>::TBinaryFunction(_Functor&& functor, const FParseExpression *lhs, const FParseExpression *rhs, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _functor(std::move(functor)), _lhs(lhs), _rhs(rhs) {
    Assert(lhs);
    Assert(rhs);
}
//----------------------------------------------------------------------------
template <typename _Functor>
TBinaryFunction<_Functor>::~TBinaryFunction() {}
//----------------------------------------------------------------------------
template <typename _Functor>
RTTI::FAtom TBinaryFunction<_Functor>::Eval(FParseContext *context) const {
    Assert(_lhs);
    Assert(_rhs);

    return _functor(context, _lhs.get(), _rhs.get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, TTernary<_Test>, template <typename _Test>)
//----------------------------------------------------------------------------
template <typename _Test>
TTernary<_Test>::TTernary(_Test&& test, const FParseExpression *pif, const FParseExpression *ptrue, const FParseExpression *pfalse, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _test(std::move(test))
,   _if(pif), _true(ptrue), _false(pfalse) {
    Assert(pif);
    Assert(ptrue);
    Assert(pfalse);
}
//----------------------------------------------------------------------------
template <typename _Test>
TTernary<_Test>::~TTernary() {}
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
