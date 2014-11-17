#pragma once

#include "Core/Parser/ParseExpression.h"

#include "Core/Parser/ParseContext.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Literal<T>, template <typename T>)
//----------------------------------------------------------------------------
template <typename T>
Literal<T>::Literal(T&& rvalue, const Lexer::Location& site)
:   ParseExpression(site)
,   _literal(RTTI::MakeAtom(std::move(rvalue))) {
    Assert(_literal);
}
//----------------------------------------------------------------------------
template <typename T>
Literal<T>::~Literal() {}
//----------------------------------------------------------------------------
template <typename T>
RTTI::MetaAtom *Literal<T>::Eval(ParseContext *context) const {
    Assert(_literal);
    return _literal.get();
}
//----------------------------------------------------------------------------
template <typename T>
String Literal<T>::ToString() const {
    Assert(_literal);
    return _literal->ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(UnaryFunction<_Functor>, template <typename _Functor>)
//----------------------------------------------------------------------------
template <typename _Functor>
UnaryFunction<_Functor>::UnaryFunction(_Functor&& functor, const ParseExpression *expr, const Lexer::Location& site)
:   ParseExpression(site)
,   _functor(std::move(functor)), _expr(expr) {
    Assert(expr);
}
//----------------------------------------------------------------------------
template <typename _Functor>
UnaryFunction<_Functor>::~UnaryFunction() {}
//----------------------------------------------------------------------------
template <typename _Functor>
RTTI::MetaAtom *UnaryFunction<_Functor>::Eval(ParseContext *context) const {
    Assert(_expr);

    return _functor(context, _expr.get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(BinaryFunction<_Functor>, template <typename _Functor>)
//----------------------------------------------------------------------------
template <typename _Functor>
BinaryFunction<_Functor>::BinaryFunction(_Functor&& functor, const ParseExpression *lhs, const ParseExpression *rhs, const Lexer::Location& site)
:   ParseExpression(site)
,   _functor(std::move(functor)), _lhs(lhs), _rhs(rhs) {
    Assert(lhs);
    Assert(rhs);
}
//----------------------------------------------------------------------------
template <typename _Functor>
BinaryFunction<_Functor>::~BinaryFunction() {}
//----------------------------------------------------------------------------
template <typename _Functor>
RTTI::MetaAtom *BinaryFunction<_Functor>::Eval(ParseContext *context) const {
    Assert(_lhs);
    Assert(_rhs);

    return _functor(context, _lhs.get(), _rhs.get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Ternary<_Test>, template <typename _Test>)
//----------------------------------------------------------------------------
template <typename _Test>
Ternary<_Test>::Ternary(_Test&& test, const ParseExpression *pif, const ParseExpression *ptrue, const ParseExpression *pfalse, const Lexer::Location& site)
:   ParseExpression(site)
,   _test(std::move(test))
,   _if(pif), _true(ptrue), _false(pfalse) {
    Assert(pif);
    Assert(ptrue);
    Assert(pfalse);
}
//----------------------------------------------------------------------------
template <typename _Test>
Ternary<_Test>::~Ternary() {}
//----------------------------------------------------------------------------*
template <typename _Test>
RTTI::MetaAtom *Ternary<_Test>::Eval(ParseContext *context) const {
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
